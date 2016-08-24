#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <stdarg.h>
#include "context.h"

void skeleton_daemon() {
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    // TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x > 0; x--) {
        close(x);
    }

    /* Open the log file */
    openlog("uts-server", LOG_PID, LOG_DAEMON);
}

void logger(rfc3161_context *ct, int priority, char *fmt, ...) {
    // ignore all messages less critical than the loglevel
    // except if the debug flag is set
    if (priority > ct->loglevel && !ct->stdout_dbg)
        return;
    FILE *stream;
    char *out;
    size_t len;
    stream = open_memstream(&out, &len);
    va_list args;

    va_start(args, fmt);
    vfprintf(stream, fmt, args);
    va_end(args);
    fflush(stream);
    fclose(stream);
    if (ct->stdout_dbg) {
        switch (priority) {
        case LOG_EMERG:
            printf("LOG_EMER   : %s", out);
            ;
        case LOG_ALERT:
            printf("LOG_ALERT  : %s", out);
            ;
        case LOG_CRIT:
            printf("LOG_CRIT   : %s", out);
            ;
        case LOG_ERR:
            printf("LOG_ERR    : %s", out);
            ;
        case LOG_WARNING:
            printf("LOG_WARNING: %s", out);
            ;
        case LOG_NOTICE:
            printf("LOG_NOTICE : %s", out);
            ;
        case LOG_INFO:
            printf("LOG_INFO   : %s", out);
            ;
        case LOG_DEBUG:
            printf("LOG_DEBUG  : %s", out);
            ;
        }
    }
    syslog(priority, out);
    free(out);
}
