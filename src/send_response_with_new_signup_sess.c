#include "common.h"

void send_response_with_new_signup_sess(int client_fd, int status, const char *signup_sess) {
    char response[512];
    int content_length = strlen(signup_sess);
    const char *status_text = (status == 200) ? "OK" : "Unprocessable Entity";

    snprintf(response, sizeof(response),
             "HTTP/1.1 %d %s\r\n"
             "Content-Length: %d\r\n"
             /*"Set-Cookie: signup_session=%s; Path=/; HttpOnly; Max-Age=300\r\n"*/
             /*Path=/: This specifies that the cookie is available across the entire domain (all paths). The cookie will be sent with requests to any URL path on your domain.*/
             /*HttpOnly: This flag prevents JavaScript from accessing the cookie through document.cookie. As you've identified, if you want JavaScript to access this cookie, you should remove this flag.*/
             "Set-Cookie: signup_session=%s; Max-Age=300\r\n"
             "Cache-Control: no-cache, no-store\r\n"
             "\r\n"
             "%s",
             status, status_text,
             content_length,
             signup_sess,
             signup_sess);

    write(client_fd, response, strlen(response));
}
