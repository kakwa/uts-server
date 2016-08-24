#include <stdlib.h>
#include <stdint.h>
#include <stddef.h> /* for offsetof() macro */
#include <string.h>


typedef struct {
    uint64_t query_counter;
} rfc3161_context;
