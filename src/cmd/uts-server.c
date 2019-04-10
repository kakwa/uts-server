#include "http.h"
#include <argp.h>
#include <ctype.h>
#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#ifdef BSD
#include <sys/syslimits.h>
#else
#include <linux/limits.h>
#endif /* BSD */
#include <stdio.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <unistd.h>

const char *argp_program_version = UTS_VERSION;

const char *argp_program_bug_address =
    "Pierre-Francois Carpentier <carpentier.pf@gmail.com>";

static char doc[] = "\nUTS micro timestamp server (RFC 3161)";

static struct argp_option options[] = {
    {"conffile", 'c', "CONFFILE", 0, "Path to configuration file"},
    {"daemonize", 'd', 0, 0, "Launch as a daemon"},
    {"pidfile", 'p', "PIDFILE", 0, "Path to pid file"},
    {"debug", 'D', 0, 0, "STDOUT debugging"},
    {0}};

/* A description of the arguments we accept. */
static char args_doc[] = "-c CONFFILE [-d] [-D] [-p <pidfile>]";

struct arguments {
    char *args[2]; /* arg1 & arg2 */
    int daemonize;
    bool stdout_dbg;
    char *conffile;
    char *pidfile;
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
    case 'p':
        arguments->pidfile = arg;
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
    args.pidfile = NULL;
    args.daemonize = 0;
    args.stdout_dbg = 0;
    argp_parse(&argp, argc, argv, 0, 0, &args);
    int ret = EXIT_SUCCESS;

    // get the full path of the configuration (daemon -> chdir to / concequently
    // full path is necessary)
    char conf_fp[PATH_MAX];
    if (realpath(args.conffile, conf_fp) == NULL) {
        syslog(LOG_CRIT, "unable to get the full path of the configuration "
                         "file, uts-server start failed");
        return EXIT_FAILURE;
    }

    init_pid(args.pidfile);
    // get the full path for the pid file
    char pid_file[PATH_MAX];
    if ((args.pidfile != NULL) && realpath(args.pidfile, pid_file) == NULL) {
        syslog(LOG_CRIT, "unable to get the full path of the pid "
                         "file, uts-server start failed");
        return EXIT_FAILURE;
    }

    // get the directory containing the configuration file
    // other uts-server files (ca, certs, etc) can be declared relatively to the
    // configuration file
    char *tmp_wd = strdup(conf_fp);
    char *conf_wd = dirname(tmp_wd);

    if (args.daemonize)
        skeleton_daemon();
    else
        set_sig_handler();

    syslog(LOG_NOTICE,
           "uts-server daemon starting with conf '%s' from working dir '%s'",
           conf_fp, conf_wd);

    if (args.pidfile != NULL) {
        if (write_pid(pid_file) == 0) {
            syslog(LOG_CRIT, "failed to write pid file '%s'", pid_file);
            return EXIT_FAILURE;
        }
    }

    while (1) {
        ret = http_server_start(conf_fp, conf_wd, args.stdout_dbg);
        break;
    }

    syslog(LOG_NOTICE, "uts-server daemon terminated.");
    free(tmp_wd);
    closelog();

    return ret;
}
