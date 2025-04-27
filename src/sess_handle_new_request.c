#include "common.h"

// rt = redis_set_key_value(redis_conf, DbIdx_ipCount[postType_0signup_1login_2_admin], "SET signup_sess:%s %s EX %d", signup_sesv, signup_sess, 300);
//int sess_handle_new_request(struct redis_config *conf, int databaseIdx, const char *fmt, ... ) {
//int sess_handle_new_request(struct redis_config *conf, int databaseIdx, const char *fmt, ... ) {
// rt = sess_handle_new_request(redis_conf, req, client_fd );
int sess_handle_new_request(struct redis_config *redis_conf, struct session_request *req , int client_fd ) {
    char signup_sess[33] = {0}; // Declare once, used in both branches
    char signup_sesv[33] = {0}; // Declare once, used in both branches
    gen_a_new_md5sum_hex_32byte(signup_sess);
    gen_a_new_md5sum_hex_32byte(signup_sesv);
    int rt = redis_set_key_value(redis_conf, DbIdx_ipCountX, "SET %s_sess:%s %s EX %d", postType_str, signup_sesv, signup_sess, 300);
    if (rt) {
        DXhttp_print_debug("set key error:%d", rt );
        //if (req->passwd) free(req->passwd);
        //if (req->signup_salt) free(req->signup_salt);
        return 161001 ;
    }
    //char login_salt[33] = {0} ;
    if ( 1 == postType_0signup_1login_2_admin ){
        //rt = redis_get_string(redis_conf, 5, 32, login_salt, "loginUser:%s", req->username ) ;
    }
    else if ( 2 == postType_0signup_1login_2_admin ){
    }
    send_response_with_new_tmp_sess(client_fd, 200, signup_sess, signup_sesv);

    return 0 ;
}

