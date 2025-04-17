#include "common.h"

static struct redis_config *redis_conf = NULL;

void set_redis_config(struct redis_config *conf) {
    redis_conf = conf;
}

void http_serve(void) {
    int server_fd = get_server_fd();
    if (server_fd == -1) {
        http_print_debug("Invalid server socket");
        return;
    }

    struct timeval timeout = { 1, 0 };
    if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        http_print_debug("Setsockopt timeout failed: %s", strerror(errno));
        return;
    }

    while (http_should_keep_running()) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            if (errno == EINTR) continue;
            http_print_debug("Accept failed: %s", strerror(errno));
            sleep(1);
            continue;
        }

        http_print_debug("Accepted client connection");

        char buffer[REQUEST_MAX_SIZE + 1];
        ssize_t n = read(client_fd, buffer, REQUEST_MAX_SIZE);
        if (n <= 0) {
            http_print_debug("Read failed or connection closed");
            close(client_fd);
            continue;
        }
        buffer[n] = '\0';
        http_print_debug("buffer is :[%s]", buffer);

        if (strncmp(buffer, "POST /signup", 12) != 0) {
            http_print_debug("Invalid request, expected POST /signup, got: %.12s", buffer);
            send_response(client_fd, 422, "Unprocessable Entity", "10");
            close(client_fd);
            continue;
        }

        char *body = strstr(buffer, "\r\n\r\n");
        if (!body) {
            http_print_debug("No request body found");
            send_response(client_fd, 422, "Unprocessable Entity", "11");
            close(client_fd);
            continue;
        }
        body += 4;

        /*
        int len = strlen( body ) ;
        http_print_debug("body len (%d):[%s]", len, body );
        if ( 2 == len && '{' == body[0] && '}' == body[1] ){
            char *cookie_session = strstr(buffer, "signup_session=");
            if (cookie_session) {
                char signup_sess[33] = {0};
                int sessLen = sscanf(cookie_session, "signup_session=%32s", signup_sess);
                if ( 32 != sessLen ) {
                    http_print_debug("signup_session len error: %d : [%s]", sessLen, signup_sess );
                    send_response(client_fd, 422, "Unprocessable Entity", "21");
                } else {
                    if ( !redis_find_signup_sess_and_reset_its_TTL300( signup_sess, redis_conf )) { // try to find current signup_sess from redis
                        // not found old signup_sess
                        gen_a_new_md5sum_hex_32byte( signup_sess ) ;
                        redis_save_signup_sess_with_TTL300( signup_sess, redis_conf ) ;
                        send_response_with_new_signup_sess(client_fd, 200, signup_sess );
                    } else {
                        send_response(client_fd, 422, "Unprocessable Entity", signup_sess ); // no new session is needed.
                    }
                }
            } else {
                http_print_debug("no signup_session found. try to gen it." );
                gen_a_new_md5sum_hex_32byte( signup_sess ) ;
                redis_save_signup_sess_with_TTL300( signup_sess, redis_conf ) ;
                send_response_with_new_signup_sess(client_fd, 200, signup_sess );
            }
            close(client_fd);
            continue;
        }
        */

        struct signup_request req = { NULL, NULL };
        if (!parse_json(body, &req)) {
            send_response(client_fd, 422, "Unprocessable Entity", "12");
            if (req.username) free(req.username);
            if (req.passwd) free(req.passwd);
            close(client_fd);
            continue;
        }

        if (!validate_username(req.username) || !validate_passwd(req.passwd)) {
            send_response(client_fd, 422, "Unprocessable Entity", "13");
            free(req.username);
            free(req.passwd);
            close(client_fd);
            continue;
        }

        char *ip = get_client_ip(client_fd, buffer);
        if (!redis_check_ip(ip, redis_conf)) {
            http_print_debug("IP check failed for %s", ip);
            send_response(client_fd, 422, "Unprocessable Entity", "14");
            free(req.username);
            free(req.passwd);
            free(ip);
            close(client_fd);
            continue;
        }

        if (!redis_check_username(req.username, redis_conf)) {
            http_print_debug("Username check failed for %s", req.username);
            redis_increment_failed(ip, redis_conf);
            send_response(client_fd, 422, "Unprocessable Entity", "15");
            free(req.username);
            free(req.passwd);
            free(ip);
            close(client_fd);
            continue;
        }

        char hash[HASH_LEN + 1], salt[SALT_LEN + 1];
        if (!compute_hash(req.username, req.passwd, hash, salt)) {
            http_print_debug("Hash computation failed");
            redis_increment_failed(ip, redis_conf);
            send_response(client_fd, 422, "Unprocessable Entity", "16");
            free(req.username);
            free(req.passwd);
            free(ip);
            close(client_fd);
            continue;
        }

        if (!redis_store_user(req.username, hash, salt, ip, redis_conf)) {
            http_print_debug("Failed to store user in Redis");
            redis_increment_failed(ip, redis_conf);
            send_response(client_fd, 503, "Service Unavailable", NULL);
            free(req.username);
            free(req.passwd);
            free(ip);
            close(client_fd);
            continue;
        }

        http_print_debug("User signup successful");
        send_response(client_fd, 200, "OK", NULL);
        free(req.username);
        free(req.passwd);
        free(ip);
        close(client_fd);
    }
}
