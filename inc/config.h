typedef struct uts_config {
    char *port;
    char *listen;
    bool https;
    cert https_cert;
    void *ts_certs;

} uts_config;

typedef struct cert {
    char *cert_file;
    char *key_file;
} cert;
