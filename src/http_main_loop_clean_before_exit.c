#include "common.h"

void http_main_loop_clean_before_exit( int *client_fd, char **ip, struct session_request * req ) {

    if ( req ) {
        if (req->username)      free(req->username);
        if (req->passwd)        free(req->passwd);
        if (req->signup_salt)   free(req->signup_salt);
        req->username    = NULL ;
        req->passwd      = NULL ;
        req->signup_salt = NULL ;
    }
    if ( ip && (*ip) ){
        free(*ip);
    }
    if ( client_fd && ((*client_fd) >= 0 )) {
        close(*client_fd);
    }

}
