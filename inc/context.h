#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h> /* for offsetof() macro */
#include <string.h>
#include <civetweb.h>
#include <openssl/ts.h>

#define HTTP_OPTIONS 1
#define LOGLEVEL_OPTIONS 2
#define TSA_OPTIONS 3
#define MAIN_CONF_SECTION "main"

#define RFC3161_OPTIONS_LEN                                                    \
    sizeof(rfc3161_options) / sizeof(struct rfc3161_option)

typedef struct {
    uint64_t query_counter;
    bool stdout_dbg;
    TS_RESP_CTX *resp_ctx;
    int loglevel;
    const char *http_options[40];
    TS_RESP_CTX *ts_ctx;
} rfc3161_context;

struct rfc3161_option {
    const char *name;
    int type;
    const char *default_value;
};

static struct rfc3161_option rfc3161_options[] = {
    {"num_threads", HTTP_OPTIONS, "50"},
    {"run_as_user", HTTP_OPTIONS, NULL},
    {"throttle", HTTP_OPTIONS, NULL},
    {"enable_keep_alive", HTTP_OPTIONS, "no"},
    {"listening_ports", HTTP_OPTIONS, "8080"},
    {"request_timeout_ms", HTTP_OPTIONS, "30000"},
    {"ssl_certificate", HTTP_OPTIONS, NULL},
    {"ssl_verify_peer", HTTP_OPTIONS, "yes"},
    {"ssl_ca_path", HTTP_OPTIONS, NULL},
    {"ssl_ca_file", HTTP_OPTIONS, NULL},
    {"ssl_verify_depth", HTTP_OPTIONS, "9"},
    {"ssl_default_verify_paths", HTTP_OPTIONS, "yes"},
    {"ssl_cipher_list", HTTP_OPTIONS, NULL},
    {"ssl_protocol_version", HTTP_OPTIONS, "0"},
    {"ssl_short_trust", HTTP_OPTIONS, "no"},
    {"access_control_allow_origin", HTTP_OPTIONS, "*"},
    {"tcp_nodelay", HTTP_OPTIONS, "0"},
    {"log_level", LOGLEVEL_OPTIONS, "info"},
};
