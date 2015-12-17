#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/ts.h>
#include <openssl/bn.h>
#include <asyncd/asyncd.h>

int ts_http_respond(short event, ad_conn_t *conn, void *userdata) {
    if (event & AD_EVENT_READ) {
        if (ad_http_get_status(conn) == AD_HTTP_REQ_DONE) {
            ad_http_response(conn, 200, "text/html", "Hello World", 11);
            return ad_http_is_keepalive_request(conn) ? AD_DONE : AD_CLOSE;
        }
    }
    return AD_OK;
}

int http_default_handler(short event, ad_conn_t *conn, void *userdata) {
    if (event & AD_EVENT_READ) {
        if (ad_http_get_status(conn) == AD_HTTP_REQ_DONE) {
            ad_http_response(conn, 501, "text/html", "Not implemented", 15);
            return AD_CLOSE; // Close connection.
        }
    }
    return AD_OK;
}

int http_server_start() {
    //SSL_load_error_strings();
    //SSL_library_init();
    ad_log_level(AD_LOG_DEBUG);
    ad_server_t *server = ad_server_new();
    ad_server_set_option(server, "server.port", "8888");
    //ad_server_set_ssl_ctx(server,
    //        ad_server_ssl_ctx_create_simple("ssl.cert", "ssl.pkey"));
    ad_server_register_hook(server, ad_http_handler, NULL); // HTTP Parser is also a hook.
    ad_server_register_hook_on_method(server, "GET", ts_http_respond, NULL);
    ad_server_register_hook(server, http_default_handler, NULL);
    return ad_server_start(server);
}
