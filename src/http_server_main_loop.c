#include "common.h"

static struct redis_config *redis_conf = NULL;

void set_redis_config(struct redis_config *conf) {
    redis_conf = conf;
}

thread_local int postType_0signup_1login_2_admin = -1 ;
thread_local const char *postType_str ;
const char* postReqAhead[] = { 
    "signup",
    "login",
    "admin",
    NULL
};
const int DbIdx_salt[] = { 
    -1,
    DatabaseIdx_salt_Login,
    DatabaseIdx_salt_Admin
};
const int DbIdx_ipCount[] = { 
    DatabaseIdx_ipCount_SignUp,
    DatabaseIdx_ipCount_Login,
    DatabaseIdx_ipCount_Admin
};
int DbIdx_ipCountX = -1 ;
// DbIdx_ipCount[postType_0signup_1login_2_admin]

void http_serve(void) {
    int server_fd = get_server_fd();
    if (server_fd == -1) {
        DXhttp_print_debug("Invalid server socket");
        return;
    }

    struct timeval timeout = { 1, 0 };
    if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        DXhttp_print_debug("Setsockopt timeout failed: %s", strerror(errno));
        return;
    }

    while (http_should_keep_running()) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            if (errno == EINTR) continue;
            DXhttp_print_debug("Accept failed: %s", strerror(errno));
            sleep(1);
            continue;
        }

        DXhttp_print_debug("Accepted client connection: ================================ [%s]: accept new connect.\n", execBinaryMd5);

        char rece_buffer[REQUEST_MAX_SIZE + 1];
        ssize_t n = read(client_fd, rece_buffer, REQUEST_MAX_SIZE);
        if (n <= 0) {
            DXhttp_print_debug("Read failed or connection closed");
            close(client_fd);
            continue;
        }
        rece_buffer[n] = '\0';
        DXhttp_print_debug("rece_buffer is :[%s]", rece_buffer);

        //if (strncmp(rece_buffer, "POST /signup", 12) != 0) {
        int rt = check_post_type_path( rece_buffer, n ) ;
        if ( rt < 0 ) {
            DXhttp_print_debug("Invalid request, expected POST /path, got: %d:[%.25s]", rt, rece_buffer);
            send_response(client_fd, 422, "Unprocessable Entity", NULL, "20:%d:[%.25s]", rt, rece_buffer);
            close(client_fd);
            continue;
        }
        DXhttp_print_debug("get postType_0signup_1login_2_admin is :%d [%s]", postType_0signup_1login_2_admin, postType_str );

        struct session_request req = { 0, NULL, NULL, NULL };
        char *ip = NULL ;

        extern int http_server_main_loop_handler( char **ip, struct redis_config *redis_conf, int *client_fd, char rece_buffer[], ssize_t n, struct session_request *req );
        rt = http_server_main_loop_handler( &ip ,redis_conf, &client_fd , rece_buffer , n, &req);
        if ( rt ) {
            if ( rt == 1 ) {
                DXhttp_print_debug("App [%s] successful ask for New Session.\n\n\n", postType_str );
            } else {
                DXhttp_print_debug("App [%s] faile during being handle, rt : %d\n\n\n", postType_str , rt );
            }
        } else {
            DXhttp_print_debug("App [%s] successful\n\n\n", postType_str );
        }
        http_main_loop_clean_before_exit( &client_fd, &ip, &req );
    } // end of while loop
}
