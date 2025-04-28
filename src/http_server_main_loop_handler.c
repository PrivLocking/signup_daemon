#include "common.h"

int http_server_main_loop_handler( char **ip, struct redis_config *redis_conf, int *client_fd, char rece_buffer[], ssize_t n, struct session_request *req ) {
    if ( !client_fd ) {
        DXhttp_print_debug("Parameter error: !client_fd" );
        return 211000;
    }
    if ( !ip || !redis_conf ) {
        send_response(*client_fd, 422, "Unprocessable Entity", NULL, "20" );
        DXhttp_print_debug("Parameter error: !ip  or !redis_conf " );
        return 211010;
    }

    *ip = http_get_client_ip(*client_fd, rece_buffer);

    int rt = redis_check_ip(*ip, redis_conf, DbIdx_ipCountX) ;
    if (rt) {
        DXhttp_print_debug("IP check failed for %s, rt %d", ip, rt );
        send_response(*client_fd, 422, "Unprocessable Entity", NULL, "22:%d", rt);
        return 211021;
    }

    char *body = strstr(rece_buffer, "\r\n\r\n");
    if (!body) {
        DXhttp_print_debug("No request body found");
        send_response(*client_fd, 422, "Unprocessable Entity", NULL, "24");
        return 211031;
    }
    body += 4;

    int len = strlen(body);
    DXhttp_print_debug("body len (%d):[%.40s]", len, body);
    // to combine signup/login, the empty body change from '{}' to '{"ver":1}'
    /*
       if (2 == len && '{' == body[0] && '}' == body[1]) { // {}
       }
       */


    rt = parse_session_json(body, req) ;
    if (rt) {
        send_response(*client_fd, 422, "Unprocessable Entity", NULL, "30:%d", rt);
        return 211041;
    }
    DXhttp_print_debug("json looks OK: %d,[%s],[%s],[%s]", req->ver, req->username, req->passwd, req->signup_salt);

    if ( !req->passwd && !req->signup_salt) { 
        DXhttp_print_debug("Empty req, means new req->" );
        rt = sess_handle_new_request(redis_conf, req, *client_fd );
        if ( rt ) {
            DXhttp_print_debug("sess_handle_new_request met error:%d", rt );
            return 211051;
        }

        return 1;
    }
    DXhttp_print_debug("nor-Empty req, need more analyze." );

    char dbSavedVerifyTmpSalt[33] ;
    rt = redis_get_hget_string(redis_conf, DbIdx_ipCountX, 32, dbSavedVerifyTmpSalt, "GET %s_sess:%s:%s", postType_str, req->username, req->signup_salt ) ;
    if ( rt ) {
        DXhttp_print_debug("no such a %s salt found! :%d" , postType_str, rt);
        send_response(*client_fd, 422, "Unprocessable Entity", NULL, "34:%d", rt );
        return 211061;
    }

    char verifyTmpValue[33] ;
    rt = cookie_extract(rece_buffer, n, verifyTmpValue, sizeof(verifyTmpValue), "%s_sess", postType_str );
    if ( rt ) {
        DXhttp_print_debug("no such a %s cookie found!  :%d", postType_str, rt );
        send_response(*client_fd, 422, "Unprocessable Entity", NULL, "38:%d", rt );
        return 211071;
    }
    DXhttp_print_debug(" dbSavedVerifyTmpSalt [%s], verifyTmpValue [%s]", dbSavedVerifyTmpSalt, verifyTmpValue);

    rt = strncmp( dbSavedVerifyTmpSalt, verifyTmpValue, 33 );
    if ( 0 != rt ) {
        DXhttp_print_debug(" dbSavedVerifyTmpSalt[%s] and verifyTmpValue[%s] not equal", dbSavedVerifyTmpSalt, verifyTmpValue );
        send_response(*client_fd, 422, "Unprocessable Entity", NULL, "39");
        return 211081;
    }

    //rt = redis_check_username(req->username, redis_conf) ;
    long tmpLong ;
    tmpLong = -1 ;
    rt = redis_get_int(redis_conf, DatabaseIdx_salt_Login, &tmpLong, "EXISTS %sUser:%s", postType_str, req->username ) ;
    if ( rt || (tmpLong != 1 )) {
        DXhttp_print_debug("Username check failed, or already exist for %s, rt-> %d, tmpLong -> %ld", req->username, rt, tmpLong );
        send_response(*client_fd, 422, "Unprocessable Entity", NULL, "24:%d:%ld", rt, tmpLong); // failed , or username exist,
        return 211091;
    }
    DXhttp_print_debug("Username check : exist. Now, do more check on %s to see if salt/hash is correct.", postType_str );

    /* in this case, dbSavedVerifyTmpSalt == verifyTmpValue, no more check/calc, just save it.
       char hash[HASH_LEN + 1], salt[SALT_LEN + 1];
       if (! compute_signup_hash2(req->username, req->passwd, hash, salt)) { */

    if ( 0 == postType_0signup_1login_2_admin ) {
        rt = redis_set_hset_key_value(redis_conf, DbIdx_ipCountX, SIGNUP_OK_TTL, "SET signupOK:%s 1", ip);
        if ( rt ) {
            DXhttp_print_debug("[SET %sOK:%s 1 EX %d] failed, rt-> %d ", postType_str, ip, SIGNUP_OK_TTL, rt);
            send_response(*client_fd, 422, "Unprocessable Entity", NULL, "34:%d", rt); 
            return 211101;
        }

        //rt = redis_hset_key_value_pair( redis_conf, DatabaseIdx_salt_Login, &tmpLong, NEW_USER_TTL_30d,
        //        "HSET loginUser:%s hash %s salt %s level 0 status active", req->username, req->passwd, req->signup_salt);
        rt = redis_set_hset_key_value( redis_conf, DatabaseIdx_salt_Login, NEW_USER_TTL_30d,
                "HSET loginUser:%s hash %s salt %s level 0 status active", req->username, req->passwd, req->signup_salt);
        if ( rt ) {
            DXhttp_print_debug("HSET for %s failed, rt-> %d", postType_str, rt );
            send_response(*client_fd, 422, "Unprocessable Entity", NULL, "44:%d", rt); 
            return 211111;
        }
        send_response(*client_fd, 200, "OK", NULL, NULL);

    } else {
        rt = 8388188 ; // under constructing
        if ( rt ) {
            DXhttp_print_debug("HSET for %s failed, rt-> %d", postType_str, rt );
            send_response(*client_fd, 422, "Unprocessable Entity", NULL, "54:%d", rt); 
            return 211121;
        }
        send_response(*client_fd, 200, "OK", NULL, NULL);
    }

    return 0;
}
