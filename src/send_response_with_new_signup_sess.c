#include "common.h"

static char body[512];
void send_response_with_new_signup_sess(int client_fd, int status, const char *signup_sess, const char *signup_sesv) {
    char response[512];
    const char *status_text = (status == 200) ? "OK" : "Unprocessable Entity";

    snprintf(body, 511, "{\"ver\": 1, \"signup_sess\": \"%s\"}", signup_sesv) ;
    int content_length = strlen(body);

    //send_response(client_fd, 200, "OK", "{\"ver\": 1, \"signup_sess\": \"%s\"}", signup_sesv);
    //
    /*Path=/: This specifies that the cookie is available across the entire domain (all paths). The cookie will be sent with requests to any URL path on your domain.*/
    /*HttpOnly: This flag prevents JavaScript from accessing the cookie through document.cookie. As you've identified, if you want JavaScript to access this cookie, you should remove this flag.*/
    /*"Set-Cookie: signup_session=%s; Path=/; HttpOnly; Max-Age=300\r\n"*/
    /*"Set-Cookie: signup_session=%s; Max-Age=300\r\n" */
    snprintf(response, sizeof(response),
             "HTTP/1.1 %d %s\r\n"
             "Content-Length: %d\r\n"
             "Set-Cookie: signup_session=%s; HttpOnly; Max-Age=300\r\n"
             "Cache-Control: no-cache, no-store\r\n"
             "\r\n"
             "%s",
             status, status_text,
             content_length,
             signup_sess,
             body);

    write(client_fd, response, strlen(response));
}
