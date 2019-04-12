#include <civetweb.h>
#include <openssl/ts.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h> /* for offsetof() macro */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HTTP_OPTIONS 1
#define LOGLEVEL_OPTIONS 2
#define LOGHANDLER_OPTIONS 3
#define TSA_OPTIONS 4
#define PATH_HTTP_OPTIONS 5

/* name of the configuration file section */
#define MAIN_CONF_SECTION "main"
#define OID_SECTION "oids"
#define TSA_SECTION "tsa"

#define RFC3161_OPTIONS_LEN                                                    \
    sizeof(rfc3161_options) / sizeof(struct rfc3161_option)

typedef struct {
    TS_RESP_CTX *ts_ctx;
    bool available;
    pthread_mutex_t lock;
} ts_resp_ctx_wrapper;

typedef struct {
    uint64_t query_counter;
    // flag for debugging
    bool stdout_dbg;
    // flag for logging to stdout
    bool stdout_logging;
    // flag for logging to stdout
    bool syslog_logging;
    // log level
    int loglevel;
    // number of threads
    int numthreads;
    // civetweb http parameters
    const char *http_options[40];
    ts_resp_ctx_wrapper *ts_ctx_pool;

    // just to track for freeing later
    CONF *conf;
    char *cust_conf[20];
    char *ca_file;
} rfc3161_context;

// definition of structure to describe
// section [ main ] attributes (name, type and default value)
struct rfc3161_option {
    const char *name;
    int type;
    const char *default_value;
};

// declaration of section [ main ] parameters and their default values
static struct rfc3161_option rfc3161_options[] = {
    {"num_threads", HTTP_OPTIONS, "10"},
    {"run_as_user", HTTP_OPTIONS, NULL},
    {"throttle", HTTP_OPTIONS, NULL},
    {"enable_keep_alive", HTTP_OPTIONS, "no"},
    {"listening_ports", HTTP_OPTIONS, "8080"},
    {"request_timeout_ms", HTTP_OPTIONS, "30000"},
    {"ssl_verify_peer", HTTP_OPTIONS, "yes"},
    {"ssl_verify_depth", HTTP_OPTIONS, "9"},
    {"ssl_default_verify_paths", HTTP_OPTIONS, "yes"},
    {"ssl_cipher_list", HTTP_OPTIONS, NULL},
    {"ssl_protocol_version", HTTP_OPTIONS, "0"},
    {"ssl_short_trust", HTTP_OPTIONS, "no"},
    {"access_control_allow_origin", HTTP_OPTIONS, "*"},
    {"tcp_nodelay", HTTP_OPTIONS, "0"},
    {"log_level", LOGLEVEL_OPTIONS, "info"},
    {"log_to_syslog", LOGHANDLER_OPTIONS, "yes"},
    {"log_to_stdout", LOGHANDLER_OPTIONS, "no"},
    {"ssl_certificate", PATH_HTTP_OPTIONS, NULL},
    {"ssl_ca_path", PATH_HTTP_OPTIONS, NULL},
    {"ssl_ca_file", PATH_HTTP_OPTIONS, NULL},
};
