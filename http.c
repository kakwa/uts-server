#include "http.h"
#include <civetweb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/syslog.h>
#include <time.h>
#include <unistd.h>
#include "http_staticpage.h"

extern int g_uts_sig_up;
extern int g_uts_sig;

static char *rand_string(char *str, size_t size) {
    const char charset[] = "1234567890ABCDEF";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int)(sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

static char *sdup(const char *str) {
    size_t len;
    char *p;

    len = strlen(str) + 1;
    if ((p = (char *)malloc(len)) != NULL) {
        memcpy(p, str, len);
    }
    return p;
}

static int log_civetweb(const struct mg_connection *conn, const char *message) {
    const struct mg_context *ctx = mg_get_context(conn);
    struct tuser_data *ud = (struct tuser_data *)mg_get_user_data(ctx);

    if (ud->first_message == NULL) {
        ud->first_message = sdup(message);
    }

    return 0;
}

void log_request_debug(const struct mg_request_info *request_info,
                       char *request_id, rfc3161_context *context) {
    if (LOG_DEBUG > context->loglevel && !context->stdout_dbg)
        return;

    for (int i = 0; i < request_info->num_headers; i++) {
        uts_logger(context, LOG_DEBUG, "Request[%s], Header[%s]: %s",
                   request_id, request_info->http_headers[i].name,
                   null_undef(request_info->http_headers[i].value));
    }
    uts_logger(context, LOG_DEBUG, "Request[%s], request_method: %s",
               request_id, null_undef(request_info->request_method));
    uts_logger(context, LOG_DEBUG, "Request[%s], request_uri: %s", request_id,
               null_undef(request_info->request_uri));
    uts_logger(context, LOG_DEBUG, "Request[%s], local_uri: %s", request_id,
               null_undef(request_info->local_uri));
    uts_logger(context, LOG_DEBUG, "Request[%s], http_version: %s", request_id,
               null_undef(request_info->http_version));
    uts_logger(context, LOG_DEBUG, "Request[%s], query_string: %s", request_id,
               null_undef(request_info->query_string));
    uts_logger(context, LOG_DEBUG, "Request[%s], remote_addr: %s", request_id,
               null_undef(request_info->remote_addr));
    uts_logger(context, LOG_DEBUG, "Request[%s], is_ssl: %d", request_id,
               request_info->is_ssl);
    uts_logger(context, LOG_DEBUG, "Request[%s], content_length: %d",
               request_id, request_info->content_length);
    uts_logger(context, LOG_DEBUG, "Request[%s], remote_port: %d", request_id,
               request_info->remote_port);
}

void log_request(const struct mg_request_info *request_info, char *request_id,
                 rfc3161_context *context, int response_code, int timer) {
    if (LOG_INFO > context->loglevel && !context->stdout_dbg)
        return;

    const char *user_agent = NULL;
    const char *content_type = NULL;

    for (int i = 0; i < request_info->num_headers; i++) {
        if (strcasecmp(request_info->http_headers[i].name, "User-Agent") == 0) {
            user_agent = request_info->http_headers[i].value;
        }
        if (strcasecmp(request_info->http_headers[i].name, "Content-Type") ==
            0) {
            content_type = request_info->http_headers[i].value;
        }
    }

    uts_logger(context, LOG_INFO,
               "Request[%s], remote_addr[%s] ssl[%d] "
               "uri[%s] http_resp_code[%d] duration[%d us] "
               "user-agent[%s] content-type[%s]",
               request_id, null_undef(request_info->remote_addr),
               request_info->is_ssl, null_undef(request_info->local_uri),
               response_code, timer, null_undef(user_agent),
               null_undef(content_type));
}

int rfc3161_handler(struct mg_connection *conn, void *context) {
    // some timer stuff
    clock_t start = clock(), diff;
    /* Handler may access the request info using mg_get_request_info */
    const struct mg_request_info *request_info = mg_get_request_info(conn);
    rfc3161_context *ct = (rfc3161_context *)context;
    int ret;
    int resp_code;
    ct->query_counter++;
    uint64_t query_id = ct->query_counter;

    bool is_tsq = 0;

    // go through every headers to find Content-Type
    // and check if it's set to "application/timestamp-query"
    // if it's the case, set is_tsq (is time-stamp query) to True
    for (int i = 0; i < request_info->num_headers; i++) {
        const char *h_name = request_info->http_headers[i].name;
        const char *h_value = request_info->http_headers[i].value;
        if (strcasecmp(h_name, "Content-Type") == 0 &&
            strcasecmp(h_value, "application/timestamp-query") == 0)
            is_tsq = 1;
    }

    unsigned char *content = NULL;
    size_t content_length = 0;

    char *serial_id = NULL;

    // Send HTTP reply to the client.
    //
    // If it's a time-stamp query.
    if (is_tsq) {
        // Recover query content from http request.
        char *query = calloc(request_info->content_length, sizeof(char));
        int query_len = mg_read(conn, query, request_info->content_length);

        // Log the query as DEBUG log in hexadecimal format
        log_hex(ct, LOG_DEBUG, "query hexdump content", (unsigned char *)query,
                request_info->content_length);
        // Get an OpenSSL TS_RESP_CTX (wrapped inside ts_resp_ctx_wrapper
        // structure).
        // get_ctxw recovers the first unused TS_RESP_CTX
        // in the ct->ts_ctx_pool pool of TS_RESP_CTX.
        // (TS_RESP_CTX are not thread safe)
        ts_resp_ctx_wrapper *ctx_w = get_ctxw(ct);
        if (ctx_w == NULL) {
            resp_code = 500;
            uts_logger(context, LOG_WARNING,
                       "Unable to get an OpenSSL ts_context in the pool");

        } else {
            // create the response
            resp_code = create_response(ct, query, query_len, ctx_w->ts_ctx,
                                        &content_length, &content, &serial_id);
            // free the TS_RESP_CTX used
            ctx_w->available = 1;
        }
        // respond according to create_response return code
        switch (resp_code) {
        case 200:
            mg_printf(conn,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: application/timestamp-reply\r\n"
                      "Content-Length: %d\r\n"
                      "\r\n",
                      (int)content_length);
            mg_write(conn, content, content_length);
            log_hex(ct, LOG_DEBUG, "response hexdump content", content,
                    content_length);
            break;
        case 400:
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\n"
                            "Content-Type: text/plain\r\n"
                            "Content-Length: 12\r\n"
                            "\r\n"
                            "client error");
            break;
        default:
            mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\n"
                            "Content-Type: text/plain\r\n"
                            "Content-Length: 17\r\n"
                            "\r\n"
                            "uts-server error");
        }
        free(query);
        free(content);
    } else {
        // default reply if we don't have a time-stamp request
        content_length = strlen(content_static_page);
        mg_printf(conn,
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/html\r\n"
                  "Content-Length: %d\r\n"
                  "\r\n",
                  (int)content_length);
        mg_write(conn, content_static_page, content_length);
    }
    // initialize a serial_id if not created by create_response
    if (serial_id == NULL) {
        serial_id = calloc(9, sizeof(char));
        serial_id = rand_string(serial_id, 8);
    }

    // some debugging logs
    log_request_debug(request_info, serial_id, ct);
    // end of some timer stuff
    diff = clock() - start;
    // log the request
    log_request(request_info, serial_id, ct, resp_code,
                (diff * 1000000 / CLOCKS_PER_SEC));
    free(serial_id);
    return 1;
}

int ca_serve_handler(struct mg_connection *conn, void *context) {
    /* In this handler, we ignore the req_info and send the file "filename". */
    const struct mg_request_info *request_info = mg_get_request_info(conn);
    clock_t start = clock(), diff;
    rfc3161_context *ct = (rfc3161_context *)context;
    const char *filename = ct->ca_file;
    if (strlen(filename) == 0) {
        uts_logger(context, LOG_NOTICE,
                   "'certs' param in '[ tsa ]' section not filed");
        mg_send_http_error(conn, 404, "CA file not available");
        diff = clock() - start;
        log_request(request_info, "CA_DL  ", ct, 404,
                    (diff * 1000000 / CLOCKS_PER_SEC));
        return 1;
    }
    if (access(filename, F_OK) != -1) {
        mg_send_file(conn, filename);
        const struct mg_response_info *ri = mg_get_response_info(conn);
        diff = clock() - start;
        log_request(request_info, "CA_DL  ", ct, 200,
                    (diff * 1000000 / CLOCKS_PER_SEC));

    } else {
        uts_logger(context, LOG_NOTICE, "CA file '%s' not available", filename);
        mg_send_http_error(conn, 404, "CA file not available");
        diff = clock() - start;
        log_request(request_info, "CA_DL  ", ct, 404,
                    (diff * 1000000 / CLOCKS_PER_SEC));
    }
    return 1;
}

int cert_serve_handler(struct mg_connection *conn, void *context) {
    /* In this handler, we ignore the req_info and send the file "filename". */
    const struct mg_request_info *request_info = mg_get_request_info(conn);
    clock_t start = clock(), diff;
    rfc3161_context *ct = (rfc3161_context *)context;
    const char *filename = ct->cert_file;
    if (strlen(filename) == 0) {
        uts_logger(context, LOG_NOTICE,
                   "'signer_cert' param in '[ tsa ]' section not filed");
        mg_send_http_error(conn, 404, "CA file not available");
        diff = clock() - start;
        log_request(request_info, "CERT_DL", ct, 404,
                    (diff * 1000000 / CLOCKS_PER_SEC));
        return 1;
    }
    if (access(filename, F_OK) != -1) {
        mg_send_file(conn, filename);
        const struct mg_response_info *ri = mg_get_response_info(conn);
        diff = clock() - start;
        log_request(request_info, "CERT_DL", ct, 200,
                    (diff * 1000000 / CLOCKS_PER_SEC));

    } else {
        uts_logger(context, LOG_NOTICE,
                   "signer certificate file '%s' not available", filename);
        mg_send_http_error(conn, 404, "CA file not available");
        diff = clock() - start;
        log_request(request_info, "CERT_DL", ct, 404,
                    (diff * 1000000 / CLOCKS_PER_SEC));
    }
    return 1;
}

int notfound_handler(struct mg_connection *conn, void *context) {
    /* In this handler, we ignore the req_info and send the file "filename". */
    const struct mg_request_info *request_info = mg_get_request_info(conn);
    rfc3161_context *ct = (rfc3161_context *)context;
    mg_send_http_error(conn, 404, "");
    return 1;
}

int http_server_start(char *conffile, char *conf_wd, bool stdout_dbg) {
    struct mg_context *ctx;
    struct mg_callbacks callbacks;
    struct tuser_data user_data;

    rfc3161_context *ct = (rfc3161_context *)calloc(1, sizeof(rfc3161_context));
    ct->stdout_dbg = stdout_dbg;
    ct->loglevel = 8;
    init_ssl();
    if (!set_params(ct, conffile, conf_wd))
        return EXIT_FAILURE;

    // Disable stdout buffering if logging to stdout
    if (ct->stdout_logging || ct->stdout_dbg)
        setbuf(stdout, NULL);

    // Prepare callbacks structure. We have only one callback, the rest are
    // NULL.
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&user_data, 0, sizeof(user_data));
    callbacks.log_message = &log_civetweb;

    // Start the web server.
    ctx = mg_start(&callbacks, &user_data, ct->http_options);
    if (ctx != NULL) {
        mg_set_request_handler(ctx, "/", rfc3161_handler, (void *)ct);
        mg_set_request_handler(ctx, "/favicon.ico", notfound_handler, (void *)ct);
        mg_set_request_handler(ctx, "/ca.pem", ca_serve_handler, (void *)ct);
        mg_set_request_handler(ctx, "/tsa_cert.pem", cert_serve_handler,
                               (void *)ct);

        // Wait until some signals are received
        while (g_uts_sig == 0) {
            sleep(1);
        }
    } else {
        uts_logger(ct, LOG_ERR, "Failed to start uts-server: %s",
                   ((user_data.first_message == NULL)
                        ? "unknown reason"
                        : user_data.first_message));
    }

    // Stop the server.
    mg_stop(ctx);
    free_uts_context(ct);
    free_ssl();

    return 0;
}
