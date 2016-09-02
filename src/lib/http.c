/*
 *    "This product includes software developed by the OpenSSL Project
 *     *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <civetweb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <time.h>
#include "http.h"

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

void log_request_debug(const struct mg_request_info *request_info,
                       char *request_id, void *context) {
    for (int i = 0; i < request_info->num_headers; i++) {
        uts_logger(context, LOG_DEBUG, "Request[%s], Header[%s]: %s",
                   request_id, request_info->http_headers[i].name,
                   request_info->http_headers[i].value);
    }
    uts_logger(context, LOG_DEBUG, "Request[%s], request_method: %s",
               request_id, request_info->request_method);
    uts_logger(context, LOG_DEBUG, "Request[%s], request_uri: %s", request_id,
               request_info->request_uri);
    uts_logger(context, LOG_DEBUG, "Request[%s], local_uri: %s", request_id,
               request_info->local_uri);
    uts_logger(context, LOG_DEBUG, "Request[%s], http_version: %s", request_id,
               request_info->http_version);
    uts_logger(context, LOG_DEBUG, "Request[%s], query_string: %s", request_id,
               request_info->query_string);
    uts_logger(context, LOG_DEBUG, "Request[%s], remote_addr: %s", request_id,
               request_info->remote_addr);
    uts_logger(context, LOG_DEBUG, "Request[%s], is_ssl: %d", request_id,
               request_info->is_ssl);
    uts_logger(context, LOG_DEBUG, "Request[%s], content_length: %d",
               request_id, request_info->content_length);
    uts_logger(context, LOG_DEBUG, "Request[%s], remote_port: %d", request_id,
               request_info->remote_port);
    //    uts_logger(context, LOG_DEBUG, "Request[%d], user_data: %s",
    //    request_id,
    //               request_info->user_data);
    //    uts_logger(context, LOG_DEBUG, "Request[%d], conn_data: %s",
    //    request_id,
    //               request_info->conn_data);
}

void log_request(const struct mg_request_info *request_info, char *request_id,
                 void *context, int response_code, int timer) {
    const char *user_agent = NULL;
    const char *content_type = NULL;

    for (int i = 0; i < request_info->num_headers; i++) {
        if (strcmp(request_info->http_headers[i].name, "User-Agent") == 0) {
            user_agent = request_info->http_headers[i].value;
        }
        if (strcmp(request_info->http_headers[i].name, "Content-Type") == 0) {
            content_type = request_info->http_headers[i].value;
        }
    }

    uts_logger(context, LOG_INFO, "Request[%s], remote_addr[%s] ssl[%d] "
                                  "uri[%s] http_resp_code[%d] duration[%d us] "
                                  "user-agent[%s] content-type[%s]",
               request_id, request_info->remote_addr, request_info->is_ssl,
               request_info->local_uri, response_code, timer, user_agent,
               content_type);
}

// This function will be called by civetweb on every new request.
static int begin_request_handler(struct mg_connection *conn) {
    const struct mg_request_info *request_info = mg_get_request_info(conn);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/plain\r\n"
              "Content-Length: 46\r\n" // Always set Content-Length
              "\r\n"
              "uts-server, a simple RFC 3161 timestamp server");

    // Returning non-zero tells civetweb that our function has replied to
    // the client, and civetweb should not send client any more data.
    return 1;
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

    for (int i = 0; i < request_info->num_headers; i++) {
        const char *h_name = request_info->http_headers[i].name;
        const char *h_value = request_info->http_headers[i].value;
        if (strcmp(h_name, "Content-Type") == 0 &&
            strcmp(h_value, "application/timestamp-query") == 0)
            is_tsq = 1;
    }

    unsigned char *content = NULL;
    size_t content_length = 0;

    char *serial_id = NULL;

    // Send HTTP reply to the client
    if (is_tsq) {
        char *query = calloc(request_info->content_length, sizeof(char));
        int query_len = mg_read(conn, query, request_info->content_length);

        log_hex(ct, LOG_DEBUG, "query hexdump content", (unsigned char *)query,
                request_info->content_length);

        int ts_resp = create_response(ct, query, query_len, ct->ts_ctx,
                                      &content_length, &content, &serial_id);
        if (ts_resp) {
            resp_code = 200;
            mg_printf(conn,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: application/timestamp-reply\r\n"
                      "Content-Length: %d\r\n" // Always set Content-Length
                      "\r\n",
                      (int)content_length);
            mg_write(conn, content, content_length);
            log_hex(ct, LOG_DEBUG, "response hexdump content", content,
                    content_length);
        } else {
            resp_code = 500;
            mg_printf(conn,
                      "HTTP/1.1 500 Internal Server Error\r\n"
                      "Content-Type: text/plain\r\n"
                      "Content-Length: 17\r\n" // Always set Content-Length
                      "\r\n"
                      "uts-server error");
        }
        free(content);
    } else {
        resp_code = 200;
        mg_printf(conn,
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/plain\r\n"
                  "Content-Length: 46\r\n" // Always set Content-Length
                  "\r\n"
                  "uts-server, a simple RFC 3161 timestamp server");
    }
    if (serial_id == NULL) {
        serial_id = calloc(9, sizeof(char));
        serial_id = rand_string(serial_id, 8);
    }

    log_request_debug(request_info, serial_id, ct);
    diff = clock() - start;
    log_request(request_info, serial_id, ct, resp_code,
                (diff * 1000000 / CLOCKS_PER_SEC));
    free(serial_id);
    return 1;
}

int http_server_start(char *conffile, char *conf_wd, bool stdout_dbg) {
    struct mg_context *ctx;
    struct mg_callbacks callbacks;

    rfc3161_context *ct = (rfc3161_context *)calloc(1, sizeof(rfc3161_context));
    ct->stdout_dbg = stdout_dbg;
    ct->loglevel = 8;
    init_ssl();
    if (!set_params(ct, conffile, conf_wd))
        return EXIT_FAILURE;

    // Prepare callbacks structure. We have only one callback, the rest are
    // NULL.
    memset(&callbacks, 0, sizeof(callbacks));

    // Start the web server.
    ctx = mg_start(&callbacks, NULL, ct->http_options);
    mg_set_request_handler(ctx, "/", rfc3161_handler, (void *)ct);

    // Wait until some signals are received
    while (g_uts_sig == 0) {
        sleep(1);
    }

    // Stop the server.
    mg_stop(ctx);
    free(ct);
    free_ssl();

    return 0;
}
