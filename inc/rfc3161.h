#include <openssl/opensslconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/syslog.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/ts.h>
#include <openssl/bn.h>
#include "context.h"

/* Name of config entry that defines the OID file. */
#define OID_SECTION "oids"
#define TSA_SECTION "tsa"

// number of char we get to log for the serial
#define SERIAL_ID_SIZE 8

#define B_FORMAT_TEXT 0x8000
#define FORMAT_UNDEF 0
#define FORMAT_TEXT (1 | B_FORMAT_TEXT) /* Generic text */
#define FORMAT_ASN1 4                   /* ASN.1/DER */

static ASN1_OBJECT *txt2obj(const char *oid);

/* Reply related functions. */
static ASN1_INTEGER *serial_cb(TS_RESP_CTX *ctx, void *data);
static TS_RESP *read_PKCS7(BIO *in_bio);
int create_response(rfc3161_context *ct, char *query, int query_len,
                    TS_RESP_CTX *resp_ctx, size_t *resp_size,
                    unsigned char **resp, char **serial_id);
static ASN1_INTEGER *serial_cb(TS_RESP_CTX *ctx, void *data);
static ASN1_INTEGER *next_serial(const char *serialfile);
static int save_ts_serial(const char *serialfile, ASN1_INTEGER *serial);
TS_RESP_CTX *create_tsctx(rfc3161_context *ct, CONF *conf, const char *section,
                          const char *policy);
int add_oid_section(rfc3161_context *ct, CONF *conf);
ts_resp_ctx_wrapper *get_ctxw(rfc3161_context *ct);
void init_ssl();
void free_ssl();
