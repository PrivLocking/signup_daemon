#include "common.h"

bool parse_json(const char *body, struct signup_request *req) {
    json_error_t error;
    json_t *root = json_loads(body, 0, &error);
    if (!root) {
        http_print_debug("JSON parse error: %s", error.text);
        return false;
    }

    json_t *username = json_object_get(root, "username");
    json_t *passwd = json_object_get(root, "passwd");
    if (!json_is_string(username) || !json_is_string(passwd)) {
        http_print_debug("JSON missing username or passwd");
        json_decref(root);
        return false;
    }

    if (json_object_size(root) > 2) {
        http_print_debug("JSON contains extra fields");
        json_decref(root);
        return false;
    }

    req->username = strdup(json_string_value(username));
    req->passwd = strdup(json_string_value(passwd));
    json_decref(root);

    for (size_t i = 0; req->username[i]; i++) {
        req->username[i] = tolower(req->username[i]);
    }

    http_print_debug("Parsed JSON: username=%s", req->username);
    return true;
}
