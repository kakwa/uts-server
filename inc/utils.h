#include "rfc3161.h"

void skeleton_daemon();
void uts_logger(rfc3161_context *ct, int priority, char *fmt, ...);
int set_params(rfc3161_context *ct, char *conf_file);
