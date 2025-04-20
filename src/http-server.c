#include "common.h"

static struct redis_config *redis_conf = NULL;

void set_redis_config(struct redis_config *conf) {
    redis_conf = conf;
}

void http_serve(void) {
    int server_fd = get_server_fd();
    if (server_fd == -1) {
        DBhttp_print_debug("Invalid server socket");
        return;
    }

    struct timeval timeout = { 1, 0 };
    if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        DBhttp_print_debug("Setsockopt timeout failed: %s", strerror(errno));
        return;
    }

    while (http_should_keep_running()) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            if (errno == EINTR) continue;
            DBhttp_print_debug("Accept failed: %s", strerror(errno));
            sleep(1);
            continue;
        }

        DBhttp_print_debug("Accepted client connection");

        char buffer[REQUEST_MAX_SIZE + 1];
        ssize_t n = read(client_fd, buffer, REQUEST_MAX_SIZE);
        if (n <= 0) {
            DBhttp_print_debug("Read failed or connection closed");
            close(client_fd);
            continue;
        }
        buffer[n] = '\0';
        DBhttp_print_debug("buffer is :[%s]", buffer);

        if (strncmp(buffer, "POST /signup", 12) != 0) {
            DBhttp_print_debug("Invalid request, expected POST /signup, got: %.12s", buffer);
            send_response(client_fd, 422, "Unprocessable Entity", "10");
            close(client_fd);
            continue;
        }

        char *body = strstr(buffer, "\r\n\r\n");
        if (!body) {
            DBhttp_print_debug("No request body found");
            send_response(client_fd, 422, "Unprocessable Entity", "11");
            close(client_fd);
            continue;
        }
        body += 4;

        int len = strlen(body);
        DBhttp_print_debug("body len (%d):[%s]", len, body);
        if (2 == len && '{' == body[0] && '}' == body[1]) { // {}
            /*
            char signup_sess[33] = {0}; // Declare once, used in both branches
            char *cookie_session = strstr(buffer, "signup_session=");
            if (cookie_session) {
                int i = 0;
                char *p = cookie_session + 15; // Skip "signup_session="

                // Only allow hexadecimal characters (0-9, a-f, A-F)
                while (i < 32 && *p && (
                            (*p >= '0' && *p <= '9') 
                            || (*p >= 'a' && *p <= 'f') 
                            // || (*p >= 'A' && *p <= 'F') // because it is cooides, it won't change case-sensive, we gen it to 0-9a-f only.
                            )) {
                    signup_sess[i++] = *p++;
                }
                signup_sess[i] = '\0';

                if (i != 32) {
                    DBhttp_print_debug("signup_session len error: %d : [%s]", i, signup_sess);
                    send_response(client_fd, 422, "Unprocessable Entity", "31");
                } else {
                    if (!redis_find_signup_sess_and_reset_its_TTL300(signup_sess, redis_conf)) {
                        // not found old signup_sess
                        gen_a_new_md5sum_hex_32byte(signup_sess);
                        redis_save_signup_sess_with_TTL300(signup_sess, redis_conf);
                        send_response_with_new_signup_sess(client_fd, 200, signup_sess);
                    } else {
                        send_response(client_fd, 422, "Unprocessable Entity", signup_sess); // no new session is needed.
                    }
                }
            } else {
                DBhttp_print_debug("no signup_session found. try to gen it.");
                gen_a_new_md5sum_hex_32byte(signup_sess);
                redis_save_signup_sess_with_TTL300(signup_sess, redis_conf);
                send_response_with_new_signup_sess(client_fd, 200, signup_sess);
            }
            */
            char *ip = get_client_ip(client_fd, buffer);
            if ( ip ) {
                if (!redis_check_ip(ip, redis_conf)) {
                    DBhttp_print_debug("IP check failed for %s", ip);
                    send_response(client_fd, 422, "Unprocessable Entity", "31");
                    free(ip);
                    close(client_fd);
                    continue;
                }
                free(ip);
            }
            char signup_sess[33] = {0}; // Declare once, used in both branches
            char signup_sesv[33] = {0}; // Declare once, used in both branches
            gen_a_new_md5sum_hex_32byte(signup_sess);
            gen_a_new_md5sum_hex_32byte(signup_sesv);
            redis_save_key_to_redis_with_ttl(5, 300, "signup_sess", signup_sess, signup_sesv, redis_conf);
            send_response(client_fd, 200, "OK", "{\"ver\": 1, \"signup_sess\": \"%s\"}", signup_sesv);
            close(client_fd);
            continue;
        }


        struct signup_request req = { NULL, NULL, NULL };
        if (!parse_json(body, &req)) {
            send_response(client_fd, 422, "Unprocessable Entity", "12");
            if (req.username) free(req.username);
            if (req.passwd) free(req.passwd);
            if (req.signup_salt) free(req.signup_salt);
            close(client_fd);
            continue;
        }

        char subCode = 0 ;
        if ( !validate_username(req.username) ) subCode |= 1;
        if ( !validate_passwd(req.passwd) )     subCode |= 2;
        if ( !validate_salt(req.signup_salt) )  subCode |= 4;
        if ( subCode ) {
            send_response(client_fd, 422, "Unprocessable Entity", "14:%d", subCode );
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            close(client_fd);
            continue;
        }

        char *ip = get_client_ip(client_fd, buffer);
        if (!redis_check_ip(ip, redis_conf)) {
            DBhttp_print_debug("IP check failed for %s", ip);
            send_response(client_fd, 422, "Unprocessable Entity", "16");
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }

        if (!redis_check_username(req.username, redis_conf)) {
            DBhttp_print_debug("Username check failed for %s", req.username);
            redis_increment_failed(ip, redis_conf);
            send_response(client_fd, 422, "Unprocessable Entity", "18");
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }

        char hash[HASH_LEN + 1], salt[SALT_LEN + 1];
        if (!compute_hash(req.username, req.passwd, hash, salt)) {
            DBhttp_print_debug("Hash computation failed");
            redis_increment_failed(ip, redis_conf);
            send_response(client_fd, 422, "Unprocessable Entity", "20");
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }

        if (!redis_store_user(req.username, hash, salt, ip, redis_conf)) {
            DBhttp_print_debug("Failed to store user in Redis");
            redis_increment_failed(ip, redis_conf);
            send_response(client_fd, 503, "Service Unavailable", NULL);
            free(req.username);
            free(req.passwd);
            free(req.signup_salt);
            free(ip);
            close(client_fd);
            continue;
        }

        DBhttp_print_debug("User signup successful");
        send_response(client_fd, 200, "OK", NULL);
        free(req.username);
        free(req.passwd);
        free(req.signup_salt);
        free(ip);
        close(client_fd);
    }
}
