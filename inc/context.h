#include <stdlib.h>
#include <stdint.h>
#include <stddef.h> /* for offsetof() macro */
#include <string.h>
#include <openssl/ts.h>

typedef struct {
    uint64_t query_counter;
    TS_RESP_CTX *resp_ctx;
} rfc3161_context;
