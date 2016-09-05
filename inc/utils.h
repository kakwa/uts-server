#include "rfc3161.h"

void skeleton_daemon();
int init_pid(char *pidfile_path);
int write_pid(char *pidfile_path);
void uts_logger(rfc3161_context *ct, int priority, char *fmt, ...);
void log_hex(rfc3161_context *ct, int priority, char *id,
             unsigned char *content, int content_length);
int set_params(rfc3161_context *ct, char *conf_file, char *conf_wd);
static char *rand_string(char *str, size_t size);
void free_uts_context(rfc3161_context *context);
int g_uts_sig_up;
int g_uts_sig;
