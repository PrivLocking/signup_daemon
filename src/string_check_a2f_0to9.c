#include "common.h"

bool string_check_a2f_0to9(char* buf, int len) {
    // Check for NULL pointer
    if (NULL == buf) {
        DBprint_debug("met NULL string!");
        return false;
    }

    // Calculate length only once
    int actual_len = strlen(buf);
    if (actual_len != len) {
        DBprint_debug("string len error: wanted %d, real len %d", len, actual_len);
        return false;
    }

    // Check each character
    for (int i = 0; i < len; i++) {
        char c = buf[i];
        // Check if character is 0-9 or a-f
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) {
            continue;
        }
        DBprint_debug("string out of allow char set: %c, (%d)", c, c);
        return false;
    }

    return true;
}

