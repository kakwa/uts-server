#include <stdbool.h>
#include "utils.h"

struct tuser_data {
	    char *first_message;
};

int http_server_start(char *conffile, char *conf_wd, bool stdout_dbg);
