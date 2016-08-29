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
#include "http.h"

void log_request_debug(const struct mg_request_info *request_info,
                       int request_id, void *context) {
    for (int i = 0; i < request_info->num_headers; i++) {
        uts_logger(context, LOG_DEBUG, "Request[%d], Header[%s]: %s",
                   request_id, request_info->http_headers[i].name,
                   request_info->http_headers[i].value);
    }
    uts_logger(context, LOG_DEBUG, "Request[%d], request_method: %s",
               request_id, request_info->request_method);
    uts_logger(context, LOG_DEBUG, "Request[%d], request_uri: %s", request_id,
               request_info->request_uri);
    uts_logger(context, LOG_DEBUG, "Request[%d], local_uri: %s", request_id,
               request_info->local_uri);
    uts_logger(context, LOG_DEBUG, "Request[%d], http_version: %s", request_id,
               request_info->http_version);
    uts_logger(context, LOG_DEBUG, "Request[%d], query_string: %s", request_id,
               request_info->query_string);
    uts_logger(context, LOG_DEBUG, "Request[%d], remote_addr: %s", request_id,
               request_info->remote_addr);
    uts_logger(context, LOG_DEBUG, "Request[%d], is_ssl: %d", request_id,
               request_info->is_ssl);
    uts_logger(context, LOG_DEBUG, "Request[%d], content_length: %d",
               request_id, request_info->content_length);
    uts_logger(context, LOG_DEBUG, "Request[%d], remote_port: %d", request_id,
               request_info->remote_port);
    //    uts_logger(context, LOG_DEBUG, "Request[%d], user_data: %s",
    //    request_id,
    //               request_info->user_data);
    //    uts_logger(context, LOG_DEBUG, "Request[%d], conn_data: %s",
    //    request_id,
    //               request_info->conn_data);
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
    /* Handler may access the request info using mg_get_request_info */
    const struct mg_request_info *request_info = mg_get_request_info(conn);
    rfc3161_context *ct = (rfc3161_context *)context;
    int ret;
    ct->query_counter++;
    uint64_t query_id = ct->query_counter;
    log_request_debug(request_info, query_id, ct);

    bool is_tsq = 0;

    for (int i = 0; i < request_info->num_headers; i++) {
        const char *h_name = request_info->http_headers[i].name;
        const char *h_value = request_info->http_headers[i].value;
        if (strcmp(h_name, "Content-Type") == 0 &&
            strcmp(h_value, "application/timestamp-query") == 0)
            is_tsq = 1;
    }

    unsigned char *content;
    int content_length = 0;

    // Send HTTP reply to the client
    if (is_tsq) {
        char *query = calloc(request_info->content_length, sizeof(char));
        mg_read(conn, query, request_info->content_length);

        log_hex(ct, LOG_DEBUG, "query hexdump content", query,
                request_info->content_length);

        int ts_resp =
            create_response(ct, query, ct->ts_ctx, &content_length, &content);
        if (ts_resp) {
            log_hex(ct, LOG_DEBUG, "response hexdump content", content,
                content_length);
            mg_printf(conn,
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: application/timestamp-reply\r\n"
                      "Content-Length: %d\r\n" // Always set Content-Length
                      "\r\n",
                      content_length);
            mg_write(conn, content, content_length);
            // free(content);
        } else {
            mg_printf(conn,
                      "HTTP/1.1 500 OK\r\n"
                      "Content-Type: text/plain\r\n"
                      "Content-Length: 17\r\n" // Always set Content-Length
                      "\r\n"
                      "uts-server error");
        }
    } else {
        mg_printf(conn,
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/plain\r\n"
                  "Content-Length: 46\r\n" // Always set Content-Length
                  "\r\n"
                  "uts-server, a simple RFC 3161 timestamp server");
    }

    return 1;
}

int http_server_start(char *conffile, bool stdout_dbg) {
    struct mg_context *ctx;
    struct mg_callbacks callbacks;

    rfc3161_context *ct = (rfc3161_context *)calloc(1, sizeof(rfc3161_context));
    ct->stdout_dbg = stdout_dbg;
    ct->loglevel = 8;
    if (!set_params(ct, conffile))
        return 1;

    // Prepare callbacks structure. We have only one callback, the rest are
    // NULL.
    memset(&callbacks, 0, sizeof(callbacks));

    // Start the web server.
    ctx = mg_start(&callbacks, NULL, ct->http_options);
    mg_set_request_handler(ctx, "/", rfc3161_handler, (void *)ct);

    // Wait until user hits "enter". Server is running in separate thread.
    // Navigating to http://localhost:8080 will invoke begin_request_handler().
    getchar();

    // Stop the server.
    mg_stop(ctx);
    free(ct);

    return 0;
}
