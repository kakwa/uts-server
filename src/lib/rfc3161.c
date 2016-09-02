/*
 *    "This product includes software developed by the OpenSSL Project
 *     *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <civetweb.h>
#include <openssl/opensslconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/syslog.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/ts.h>
#include <openssl/bn.h>
#include <syslog.h>
#include "utils.h"

#define OID_SECTION "oids"

// number of char we get to log for the serial
#define SERIAL_ID_SIZE 8

/* Reply related functions. */
static int reply_command(CONF *conf, char *section, char *engine, char *query,
                         char *passin, char *inkey, const EVP_MD *md,
                         char *signer, char *chain, const char *policy,
                         char *in, int token_in, char *out, int token_out,
                         int text);
static ASN1_INTEGER *serial_cb(TS_RESP_CTX *ctx, void *data);

#define B_FORMAT_TEXT 0x8000
#define FORMAT_UNDEF 0
#define FORMAT_TEXT (1 | B_FORMAT_TEXT) /* Generic text */
#define FORMAT_ASN1 4                   /* ASN.1/DER */

/*
 * Reply-related method definitions.
 */

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

void init_ssl(){
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    SSL_library_init();
    ERR_load_TS_strings();
}

void free_ssl() {
    ERR_free_strings();
}

TS_RESP_CTX *create_tsctx(rfc3161_context *ct, CONF *conf, const char *section,
                          const char *policy) {
    unsigned long err_code;
    unsigned long err_code_prev = 0;
    TS_RESP_CTX *resp_ctx = NULL;

    if ((section = TS_CONF_get_tsa_section(conf, section)) == NULL) {
        uts_logger(ct, LOG_ERR, "failed to get or use '%s' in section [ %s ]",
                   "default_tsa", "tsa");
        goto end;
    }
    if ((resp_ctx = TS_RESP_CTX_new()) == NULL) {
        uts_logger(ct, LOG_ERR, "failed to initialize tsa context");
        goto end;
    }
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

    if ((query_bio = BIO_new_mem_buf(query, query_len)) == NULL) {
        uts_logger(ct, LOG_ERR, "failed to parse query");
        goto end;
    }
    if ((ts_response = TS_RESP_create_response(resp_ctx, query_bio)) == NULL) {
        uts_logger(ct, LOG_ERR, "failed to create ts response");
        goto end;
    }

    FILE *stream = open_memstream((char **)resp, (size_t *)resp_size);
    ret = i2d_TS_RESP_fp(stream, ts_response);
    fflush(stream);
    fclose(stream);
    ret = 1;

end:
    BIO_free_all(query_bio);

    // recover some status and error messages
    BUF_MEM *bptr;
    TS_STATUS_INFO_print_bio(status_bio, ts_response->status_info);
    BIO_get_mem_ptr(status_bio, &bptr);

    ASN1_INTEGER *serial = ts_response->tst_info->serial;
    BIGNUM *serial_bn = ASN1_INTEGER_to_BN(serial, NULL);
    char *serial_hex = BN_bn2hex(serial_bn);
    BN_free(serial_bn);
    *serial_id = calloc(SERIAL_ID_SIZE + 1, sizeof(char));
    strncpy(*serial_id, serial_hex, SERIAL_ID_SIZE);

    // replacing '\n' by '|' to log on one line only
    char *temp = strstr(bptr->data, "\n");
    while (temp != NULL && (temp = strstr(bptr->data, "\n")) != NULL) {
        temp[0] = '|';
    }
    uts_logger(ct, LOG_DEBUG,
               "timestamp full serial: %s (response serial '%s...')",
               serial_hex, *serial_id);
    uts_logger(ct, LOG_DEBUG,
               "TimeStamp OpenSSL status: |%s (response serial '%s...')",
               bptr->data, *serial_id);

    long status = ASN1_INTEGER_get(ts_response->status_info->status);
    switch (status) {
    case TS_STATUS_GRANTED:
        uts_logger(ct, LOG_INFO,
                   "timestamp request granted (response serial '%s...')",
                   *serial_id);
        ret = 1;
        break;
    case TS_STATUS_GRANTED_WITH_MODS:
        uts_logger(ct, LOG_NOTICE, "timestamp request granted with "
                                   "modification (response serial '%s...')",
                   *serial_id);
        ret = 1;
        break;
    case TS_STATUS_REJECTION:
        uts_logger(ct, LOG_WARNING,
                   "timestamp request rejected (response serial '%s...')",
                   *serial_id);
        ret = 0;
        break;
    case TS_STATUS_WAITING:
        uts_logger(ct, LOG_NOTICE,
                   "timestamp request waiting (response serial '%s...')",
                   *serial_id);
        ret = 0;
        break;
    case TS_STATUS_REVOCATION_WARNING:
        uts_logger(
            ct, LOG_WARNING,
            "timestamp request revocation warning (response serial '%s...')",
            *serial_id);
        ret = 0;
        break;
    case TS_STATUS_REVOCATION_NOTIFICATION:
        uts_logger(ct, LOG_NOTICE, "timestamp request revovation notification "
                                   "(response serial '%s...')",
                   *serial_id);
        ret = 0;
        break;
    default:
        uts_logger(ct, LOG_ERR,
                   "unknown error code '%d' (response serial '%s...')", status,
                   *serial_id);
        ret = 0;
    }

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
    BIO_free_all(status_bio);
    TS_RESP_free(ts_response);
    free(serial_hex);
    return ret;
}

static ASN1_INTEGER *serial_cb(TS_RESP_CTX *ctx, void *data42) {
    unsigned char data[150] = {0};
    RAND_bytes(data, sizeof(data));
    // data[0] &= 0x7F;

    // build big number from our bytes
    BIGNUM *bn = BN_new();
    BN_bin2bn(data, sizeof(data), bn);

    // build the ASN1_INTEGER from our BIGNUM
    ASN1_INTEGER *asnInt = ASN1_INTEGER_new();
    BN_to_ASN1_INTEGER(bn, asnInt);

    // cleanup
    // ASN1_INTEGER_free(asnInt);
    BN_free(bn);
    return asnInt;
}
