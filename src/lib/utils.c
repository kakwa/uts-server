#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <stdarg.h>
#include <openssl/bio.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include "utils.h"

typedef struct _code {
    char *c_name;
    int c_val;
} CODE;

CODE prioritynames[] = {{"alert", LOG_ALERT},
                        {"crit", LOG_CRIT},
                        {"debug", LOG_DEBUG},
                        {"emerg", LOG_EMERG},
                        {"err", LOG_ERR},
                        {"error", LOG_ERR},
                        {"info", LOG_INFO},
                        {"notice", LOG_NOTICE},
                        {"panic", LOG_EMERG},
                        {"warn", LOG_WARNING},
                        {"warning", LOG_WARNING},
                        {NULL, -1}};

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
            printf("LOG_EMER   : %s\n", out);
            break;
        case LOG_ALERT:
            printf("LOG_ALERT  : %s\n", out);
            break;
            ;
        case LOG_CRIT:
            printf("LOG_CRIT   : %s\n", out);
            break;
            ;
        case LOG_ERR:
            printf("LOG_ERR    : %s\n", out);
            break;
            ;
        case LOG_WARNING:
            printf("LOG_WARNING: %s\n", out);
            break;
            ;
        case LOG_NOTICE:
            printf("LOG_NOTICE : %s\n", out);
            break;
            ;
        case LOG_INFO:
            printf("LOG_INFO   : %s\n", out);
            break;
            ;
        case LOG_DEBUG:
            printf("LOG_DEBUG  : %s\n", out);
            break;
            ;
        }
    }
    syslog(priority, out);
    free(out);
}

static BIO *bio_open_default(rfc3161_context *ct, const char *filename,
                             int format) {
    BIO *ret;
    format = FORMAT_TEXT;

    if (filename == NULL || strcmp(filename, "-") == 0) {
        logger(ct, LOG_CRIT, "Can't open %s, %s", filename, strerror(errno));
        return NULL;
    } else {
        ret = BIO_new_file(filename, "rb");
        if (ret != NULL)
            return ret;
        logger(ct, LOG_CRIT, "Can't open %s for %s, %s", filename, "rb",
               strerror(errno));
    }
    // ERR_print_errors(bio_err);
    return NULL;
}

static CONF *load_config_file(rfc3161_context *ct, const char *filename) {
    long errorline = -1;
    BIO *in;
    CONF *conf;
    int i;
    ct->loglevel = LOG_INFO;
    if (filename == NULL) {
        logger(ct, LOG_WARNING, "no configuration file passed");
        return NULL;
    }
    in = bio_open_default(ct, filename, 'r');
    if (in == NULL) {
        logger(ct, LOG_CRIT, "Can't load config file \"%s\"", filename);
        return NULL;
    }

    conf = NCONF_new(NULL);
    i = NCONF_load_bio(conf, in, &errorline);
    BIO_free(in);
    if (i > 0) {
        return conf;
    }
    if (errorline <= 0)
        logger(ct, LOG_CRIT, "Can't load config file \"%s\"", filename);
    else
        logger(ct, LOG_CRIT, "Error on line %ld of config file \"%s\"",
               errorline, filename);
    NCONF_free(conf);
    return NULL;
}

int set_params(rfc3161_context *ct, char *conf_file) {
    int ret = 0;
    CONF *conf = load_config_file(ct, conf_file);
    ret = 1;
    int http_counter = 0;
    // first pass to set the loglevel as soon as possible
    for (int i = 0; i < RFC3161_OPTIONS_LEN; i++) {
        int type = rfc3161_options[i].type;
        const char *name = rfc3161_options[i].name;
        const char *default_value = rfc3161_options[i].default_value;
        const char *value = NCONF_get_string(conf, MAIN_CONF_SECTION, name);
        switch (type) {
        case LOGLEVEL_OPTIONS:
            for (int j = 0;; j++) {
                if (prioritynames[j].c_name == NULL)
                    break;
                if (strcmp(prioritynames[j].c_name, value))
                    ct->loglevel = prioritynames[j].c_val;
            }
            break;
            ;
        }
    }
    for (int i = 0; i < RFC3161_OPTIONS_LEN; i++) {
        int type = rfc3161_options[i].type;
        const char *name = rfc3161_options[i].name;
        const char *default_value = rfc3161_options[i].default_value;
        const char *value = NCONF_get_string(conf, MAIN_CONF_SECTION, name);
        if (value == NULL) {
            logger(ct, LOG_NOTICE,
                   "configuration param['%s'] not set, using default: '%s'",
                   name, default_value);
            value = default_value;
        }
        logger(ct, LOG_DEBUG, "configuration param['%s'] = '%s'", name, value);
        switch (type) {
        case HTTP_OPTIONS:
            if (value != NULL) {
                ct->http_options[http_counter] = name;
                http_counter++;
                ct->http_options[http_counter] = value;
                http_counter++;
            }
            break;
            ;
        case TSA_OPTIONS:
            break;
            ;
        }
        ct->http_options[http_counter] = NULL;
    }

    ct->ts_ctx = create_tsctx(conf, "tsa", NULL);
    if (ct->ts_ctx == NULL)
        ret = 0;
    return ret;

end:
    return 0;
}
