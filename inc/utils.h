#include "rfc3161.h"

typedef struct _code {
    char *c_name;
    int c_val;
} CODE;

static void signal_handler_general(int sig_num);
static void signal_handler_up(int sig_num);
void set_sig_handler();
void skeleton_daemon();
int init_pid(char *pidfile_path);
int write_pid(char *pidfile_path);
void uts_logger(rfc3161_context *ct, int priority, char *fmt, ...);
void log_hex(rfc3161_context *ct, int priority, char *id,
             unsigned char *content, int content_length);
int set_params(rfc3161_context *ct, char *conf_file, char *conf_wd);
static char *rand_string(char *str, size_t size);
void free_uts_context(rfc3161_context *ct);
const char *null_undef(const char *in);

// some global variable to handle signals
int g_uts_sig_up;
int g_uts_sig;
