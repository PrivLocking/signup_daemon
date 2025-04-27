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
            send_response(client_fd, 422, "Unprocessable Entity", "20:%d:[%.25s]", rt, rece_buffer);
            close(client_fd);
            continue;
        }
        DXhttp_print_debug("get postType_0signup_1login_2_admin is :%d [%s]", postType_0signup_1login_2_admin, postType_str );

        char *ip = http_get_client_ip(client_fd, rece_buffer);
        rt = redis_check_ip(ip, redis_conf, DbIdx_ipCountX) ;
        if (rt) {
            DXhttp_print_debug("IP check failed for %s, rt %d", ip, rt );
            send_response(client_fd, 422, "Unprocessable Entity", "22:%d", rt);
            free(ip);
            close(client_fd);
            continue;
        }

        char *body = strstr(rece_buffer, "\r\n\r\n");
        if (!body) {
            DXhttp_print_debug("No request body found");
            send_response(client_fd, 422, "Unprocessable Entity", "24");
            free(ip);
            close(client_fd);
            continue;
        }
        body += 4;

        int len = strlen(body);
        DXhttp_print_debug("body len (%d):[%.20s]", len, body);
        // to combine signup/login, the empty body change from '{}' to '{"ver":1}'
        /*
        if (2 == len && '{' == body[0] && '}' == body[1]) { // {}
        }
        */


        struct signup_request req = { 0, NULL, NULL, NULL };
        rt = parse_signup_json(body, &req) ;
        if (rt) {
            send_response(client_fd, 422, "Unprocessable Entity", "10:%d", rt);
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
            char signup_sess[33] = {0}; // Declare once, used in both branches
            char signup_sesv[33] = {0}; // Declare once, used in both branches
            gen_a_new_md5sum_hex_32byte(signup_sess);
            gen_a_new_md5sum_hex_32byte(signup_sesv);
            rt = redis_set_key_value(redis_conf, DbIdx_ipCountX, "SET %s_sess:%s %s EX %d", postType_str, signup_sesv, signup_sess, 300);
            if (rt) {
                DXhttp_print_debug("set key error:%d", rt );
                free(ip);
                close(client_fd);
                continue;
            }
            //char login_salt[33] = {0} ;
            if ( 1 == postType_0signup_1login_2_admin ){
                //rt = redis_get_string(redis_conf, 5, 32, login_salt, "loginUser:%s", req.username ) ;
            }
            else if ( 2 == postType_0signup_1login_2_admin ){
            }
            send_response_with_new_tmp_sess(client_fd, 200, signup_sess, signup_sesv);

            free(ip);
            close(client_fd);
            continue;
        }

        char dbSavedVerifyTmpSalt[33] ;
        rt = redis_get_string(redis_conf, DbIdx_ipCountX, 32, dbSavedVerifyTmpSalt, "%s_sess:%s", postType_str, req.signup_salt ) ;
        if ( rt ) {
            DXhttp_print_debug("no such a %s salt found! :%d" , postType_str, rt);
            send_response(client_fd, 422, "Unprocessable Entity", "16:%d", rt );
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
            send_response(client_fd, 422, "Unprocessable Entity", "18:%d", rt );
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
            send_response(client_fd, 422, "Unprocessable Entity", "20");
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
        if ( rt || (tmpLong != 0 )) {
            DXhttp_print_debug("Username check failed, or already exist for %s, rt-> %d, tmpLong -> %ld", req.username, rt, tmpLong );
            send_response(client_fd, 422, "Unprocessable Entity", "24:%d:%ld", rt, tmpLong); // failed , or username exist,
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }
        DXhttp_print_debug("Username check ok, not exist. can %s if salt/hash is correct.", postType_str );

        /* in this case, dbSavedVerifyTmpSalt == verifyTmpValue, no more check/calc, just save it.
        char hash[HASH_LEN + 1], salt[SALT_LEN + 1];
        if (! compute_signup_hash2(req.username, req.passwd, hash, salt)) { */

        rt = redis_set_key_value(redis_conf, DbIdx_ipCountX, "SET signupOK:%s 1 EX %d", ip, SIGNUP_OK_TTL);
        if ( rt ) {
            DXhttp_print_debug("[SET %sOK:%s 1 EX %d] failed, rt-> %d ", postType_str, ip, SIGNUP_OK_TTL, rt);
            send_response(client_fd, 422, "Unprocessable Entity", "34:%d", rt); 
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }

        if ( 0 == postType_0signup_1login_2_admin ) {
            rt = redis_hset_key_value_pair( redis_conf, DatabaseIdx_salt_Login, &tmpLong, NEW_USER_TTL_30d,
                    "HSET loginUser:%s hash %s salt %s level 0 status active", req.username, req.passwd, req.signup_salt);
        } else {
            rt = 8388188 ; // under constructing
        }
        if ( rt ) {
            DXhttp_print_debug("HSET failed, rt-> %d ", rt);
            send_response(client_fd, 422, "Unprocessable Entity", "44:%d", rt); 
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }

        DXhttp_print_debug("User %s successful", postType_str );
        send_response(client_fd, 200, "OK", NULL);
        free(req.username);
        free(req.passwd);
        free(req.signup_salt);
        free(ip);
        close(client_fd);
    }
}
