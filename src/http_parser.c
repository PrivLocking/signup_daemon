#include "common.h"

// {"username":"aaabbbcccdddeee4","signup_salt":"b0e082f57264abc6a67bc78cbd4ae159","passwd":"5f669ecdfa48510b198c9019cb13691daab2b279a427d8293c0e0f2a3d316c75"}]
// {"ver":1,"username":"aaabbbcccdddeee11","login_salt":"bafe84bbcaa3daefcaef5eae10a04930","passwd":"7f75ec840a0461ff9ccd49405b6ffdccb5181ab02efaaea7833f5c216cb5e24f"}
bool parse_session_json(const char *body, struct session_request *req) {
    if ( !body || !req ) {
        DXhttp_print_debug("parameter error" );
        return 123010;
    }

    json_error_t error;
    json_t *root = json_loads(body, 0, &error);
    if (!root) {
        DXhttp_print_debug("JSON parse error: %s", error.text);
        return 123015;
    }

    char saltName[64];  // Make sure this is large enough
    snprintf(saltName, sizeof(saltName), "%s_salt", postType_str );

    json_t *base_salt = json_object_get(root, saltName);
    json_t *username = json_object_get(root, "username");
    json_t *passwd = json_object_get(root, "passwd");
    json_t *ver = json_object_get(root, "ver");

    if ( !ver || !json_is_integer( ver ) ){
        DXhttp_print_debug("JSON missing ver or ver error");
        json_decref(root);
        return 123020;
    }
    req -> ver = json_integer_value( ver ) ;

    if (!json_is_string(username)) {
        DXhttp_print_debug("JSON username error" );
        return 123030 ;
    }
    req->username = strdup(json_string_value(username));
    for (size_t i = 0; req->username[i]; i++) {
        req->username[i] = tolower(req->username[i]);
    }

    if ( !validate_username(req->username) ) {
        DXhttp_print_debug("JSON username error:%d,[%s]", strlen(req->username), req->username);
        return 123033 ;
    }

    if (!json_is_string(passwd) && !json_is_string(base_salt) ) { // only ver and username, it is req for new. base check passed.
        json_decref(root);

        DXhttp_print_debug("Parsed JSON: ver:%d, username=%s only, it is req for new ", req->ver, req->username);
        return 0;
    }

    if ( !json_is_string(passwd) || !json_is_string(base_salt) ) {
        DXhttp_print_debug("JSON missing username or passwd or base_salt");
        json_decref(root);
        return 123040;
    }

    if (json_object_size(root) > 4) {
        DXhttp_print_debug("JSON contains extra fields");
        json_decref(root);
        return 123050;
    }

    req->passwd      = strdup(json_string_value(passwd));
    req->signup_salt = strdup(json_string_value(base_salt));

    if ( !validate_passwd(req->passwd) ) {
        DXhttp_print_debug("passwd hash error");
        json_decref(root);
        return 123060;
    }

    if ( !validate_signup_salt(req->signup_salt) ) {
        DXhttp_print_debug("base_salt error");
        json_decref(root);
        return 123070;
    }

    json_decref(root);

    DXhttp_print_debug("Parsed JSON: ver:%d, username=%s, base_salt:%s, passwd:%s", req->ver, req->username, req->signup_salt, req->passwd );
    return 0;
}
