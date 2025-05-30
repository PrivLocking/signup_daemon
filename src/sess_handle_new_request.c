#include "common.h"

// rt = sess_handle_new_request(redis_conf, &req, client_fd );
int sess_handle_new_request(struct redis_config *redis_conf, struct session_request *req , int client_fd ) {
    char signup_sess[33] = {0}; // Declare once, used in both branches
    char signup_sesv[33] = {0}; // Declare once, used in both branches
    gen_a_new_md5sum_hex_32byte(signup_sess);
    gen_a_new_md5sum_hex_32byte(signup_sesv);
    int rt = redis_set_hset_key_value(redis_conf, DbIdx_ipCountX, 300, "SET %s_sess:%s:%s %s", postType_str, req->username, signup_sesv, signup_sess);
    if (rt) {
        DXhttp_print_debug("set key error:%d", rt );
        return 161001 ;
    }
    if ( 1 == postType_0signup_1login_2_admin ){
        char login_salt[33] = {0} ;
        rt = redis_get_hget_string(redis_conf, DatabaseIdx_salt_Login, 32, login_salt, "HGET loginUser:%s salt", req->username ) ;
        if (rt) {
            DXhttp_print_debug("set key error:%d", rt );
            return 161010 ;
        }

        //char buf[101] ; snprintf(buf, 100, "%s_sess=%s; HttpOnly; Max-Age=300", postType_str, signup_sess );
        //char *strArr[] = { buf, NULL } ;
        //send_response(client_fd, 200, "OK", strArr, "{\"ver\": 1, \"login_sess\": \"%s\", \"login_salt\": \"%s\"}", signup_sesv, login_salt) ;
        send_response_sess3(client_fd, signup_sess, signup_sesv, login_salt);

        return 0 ;
    }
    send_response_sess2(client_fd, signup_sess, signup_sesv);

    return 0 ;
}

