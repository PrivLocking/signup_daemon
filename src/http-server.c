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

        char *ip = http_get_client_ip(client_fd, rece_buffer);
        rt = redis_check_ip(ip, redis_conf, DbIdx_ipCountX) ;
        if (rt) {
            DXhttp_print_debug("IP check failed for %s, rt %d", ip, rt );
            send_response(client_fd, 422, "Unprocessable Entity", NULL, "22:%d", rt);
            free(ip);
            close(client_fd);
            continue;
        }

        char *body = strstr(rece_buffer, "\r\n\r\n");
        if (!body) {
            DXhttp_print_debug("No request body found");
            send_response(client_fd, 422, "Unprocessable Entity", NULL, "24");
            free(ip);
            close(client_fd);
            continue;
        }
        body += 4;

        int len = strlen(body);
        DXhttp_print_debug("body len (%d):[%.40s]", len, body);
        // to combine signup/login, the empty body change from '{}' to '{"ver":1}'
        /*
        if (2 == len && '{' == body[0] && '}' == body[1]) { // {}
        }
        */


        struct session_request req = { 0, NULL, NULL, NULL };
        rt = parse_session_json(body, &req) ;
        if (rt) {
            send_response(client_fd, 422, "Unprocessable Entity", NULL, "10:%d", rt);
            if (req.username) free(req.username);
            if (req.passwd) free(req.passwd);
            if (req.signup_salt) free(req.signup_salt);
            // please notice : here , we must check the req.xxx, it maybe NULL. after that, it must be not NULL, and no more check is needed.
            free(ip);
            close(client_fd);
            continue;
        }
        DXhttp_print_debug("json looks OK: %d,[%s],[%s],[%s]", req.ver, req.username, req.passwd, req.signup_salt);

        if ( !req.passwd && !req.signup_salt) { 
            DXhttp_print_debug("Empty req, means new req." );
            rt = sess_handle_new_request(redis_conf, &req, client_fd );
            if ( rt ) {
                DXhttp_print_debug("sess_handle_new_request met error:%d", rt );
            }

            free(req.username);
            free(ip);
            close(client_fd);
            continue;
        }
        DXhttp_print_debug("nor-Empty req, need more analyze." );

        char dbSavedVerifyTmpSalt[33] ;
        rt = redis_get_hget_string(redis_conf, DbIdx_ipCountX, 32, dbSavedVerifyTmpSalt, "GET %s_sess:%s:%s", postType_str, req.username, req.signup_salt ) ;
        if ( rt ) {
            DXhttp_print_debug("no such a %s salt found! :%d" , postType_str, rt);
            send_response(client_fd, 422, "Unprocessable Entity", NULL, "16:%d", rt );
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }

        char verifyTmpValue[33] ;
        rt = cookie_extract(rece_buffer, n, verifyTmpValue, sizeof(verifyTmpValue), "%s_sess", postType_str );
        if ( rt ) {
            DXhttp_print_debug("no such a %s cookie found!  :%d", postType_str, rt );
            send_response(client_fd, 422, "Unprocessable Entity", NULL, "18:%d", rt );
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }
        DXhttp_print_debug(" dbSavedVerifyTmpSalt [%s], verifyTmpValue [%s]", dbSavedVerifyTmpSalt, verifyTmpValue);

        rt = strncmp( dbSavedVerifyTmpSalt, verifyTmpValue, 33 );
        if ( 0 != rt ) {
            DXhttp_print_debug(" dbSavedVerifyTmpSalt[%s] and verifyTmpValue[%s] not equal", dbSavedVerifyTmpSalt, verifyTmpValue );
            send_response(client_fd, 422, "Unprocessable Entity", NULL, "20");
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }

        //rt = redis_check_username(req.username, redis_conf) ;
        long tmpLong ;
        tmpLong = -1 ;
        rt = redis_get_int(redis_conf, DatabaseIdx_salt_Login, &tmpLong, "EXISTS %sUser:%s", postType_str, req.username ) ;
        if ( rt || (tmpLong != 1 )) {
            DXhttp_print_debug("Username check failed, or already exist for %s, rt-> %d, tmpLong -> %ld", req.username, rt, tmpLong );
            send_response(client_fd, 422, "Unprocessable Entity", NULL, "24:%d:%ld", rt, tmpLong); // failed , or username exist,
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }
        DXhttp_print_debug("Username check : exist. Now, do more check on %s to see if salt/hash is correct.", postType_str );

        /* in this case, dbSavedVerifyTmpSalt == verifyTmpValue, no more check/calc, just save it.
        char hash[HASH_LEN + 1], salt[SALT_LEN + 1];
        if (! compute_signup_hash2(req.username, req.passwd, hash, salt)) { */

        if ( 0 == postType_0signup_1login_2_admin ) {
            rt = redis_set_hset_key_value(redis_conf, DbIdx_ipCountX, SIGNUP_OK_TTL, "SET signupOK:%s 1", ip);
            if ( rt ) {
                DXhttp_print_debug("[SET %sOK:%s 1 EX %d] failed, rt-> %d ", postType_str, ip, SIGNUP_OK_TTL, rt);
                send_response(client_fd, 422, "Unprocessable Entity", NULL, "34:%d", rt); 
                free(req.username);
                free(req.passwd);
                free(req.signup_salt);
                free(ip);
                close(client_fd);
                continue;
            }

            //rt = redis_hset_key_value_pair( redis_conf, DatabaseIdx_salt_Login, &tmpLong, NEW_USER_TTL_30d,
            //        "HSET loginUser:%s hash %s salt %s level 0 status active", req.username, req.passwd, req.signup_salt);
            rt = redis_set_hset_key_value( redis_conf, DatabaseIdx_salt_Login, NEW_USER_TTL_30d,
                    "HSET loginUser:%s hash %s salt %s level 0 status active", req.username, req.passwd, req.signup_salt);
            if ( rt ) {
                DXhttp_print_debug("HSET for %s failed, rt-> %d", postType_str, rt );
                send_response(client_fd, 422, "Unprocessable Entity", NULL, "44:%d", rt); 
                free(req.username);
                free(req.passwd);
                free(req.signup_salt);
                free(ip);
                close(client_fd);
                continue;
            }
            send_response(client_fd, 200, "OK", NULL, NULL);

        } else {
            rt = 8388188 ; // under constructing
            if ( rt ) {
                DXhttp_print_debug("HSET for %s failed, rt-> %d", postType_str, rt );
                send_response(client_fd, 422, "Unprocessable Entity", NULL, "54:%d", rt); 
                free(req.username);
                free(req.passwd);
                free(req.signup_salt);
                free(ip);
                close(client_fd);
                continue;
            }
            send_response(client_fd, 200, "OK", NULL, NULL);
        }

        DXhttp_print_debug("User %s successful", postType_str );
        free(req.username);
        free(req.passwd);
        free(req.signup_salt);
        free(ip);
        close(client_fd);
    } // end of while loop
}
