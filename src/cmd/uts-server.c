#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <argp.h>
#include <sys/syslog.h>
#include "http.h"

const char *argp_program_version = UTS_VERSION;

const char *argp_program_bug_address =
    "Pierre-Francois Carpentier <carpentier.pf@gmail.com>";

static char doc[] = "\nUTS micro timestamp server (RFC 3161)";

static struct argp_option options[] = {
    {"conffile", 'c', "CONFFILE", 0, "Path to configuration file"},
    {"daemonize", 'd', 0, 0, "Launch as a daemon"},
    {"debug", 'D', 0, 0, "STDOUT debugging"},
    {0}};

/* A description of the arguments we accept. */
static char args_doc[] = "-c CONFFILE [-d] [-D]";

struct arguments {
    char *args[2]; /* arg1 & arg2 */
    int daemonize;
    bool stdout_dbg;
    char *conffile;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    /* Get the input argument from argp_parse, which we
       know is a pointer to our arguments structure. */
    struct arguments *arguments = (struct arguments *)state->input;

    switch (key) {
    case 'd':
        arguments->daemonize = 1;
        break;
    case 'D':
        arguments->stdout_dbg = 1;
        break;
    case 'c':
        arguments->conffile = arg;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* Our argp parser. */
static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv) {
    struct arguments args;
    args.conffile = NULL;
    args.daemonize = 0;
    args.stdout_dbg = 0;
    argp_parse(&argp, argc, argv, 0, 0, &args);
    int ret = EXIT_SUCCESS;

    // get the current path, the configuration can be relative to this path
   char conf_wd[PATH_MAX];
   if (getcwd(conf_wd, sizeof(conf_wd)) == NULL){
        syslog(LOG_CRIT, "unable to get the current, uts-server start failed");
	return EXIT_FAILURE;
   }

    if (args.daemonize)
        skeleton_daemon();

    syslog(LOG_NOTICE, "uts-server daemon starting with conf '%s' from working dir '%s'", args.conffile, conf_wd);

    while (1) {
        ret = http_server_start(args.conffile, conf_wd, args.stdout_dbg);
        break;
    }

    syslog(LOG_NOTICE, "uts-server daemon terminated.");
    closelog();

    return ret;
}
