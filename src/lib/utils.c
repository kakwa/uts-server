#include "utils.h"
#include <errno.h>
#include <fcntl.h>
#include <openssl/bio.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <syslog.h>
#include <unistd.h>

static void signal_handler_general(int sig_num) {
    g_uts_sig = sig_num;
}

static void signal_handler_up(int sig_num) {
    g_uts_sig_up = sig_num;
}

CODE prioritynames[] = {
    {"alert", LOG_ALERT},  {"crit", LOG_CRIT},       {"debug", LOG_DEBUG},
    {"emerg", LOG_EMERG},  {"err", LOG_ERR},         {"error", LOG_ERR},
    {"info", LOG_INFO},    {"notice", LOG_NOTICE},   {"panic", LOG_EMERG},
    {"warn", LOG_WARNING}, {"warning", LOG_WARNING}, {NULL, -1}};

int init_pid(char *pidfile_path) {
    // if pidfile_path is null, the user did not request one
    // exit success
    if (pidfile_path == NULL)
        return 1;

    int fd = open(pidfile_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    // in case we can't open it
    if (fd == -1) {
        syslog(LOG_CRIT, "failed to open the pid file");
        return 0;
    }

    close(fd);
    return 1;
}

int write_pid(char *pidfile_path) {
    // if pidfile_path is null, the user did not request one
    // exit success
    if (pidfile_path == NULL)
        return 1;

    int fd = open(pidfile_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    // in case we can't open it
    if (fd == -1) {
        syslog(LOG_CRIT, "failed to open the pid file");
        return 0;
    }

    char buf[100];
    snprintf(buf, 100, "%ld\n", (long)getpid());
    if (write(fd, buf, strlen(buf)) != strlen(buf)) {
        syslog(LOG_CRIT, "failed to write the pid");
        close(fd);
        return 0;
    }
    close(fd);
    return 1;
}

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

    g_uts_sig_up = 0;
    g_uts_sig = 0;
    /* Catch, ignore and handle signals */
    // TODO: Implement a working signal handler */
    signal(SIGTERM, signal_handler_general);
    signal(SIGINT, signal_handler_general);
    signal(SIGHUP, signal_handler_up);
    signal(SIGCHLD, SIG_IGN);

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
    // openlog("uts-server", LOG_PID, LOG_DAEMON);
}

// log a binary blob as hexadecimal
void log_hex(rfc3161_context *ct, int priority, char *id,
             unsigned char *content, int content_length) {
    if (priority > ct->loglevel && !ct->stdout_dbg)
        return;
    FILE *stream;
    char *out;
    size_t len;
    stream = open_memstream(&out, &len);

    for (int i = 0; i < content_length; i++) {
        fprintf(stream, "%02x", content[i]);
    }
    fflush(stream);
    fclose(stream);
    uts_logger(ct, priority, "%s: %s", id, out);
    free(out);
}

// logger function
void uts_logger(rfc3161_context *ct, int priority, char *fmt, ...) {
    // ignore all messages less critical than the loglevel
    // except if the debug flag is set
    if (priority > ct->loglevel && !ct->stdout_dbg)
        return;

    // build the out log message
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

    // if in debugging mode, also log to stdout
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
    syslog(priority, "%s", out);
    free(out);
}

// OpenSSL file openner (use for opening the configuration file
static BIO *bio_open_default(rfc3161_context *ct, const char *filename,
                             int format) {
    BIO *ret;
    format = FORMAT_TEXT;

    if (filename == NULL || strcmp(filename, "-") == 0) {
        uts_logger(ct, LOG_CRIT, "Can't open %s, %s", filename,
                   strerror(errno));
        return NULL;
    } else {
        ret = BIO_new_file(filename, "rb");
        if (ret != NULL)
            return ret;
        uts_logger(ct, LOG_CRIT, "Can't open %s for %s, %s", filename, "rb",
                   strerror(errno));
    }
    // ERR_print_errors(bio_err);
    return NULL;
}

// loading the configuration file and parsing it using the OpenSSL parser
static CONF *load_config_file(rfc3161_context *ct, const char *filename) {
    long errorline = -1;
    BIO *in;
    CONF *conf;
    int i;
    ct->loglevel = LOG_INFO;
    if (filename == NULL) {
        uts_logger(ct, LOG_WARNING, "no configuration file passed");
        return NULL;
    }
    in = bio_open_default(ct, filename, 'r');
    if (in == NULL) {
        uts_logger(ct, LOG_CRIT, "Can't load config file \"%s\"", filename);
        return NULL;
    }

    conf = NCONF_new(NULL);
    i = NCONF_load_bio(conf, in, &errorline);
    BIO_free(in);
    if (i > 0) {
        return conf;
    }
    if (errorline <= 0)
        uts_logger(ct, LOG_CRIT, "Can't load config file \"%s\"", filename);
    else
        uts_logger(ct, LOG_CRIT, "Error on line %ld of config file \"%s\"",
                   errorline, filename);
    NCONF_free(conf);
    return NULL;
}

// initialize the rfc3161_context according to the conf_file content
int set_params(rfc3161_context *ct, char *conf_file, char *conf_wd) {
    // chdir in configuration file directory
    // (some parameters like certificates can be declared
    // relatively to the configuration file).
    chdir(conf_wd);
    int ret = 1;
    int http_counter = 0;
    int cust_counter = 0;
    int numthreads = 42;

    NCONF_free(ct->conf);
    // load the configuration file
    ct->conf = load_config_file(ct, conf_file);
    if (ct->conf == NULL)
        goto end;

    // first pass to set the loglevel as soon as possible
    for (int i = 0; i < RFC3161_OPTIONS_LEN; i++) {
        int type = rfc3161_options[i].type;
        const char *name = rfc3161_options[i].name;
        const char *default_value = rfc3161_options[i].default_value;
        const char *value = NCONF_get_string(ct->conf, MAIN_CONF_SECTION, name);
        if (value == NULL) {
            uts_logger(ct, LOG_NOTICE,
                       "configuration param['%s'] not set, using default: '%s'",
                       name, default_value);
            value = default_value;
        }
        switch (type) {
        case LOGLEVEL_OPTIONS:
            for (int j = 0;; j++) {
                if (prioritynames[j].c_name == NULL)
                    break;
                if (strcmp(prioritynames[j].c_name, value) == 0) {
                    ct->loglevel = prioritynames[j].c_val;
                    break;
                }
            }
            break;
            ;
        }
    }
    // parse the options to get the civetweb options and a few other things
    for (int i = 0; i < RFC3161_OPTIONS_LEN; i++) {
        int type = rfc3161_options[i].type;
        const char *name = rfc3161_options[i].name;
        const char *default_value = rfc3161_options[i].default_value;
        const char *value = NCONF_get_string(ct->conf, MAIN_CONF_SECTION, name);
        if (value == NULL) {
            uts_logger(ct, LOG_NOTICE,
                       "configuration param['%s'] not set, using default: '%s'",
                       name, default_value);
            value = default_value;
        }
        uts_logger(ct, LOG_DEBUG, "configuration param['%s'] = '%s'", name,
                   value);
        switch (type) {
        // if it's an http (civetweb) option, put it in the http_options buffer
        // like civetweb is expected it.
        case HTTP_OPTIONS:
            if (value != NULL) {
                ct->http_options[http_counter] = name;
                http_counter++;
                ct->http_options[http_counter] = value;
                http_counter++;
            }
            // recover the num_threads parameter as it's used to
            // initialize the TS_RESP_CTX pool
            if (strcmp(name, "num_threads") == 0)
                numthreads = atoi(value);
            break;
            ;
        // if it's a path, resolve the full path first
        // and put it in the http_options buffer
        case PATH_HTTP_OPTIONS:
            if (value != NULL) {
                char *ptr = NULL;
                ptr = realpath(value, NULL);
                ct->http_options[http_counter] = name;
                http_counter++;
                ct->http_options[http_counter] = ptr;
                http_counter++;
                ct->cust_conf[cust_counter] = ptr;
                cust_counter++;
            }
            break;
            ;
        }
        ct->http_options[http_counter] = NULL;
    }

    if (!add_oid_section(ct, ct->conf))
        ret = 0;
    // initialize the TS_RESP_CTX pool
    // as TS_RESP_CTX is not thread safe,
    // creates 'num_threads' TS_RESP_CTX (one per thread)
    ct->ts_ctx_pool = calloc(numthreads, sizeof(ts_resp_ctx_wrapper));
    ct->numthreads = numthreads;
    for (int i = 0; i < numthreads; i++) {
        ct->ts_ctx_pool[i].ts_ctx =
            create_tsctx(ct, ct->conf, TSA_SECTION, NULL);
        ct->ts_ctx_pool[i].available = 1;
        if (ct->ts_ctx_pool[i].ts_ctx == NULL) {
            ret = 0;
            break;
        }
    }
    // like any good daemon, return to '/' once the configuration is loaded
    chdir("/");
    return ret;

end:
    chdir("/");
    return 0;
}

// free the rfc3161_context structure
void free_uts_context(rfc3161_context *ct) {
    for (int i = 0; i < ct->numthreads; i++) {
        TS_RESP_CTX_free(ct->ts_ctx_pool[i].ts_ctx);
    }
    for (int i = 0; i < 20; i++) {
        free(ct->cust_conf[i]);
    }
    free(ct->ts_ctx_pool);
    NCONF_free(ct->conf);
    free(ct);
}
