#include "common.h"

static char buf1[101] ; 
static char *strArr1[] = { buf1, NULL } ;

void send_response_sess2(int client_fd, char *sess, char *sesv ) {
    snprintf(buf1, 100, "%s_sess=%s; HttpOnly; Max-Age=300", postType_str, sess );
    send_response(client_fd, 200, "OK", strArr1, "{\"ver\": 1," 
            " \"%s_sess\": \"%s\""
            "}", 
            postType_str, sesv
            ) ;
}

void send_response_sess3(int client_fd, char *sess, char *sesv, char *salt ) {
    snprintf(buf1, 100, "%s_sess=%s; HttpOnly; Max-Age=300", postType_str, sess );
    send_response(client_fd, 200, "OK", strArr1, "{\"ver\": 1,"
            " \"%s_sess\": \"%s\""
            ", \"%s_salt\": \"%s\""
            "}", 
            postType_str, sesv, 
            postType_str, salt
            ) ;
}
