#include "common.h"

static struct redis_config *redis_conf = NULL;

void set_redis_config(struct redis_config *conf) {
    redis_conf = conf;
}

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

        char *ip = http_get_client_ip(client_fd, rece_buffer);
        int rt = redis_check_ip(ip, redis_conf, DatabaseIdx_SignUp) ;
        if (rt) {
            DXhttp_print_debug("IP check failed for %s, rt %d", ip, rt );
            send_response(client_fd, 422, "Unprocessable Entity", "22:%d", rt);
            free(ip);
            close(client_fd);
            continue;
        }

        if (strncmp(rece_buffer, "POST /signup", 12) != 0) {
            DXhttp_print_debug("Invalid request, expected POST /signup, got: %.12s", rece_buffer);
            send_response(client_fd, 422, "Unprocessable Entity", "10");
            free(ip);
            close(client_fd);
            continue;
        }

        char *body = strstr(rece_buffer, "\r\n\r\n");
        if (!body) {
            DXhttp_print_debug("No request body found");
            send_response(client_fd, 422, "Unprocessable Entity", "11");
            free(ip);
            close(client_fd);
            continue;
        }
        body += 4;

        int len = strlen(body);
        DXhttp_print_debug("body len (%d):[%s]", len, body);
        if (2 == len && '{' == body[0] && '}' == body[1]) { // {}
            char signup_sess[33] = {0}; // Declare once, used in both branches
            char signup_sesv[33] = {0}; // Declare once, used in both branches
            gen_a_new_md5sum_hex_32byte(signup_sess);
            gen_a_new_md5sum_hex_32byte(signup_sesv);
            //redis_save_key_to_redis_with_ttl(DatabaseIdx_SignUp, 300, "signup_sess", signup_sesv, signup_sess, redis_conf);
            rt = redis_set_key_value(redis_conf, DatabaseIdx_SignUp, "SET signup_sess:%s %s EX %d", signup_sesv, signup_sess, 300);
            //send_response(client_fd, 200, "OK", "{\"ver\": 1, \"signup_sess\": \"%s\"}", signup_sesv);
            send_response_with_new_signup_sess(client_fd, 200, signup_sess, signup_sesv);

            free(ip);
            close(client_fd);
            continue;
        }


        struct signup_request req = { NULL, NULL, NULL };
        rt = parse_signup_json(body, &req) ;
        if (rt) {
            send_response(client_fd, 422, "Unprocessable Entity", "12:%d", rt);
            if (req.username) free(req.username);
            if (req.passwd) free(req.passwd);
            if (req.signup_salt) free(req.signup_salt);
            // please notice : here , we must check the req.xxx, it maybe NULL. after that, it must be not NULL, and no more check is needed.
            free(ip);
            close(client_fd);
            continue;
        }
        DXhttp_print_debug("json looks OK");

        char subCode = 0 ;
        if ( !validate_username(req.username) )        subCode |= 1;
        if ( !validate_passwd(req.passwd) )            subCode |= 2;
        if ( !validate_signup_salt(req.signup_salt) )  subCode |= 4;
        if ( subCode ) {
            send_response(client_fd, 422, "Unprocessable Entity", "14:%d", subCode );
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }
        DXhttp_print_debug("username/passwd/signup_salt check more details: all looks good");

        char dbSavedSignUpSalt[33] ;
        rt = redis_get_string(redis_conf, DatabaseIdx_SignUp, 32, dbSavedSignUpSalt, "signup_sess:%s", req.signup_salt ) ;
        if ( rt ) {
            DXhttp_print_debug("no such a signup salt found!" );
            send_response(client_fd, 422, "Unprocessable Entity", "16:%d", rt );
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }

        char signUpCookieBuf[33] ;
        rt = cookie_extract(rece_buffer, n, signUpCookieBuf, sizeof(signUpCookieBuf), "signup_session" );
        if ( rt ) {
            DXhttp_print_debug("no such a signup salt found!" );
            send_response(client_fd, 422, "Unprocessable Entity", "18:%d", rt );
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }
        DXhttp_print_debug(" dbSavedSignUpSalt [%s], signUpCookieBuf [%s]", dbSavedSignUpSalt, signUpCookieBuf);

        rt = strncmp( dbSavedSignUpSalt, signUpCookieBuf, 33 );
        if ( 0 != rt ) {
            DXhttp_print_debug(" dbSavedSignUpSalt[%s] and signUpCookieBuf[%s] not equal", dbSavedSignUpSalt, signUpCookieBuf );
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
        rt = redis_get_int(redis_conf, DatabaseIdx_UserName, &tmpLong, "EXISTS user:%s", req.username ) ;
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
        DXhttp_print_debug("Username check ok, not exist. can signup if salt/hash is correct.");

        /* in this case, dbSavedSignUpSalt == signUpCookieBuf, no more check/calc, just save it.
        char hash[HASH_LEN + 1], salt[SALT_LEN + 1];
        if (! compute_signup_hash2(req.username, req.passwd, hash, salt)) { */

        rt = redis_set_key_value(redis_conf, DatabaseIdx_SignUp, "SET signupOK:%s 1 EX %d", ip, SIGNUP_OK_TTL);
        if ( rt ) {
            DXhttp_print_debug("[SET signupOK:%s 1 EX %d] failed, rt-> %d ", ip, SIGNUP_OK_TTL, rt);
            send_response(client_fd, 422, "Unprocessable Entity", "34:%d", rt); 
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }

        //if (!redis_store_user(req.username, hash, salt, ip, redis_conf)) {
        rt = redis_hset_key_value_pair( redis_conf, DatabaseIdx_UserName, &tmpLong, NEW_USER_TTL_30d,
                "HSET user:%s hash %s salt %s level 0 status active", req.username, req.passwd, req.signup_salt);
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

        DXhttp_print_debug("User signup successful");
        send_response(client_fd, 200, "OK", NULL);
        free(req.username);
        free(req.passwd);
        free(req.signup_salt);
        free(ip);
        close(client_fd);
    }
}
