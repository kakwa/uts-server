#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h> /* for offsetof() macro */
#include <string.h>
#include <openssl/ts.h>

typedef struct {
    uint64_t query_counter;
    bool stdout_dbg;
    TS_RESP_CTX *resp_ctx;
    int loglevel;
} rfc3161_context;
