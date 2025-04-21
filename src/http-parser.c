#include "common.h"

// {"username":"aaabbbcccdddeee4","signup_salt":"b0e082f57264abc6a67bc78cbd4ae159","passwd":"5f669ecdfa48510b198c9019cb13691daab2b279a427d8293c0e0f2a3d316c75"}]
bool parse_signup_json(const char *body, struct signup_request *req) {
    json_error_t error;
    json_t *root = json_loads(body, 0, &error);
    if (!root) {
        http_print_debug("JSON parse error: %s", error.text);
        return 123001;
    }

    json_t *username = json_object_get(root, "username");
    json_t *passwd = json_object_get(root, "passwd");
    json_t *signup_salt = json_object_get(root, "signup_salt");
    if (!json_is_string(username) || !json_is_string(passwd) || !json_is_string(signup_salt) ) {
        http_print_debug("JSON missing username or passwd or signup_salt");
        json_decref(root);
        return 123003;
    }

    if (json_object_size(root) > 3) {
        http_print_debug("JSON contains extra fields");
        json_decref(root);
        return 123005;
    }

    req->username = strdup(json_string_value(username));
    req->passwd = strdup(json_string_value(passwd));
    req->signup_salt = strdup(json_string_value(signup_salt));

    if ( strlen( req->username ) < 8 ) {
        http_print_debug("username too short");
        json_decref(root);
        return 123007;
    }

    if ( !string_check_a2f_0to9( req->passwd, 64 )) {
        http_print_debug("passwd hash error");
        json_decref(root);
        return 123009;
    }

    if ( !string_check_a2f_0to9( req->signup_salt, 32 )) {
        http_print_debug("signup_salt error");
        json_decref(root);
        return 123011;
    }

    json_decref(root);

    for (size_t i = 0; req->username[i]; i++) {
        req->username[i] = tolower(req->username[i]);
    }

    http_print_debug("Parsed JSON: username=%s", req->username);
    return 0;
}
