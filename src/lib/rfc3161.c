/*
 *    "This product includes software developed by the OpenSSL Project
 *     *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 */

#include "utils.h"
#include <civetweb.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/opensslconf.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/ts.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/syslog.h>
#include <syslog.h>

int add_oid_section(rfc3161_context *ct, CONF *conf) {
    char *p;
    STACK_OF(CONF_VALUE) * sktmp;
    CONF_VALUE *cnf;
    int i;

    if ((sktmp = NCONF_get_section(conf, OID_SECTION)) == NULL) {
        uts_logger(ct, LOG_ERR, "problem loading oid section %s\n", p);
        return 0;
    }
    for (i = 0; i < sk_CONF_VALUE_num(sktmp); i++) {
        cnf = sk_CONF_VALUE_value(sktmp, i);
        if (OBJ_create(cnf->value, cnf->name, cnf->name) == NID_undef) {
            uts_logger(ct, LOG_ERR, "problem creating object %s=%s\n",
                       cnf->name, cnf->value);
            return 0;
        }
    }
    return 1;
}

// initialize OpenSSL global black magic...
void init_ssl() {
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    SSL_library_init();
    ERR_load_TS_strings();
}

// free OpenSSL global black magic...
void free_ssl() {
    CONF_modules_unload(1);
    EVP_cleanup();
    // ENGINE_cleanup();
    CRYPTO_cleanup_all_ex_data();
    EVP_cleanup();
    ERR_free_strings();
    OBJ_cleanup();
}

// Recover a ts wrapper from a pool of TS_RESP_CTX.
// Used because TS_RESP_CTX is not thread safe.
ts_resp_ctx_wrapper *get_ctxw(rfc3161_context *ct) {
    ts_resp_ctx_wrapper *ret = NULL;
    // itarate on the 'numthreads' TS_RESP_CTX we have in the pool
    // we have as much as TS_RESP_CTX as parallele handlers
    for (int i = 0; i < ct->numthreads; i++) {
        // wait until we have exclusive access to read and maybe
        // write the ts_resp_ctx_wrapper structure.
        pthread_mutex_lock(&ct->ts_ctx_pool[i].lock);
        // if the TS_RESP_CTX is available,
        // take it and mark it as reserved (available = 0)
        if (ct->ts_ctx_pool[i].available) {
            ct->ts_ctx_pool[i].available = 0;
            ret = &(ct->ts_ctx_pool[i]);
            // unlock the the ts_resp_ctx_wrapper
            pthread_mutex_unlock(&ct->ts_ctx_pool[i].lock);
            // return the ts_resp_ctx_wrapper
            return ret;
        }
        // unlock in case the ts_resp_ctx_wrapper was not available
        pthread_mutex_unlock(&ct->ts_ctx_pool[i].lock);
    }
    // default return if no TS_RESP_CTX wa available
    return ret;
}

// Build a random serial for each request.
// It's less painful to manage than an incremental serial stored in a file
// and a 150 bits size is more than enough to prevent collision.
static ASN1_INTEGER *serial_cb(TS_RESP_CTX *ctx, void *data42) {
    unsigned char data[20] = {0};
    RAND_bytes(data, sizeof(data));
    // data[0] &= 0x7F;

    // build big number from our bytes
    BIGNUM *bn = BN_new();
    BN_bin2bn(data, sizeof(data), bn);

    // build the ASN1_INTEGER from our BIGNUM
    ASN1_INTEGER *asnInt = ASN1_INTEGER_new();
    BN_to_ASN1_INTEGER(bn, asnInt);

    // cleanup
    BN_free(bn);
    return asnInt;
}

// create a TS_RESP_CTX (OpenSSL Time-Stamp Response Context)
TS_RESP_CTX *create_tsctx(rfc3161_context *ct, CONF *conf, const char *section,
                          const char *policy) {
    unsigned long err_code;
    unsigned long err_code_prev = 0;
    TS_RESP_CTX *resp_ctx = NULL;

    // recover the section defining the default tsa
    // if ((section = TS_CONF_get_tsa_section(conf, section)) == NULL) {
    //    uts_logger(ct, LOG_ERR, "failed to get or use '%s' in section [ %s ]",
    //               "default_tsa", "tsa");
    //    goto end;
    //}
    if ((resp_ctx = TS_RESP_CTX_new()) == NULL) {
        uts_logger(ct, LOG_ERR, "failed to initialize tsa context");
        goto end;
    }
    // recover and set various parameters
    TS_RESP_CTX_set_serial_cb(resp_ctx, serial_cb, NULL);
    if (!TS_CONF_set_crypto_device(conf, section, NULL)) {
        uts_logger(ct, LOG_ERR, "failed to get or use '%s' in section [ %s ]",
                   "crypto_device", section);
        uts_logger(ct, LOG_ERR,
                   "failed to get or use the crypto device in section [ %s ]",
                   section);
        goto end;
    }
    if (!TS_CONF_set_signer_cert(conf, section, NULL, resp_ctx)) {
        uts_logger(ct, LOG_ERR, "failed to get or use '%s' in section [ %s ]",
                   "signer_cert", section);
        goto end;
    }
    if (!TS_CONF_set_certs(conf, section, NULL, resp_ctx)) {
        uts_logger(ct, LOG_ERR, "failed to get or use '%s' in section [ %s ]",
                   "default_certs", section);
        goto end;
    }
    if (!TS_CONF_set_signer_key(conf, section, NULL, NULL, resp_ctx)) {
        uts_logger(ct, LOG_ERR, "failed to get or use '%s' in section [ %s ]",
                   "signer_key", section);
        goto end;
    }

    // if (md) {
    //         if (!TS_RESP_CTX_set_signer_digest(resp_ctx, md))
    //                 goto end;
    // } else if (!TS_CONF_set_signer_digest(conf, section, NULL, resp_ctx)) {
    //         goto end;
    // }

    if (!TS_CONF_set_def_policy(conf, section, policy, resp_ctx)) {
        uts_logger(ct, LOG_ERR, "failed to get or use '%s' in section [ %s ]",
                   "default_policy", section);
        goto end;
    }
    if (!TS_CONF_set_policies(conf, section, resp_ctx)) {
        uts_logger(ct, LOG_ERR, "failed to get or use '%s' in section [ %s ]",
                   "policies", section);
        goto end;
    }
    if (!TS_CONF_set_digests(conf, section, resp_ctx)) {
        uts_logger(ct, LOG_ERR, "failed to get or use '%s' in section [ %s ]",
                   "digests", section);
        goto end;
    }
    if (!TS_CONF_set_accuracy(conf, section, resp_ctx)) {
        uts_logger(ct, LOG_ERR, "failed to get or use '%s' in section [ %s ]",
                   "accuracy", section);
        goto end;
    }
    if (!TS_CONF_set_clock_precision_digits(conf, section, resp_ctx)) {
        uts_logger(ct, LOG_ERR, "failed to get or use '%s' in section [ %s ]",
                   "clock_precision_digits", section);
        goto end;
    }
    if (!TS_CONF_set_ordering(conf, section, resp_ctx)) {
        uts_logger(ct, LOG_ERR, "failed to get or use '%s' in section [ %s ]",
                   "ordering", section);
        goto end;
    }
    if (!TS_CONF_set_tsa_name(conf, section, resp_ctx)) {
        uts_logger(ct, LOG_ERR, "failed to get or use '%s' in section [ %s ]",
                   "tsa_name", section);
        goto end;
    }
    if (!TS_CONF_set_ess_cert_id_chain(conf, section, resp_ctx)) {
        uts_logger(ct, LOG_ERR, "failed to get or use '%s' in section [ %s ]",
                   "ess_cert_id_chain", section);
        goto end;
    }
    return resp_ctx;
end:
    // log the OpenSSL errors if any in case of error
    while ((err_code = ERR_get_error())) {
        if (err_code_prev != err_code) {
            uts_logger(ct, LOG_DEBUG, "OpenSSL exception: '%s'",
                       ERR_error_string(err_code, NULL));
            uts_logger(ct, LOG_ERR, "error '%s' in OpenSSL component '%s'",
                       ERR_reason_error_string(err_code),
                       ERR_lib_error_string(err_code));
        }
        err_code_prev = err_code;
    }
    // free the TS_RESP_CTX if initialization has faile
    TS_RESP_CTX_free(resp_ctx);
    return NULL;
}

int create_response(rfc3161_context *ct, char *query, int query_len,
                    TS_RESP_CTX *resp_ctx, size_t *resp_size,
                    unsigned char **resp, char **serial_id) {
    int ret = 0;
    TS_RESP *ts_response = NULL;
    BIO *query_bio = NULL;
    BIO *out_bio = NULL;
    BIO *status_bio = BIO_new(BIO_s_mem());
    ;
    unsigned long err_code;
    unsigned long err_code_prev = 0;

    // create the input bio for OpenSSL containing the query
    if ((query_bio = BIO_new_mem_buf(query, query_len)) == NULL) {
        uts_logger(ct, LOG_ERR, "failed to parse query");
        goto end;
    }

    // generate the response
    if ((ts_response = TS_RESP_create_response(resp_ctx, query_bio)) == NULL) {
        uts_logger(ct, LOG_ERR, "failed to create ts response");
        goto end;
    }

    // put the reponse data in *resp char *
    FILE *stream = open_memstream((char **)resp, (size_t *)resp_size);
    ret = i2d_TS_RESP_fp(stream, ts_response);
    fflush(stream);
    fclose(stream);
    ret = 1;

end:
    BIO_free_all(query_bio);

    // recover some status and error messages
    BUF_MEM *bptr;
    TS_STATUS_INFO_print_bio(status_bio, TS_RESP_get_status_info(ts_response));
    BIO_get_mem_ptr(status_bio, &bptr);

    char *serial_hex = NULL;
    *serial_id = calloc(SERIAL_ID_SIZE + 1, sizeof(char));

#ifdef OPENSSL_API_1_1
    // if we have a proper response, we recover the serial to identify the logs
    if (TS_RESP_get_tst_info(ts_response) != NULL &&
        TS_TST_INFO_get_serial(TS_RESP_get_tst_info(ts_response)) != NULL) {
        const ASN1_INTEGER *serial =
            TS_TST_INFO_get_serial(TS_RESP_get_tst_info(ts_response));
        BIGNUM *serial_bn = ASN1_INTEGER_to_BN(serial, NULL);
        serial_hex = BN_bn2hex(serial_bn);
        BN_free(serial_bn);
    } else {
        serial_hex = calloc(SERIAL_ID_SIZE, sizeof(char));
        strncpy(serial_hex, " NO ID   ", SERIAL_ID_SIZE + 2);
    }
#endif
#ifdef OPENSSL_API_1_0
    if (ts_response->tst_info != NULL &&
        ts_response->tst_info->serial != NULL) {
        ASN1_INTEGER *serial = ts_response->tst_info->serial;
        BIGNUM *serial_bn = ASN1_INTEGER_to_BN(serial, NULL);
        serial_hex = BN_bn2hex(serial_bn);
        BN_free(serial_bn);
    } else {
        serial_hex = calloc(SERIAL_ID_SIZE, sizeof(char));
        strncpy(serial_hex, " NO ID   ", SERIAL_ID_SIZE + 2);
    }
#endif

    // get a short version of the serial (150 bits in hexa is a bit long)
    strncpy(*serial_id, serial_hex, SERIAL_ID_SIZE);

    // log the full serial as a debug message
    uts_logger(ct, LOG_DEBUG, "Request[%s], timestamp full serial: %s",
               *serial_id, serial_hex);

    free(serial_hex);

    // replacing '\n' by '|' to log on one line only
    char *temp = strstr(bptr->data, "\n");
    while (temp != NULL && (temp = strstr(bptr->data, "\n")) != NULL) {
        temp[0] = '|';
    }
    // log the full responce status, info and failure info
    uts_logger(ct, LOG_DEBUG, "Request[%s], TimeStamp OpenSSL status: |%s",
               *serial_id, bptr->data);

// emit logs according the return value
// and set the return code
#ifdef OPENSSL_API_1_1
    long status = ASN1_INTEGER_get(
        TS_STATUS_INFO_get0_status(TS_RESP_get_status_info(ts_response)));
#endif
#ifdef OPENSSL_API_1_0
    long status = ASN1_INTEGER_get(ts_response->status_info->status);
#endif
    switch (status) {
    case TS_STATUS_GRANTED:
        uts_logger(ct, LOG_DEBUG, "Request[%s], timestamp request granted",
                   *serial_id);
        ret = 200;
        break;
    case TS_STATUS_GRANTED_WITH_MODS:
        uts_logger(ct, LOG_NOTICE, "timestamp request granted with "
                                   "modification",
                   *serial_id);
        ret = 200;
        break;
    case TS_STATUS_REJECTION:
        uts_logger(ct, LOG_WARNING, "Request[%s], timestamp request rejected",
                   *serial_id);
        ret = 400;
        break;
    case TS_STATUS_WAITING:
        uts_logger(ct, LOG_NOTICE, "Request[%s], timestamp request waiting",
                   *serial_id);
        ret = 400;
        break;
    case TS_STATUS_REVOCATION_WARNING:
        uts_logger(ct, LOG_WARNING,
                   "Request[%s], timestamp request revocation warning",
                   *serial_id);
        ret = 200;
        break;
    case TS_STATUS_REVOCATION_NOTIFICATION:
        uts_logger(ct, LOG_NOTICE,
                   "Request[%s], timestamp request revovation notification",
                   *serial_id);
        ret = 500;
        break;
    default:
        uts_logger(ct, LOG_ERR, "Request[%s], unknown error code '%d'", status,
                   *serial_id);
        ret = 500;
    }

    // log the OpenSSL errors if any
    while ((err_code = ERR_get_error())) {
        if (err_code_prev != err_code) {
            ERR_load_TS_strings();
            uts_logger(ct, LOG_DEBUG, "OpenSSL exception: '%s'",
                       ERR_error_string(err_code, NULL));
            uts_logger(ct, LOG_ERR, "error '%s' in OpenSSL component '%s'",
                       ERR_reason_error_string(err_code),
                       ERR_lib_error_string(err_code));
        }
        err_code_prev = err_code;
    }

    // some cleaning
    BIO_free_all(status_bio);
    TS_RESP_free(ts_response);
    return ret;
}
