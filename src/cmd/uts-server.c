#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <argp.h>
#include <sys/syslog.h>
#include "rfc3161.h"
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

    if (args.daemonize)
        skeleton_daemon();

    while (1) {
        // TODO: Insert daemon code here.
        http_server_start(args.conffile, args.stdout_dbg);
        syslog(LOG_NOTICE, "uts-server daemon started.");
        break;
    }

    syslog(LOG_NOTICE, "uts-server daemon terminated.");
    closelog();

    return EXIT_SUCCESS;
}
