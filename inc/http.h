#include "utils.h"
#include <stdbool.h>

struct tuser_data {
    char *first_message;
};

int http_server_start(char *conffile, char *conf_wd, bool stdout_dbg);

#define STATIC_PAGE                                                               \
    "HTTP/1.1 200 OK\r\n"                                                         \
    "Content-Type: text/html\r\n"                                                 \
    "\r\n"                                                                        \
    "<html>"                                                                      \
    "<head>"                                                                      \
    "  <meta charset=\"utf-8\">"                                                  \
    "  <title>uts-server</title>"                                                 \
    "  <meta name=\"author\" content=\"Pierre-Francois Carpentier\">"             \
    "  <meta name=\"description\" content=\"uts-server\">"                        \
    "<style>"                                                                     \
    ".rcorners {"                                                                 \
    "  border-radius: 10px;"                                                      \
    "  border: 2px solid #0080ff;"                                                \
    "  margin: 20px;"                                                             \
    "  padding: 10px;"                                                            \
    "  box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, "      \
    "0, 0.19);"                                                                   \
    "  margin-left: auto;"                                                        \
    "  margin-right: auto;"                                                       \
    "  width: 40%;"                                                               \
    "}"                                                                           \
    "body {"                                                                      \
    "  margin: 0px;"                                                              \
    "}"                                                                           \
    ".code {"                                                                     \
    "  border-radius: 3px;"                                                       \
    "  border: 2px solid #000000;"                                                \
    "  margin: 20px;"                                                             \
    "  padding: 10px;"                                                            \
    "  width: 90%;"                                                               \
    "  background: #404040;"                                                      \
    "  color: #e6e6e6;"                                                           \
    "  margin-left: auto;"                                                        \
    "  margin-right: auto;"                                                       \
    "}"                                                                           \
    ".button {"                                                                   \
    "  background-color: #3366ff;"                                                \
    "  border: none;"                                                             \
    "  color: white;"                                                             \
    "  text-align: center;"                                                       \
    "  text-decoration: none;"                                                    \
    "  display: inline-block;"                                                    \
    "  font-size: 14px;"                                                          \
    "  margin: 4px 2px;"                                                          \
    "  cursor: pointer;"                                                          \
    "  border-radius: 2px;"                                                       \
    "  padding: 10px 24px;"                                                       \
    "  margin: 0 auto;"                                                           \
    "  display: inline;"                                                          \
    "  box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, "      \
    "0, 0.19);"                                                                   \
    "  margin: 0 auto;"                                                           \
    "}"                                                                           \
    ".desc {"                                                                     \
    "  text-decoration: underline;"                                               \
    "  text-align: center;"                                                       \
    "  font-size: 20px;"                                                          \
    "  margin-top: 20px"                                                          \
    "}"                                                                           \
    ".center {"                                                                   \
    "  text-align: center;"                                                       \
    "}"                                                                           \
    ".footer {"                                                                   \
    "  position: fixed;"                                                          \
    "  bottom: 0px;"                                                              \
    "  padding-top: 5px;"                                                         \
    "  border-top: 1px solid gray;"                                               \
    "  width: 100%;"                                                              \
    "  background-color: #f5f5f5;"                                                \
    "  font-size: 14px;"                                                          \
    "  text-align: center;"                                                       \
    "}"                                                                           \
    ".var {"                                                                      \
    "  color: #99ccff;"                                                           \
    "}"                                                                           \
    "</style>"                                                                    \
    "</head>"                                                                     \
    "<body>"                                                                      \
    "<div class=\"desc\">"                                                        \
    "  uts-server, a simple RFC 3161 timestamp server"                            \
    "</div>"                                                                      \
    "<div class=\"rcorners\">"                                                    \
    "  For timestamping a file with OpenSSL and curl, run the following "         \
    "commands"                                                                    \
    "  (setting the $UTS_SERVER_URL, $FILE and $FILE_TIMESTAMP variables):"       \
    "  <div class=\"code\">"                                                      \
    "   openssl ts -query -data \"<span class=\"var\">$FILE</span>\" -out "       \
    "\"ts_req.ts\";<br/>"                                                         \
    "   curl \"<span class=\"var\">$UTS_SERVER_URL</span>\" \\<br/>"              \
    "   &nbsp;&nbsp;&nbsp;&nbsp; -H \"Content-Type: "                             \
    "application/timestamp-query\" \\<br/>"                                       \
    "   &nbsp;&nbsp;&nbsp;&nbsp; -f -g --data-binary \"@ts_req.ts\" -o "          \
    "\"<span class=\"var\">$FILE_TIMESTAMP</span>\""                              \
    "  </div>"                                                                    \
    "  For verifying the timestamp with OpenSSL, download the CA and the "        \
    "signer cert, and run the following command:"                                 \
    "  <div class=\"code\">"                                                      \
    "    openssl ts -verify -in \"<span "                                         \
    "class=\"var\">$FILE_TIMESTAMP</span>\" \\<br/>"                              \
    "    &nbsp;&nbsp;&nbsp;&nbsp; -data \"<span class=\"var\">$FILE</span>\" "    \
    "-CAfile ca.pem -untrusted tsa_cert.pem"                                      \
    "  </div>"                                                                    \
    "  <div class=\"center\">"                                                    \
    "    <a href=\"./ca.pem\" download><button class=\"button\">Dowload CA "      \
    "file</button></a>"                                                           \
    "    <a href=\"./tsa_cert.pem\" download><button "                            \
    "class=\"button\">Dowload tsa cert file</button></a>"                         \
    "  </div>"                                                                    \
    "</div>"                                                                      \
    "<div class=\"footer\">"                                                      \
    "  <div class=\"container\">"                                                 \
    "    <a href=\"http://uts-server.readthedocs.org\" "                          \
    "target=\"_blank\">uts-server</a>"                                            \
    "    • © 2019 • Pierre-François Carpentier • Released under the MIT " \
    "License"                                                                     \
    "  </div>"                                                                    \
    "</div>"                                                                      \
    "</body>"                                                                     \
    "</html>"
