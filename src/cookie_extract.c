#include "common.h"

int cookie_extract(const char *buffer, size_t n, char *output_buf, size_t output_buf_size, const char *cookie_name) {
    if (!buffer || !output_buf || !cookie_name || output_buf_size < 2) {
        return 125001;
    }

    // Limit scan to headers only
    const char *header_end = memmem(buffer, n, "\r\n\r\n", 4);
    if (!header_end) {
        return 125003; // No headers/body separator
    }
    size_t header_len = header_end - buffer;

    // Create a copy of headers to safely scan
    char header_copy[header_len + 1];
    memcpy(header_copy, buffer, header_len);
    header_copy[header_len] = '\0';

    // Scan line by line
    size_t name_len = strlen(cookie_name);
    char *line = strtok(header_copy, "\r\n");
    while (line) {
        if (strncasecmp(line, "Cookie:", 7) == 0) {
            char *cookies = line + 7;
            while (*cookies == ' ') cookies++;

            // Search for the cookie by name
            const char *p = cookies;
            while ((p = strstr(p, cookie_name))) {
                if ((p == cookies || *(p - 1) == ' ' || *(p - 1) == ';') &&
                    strncmp(p + name_len, "=", 1) == 0) {
                    p += name_len + 1; // skip "name="

                    // Find end of cookie value
                    const char *end = strpbrk(p, "; \r\n");
                    size_t val_len = end ? (size_t)(end - p) : strlen(p);

                    if (val_len >= output_buf_size) {
                        return 125005; // too long
                    }

                    memcpy(output_buf, p, val_len);
                    output_buf[val_len] = '\0';
                    return 0;
                }
                p += name_len;
            }
        }
        line = strtok(NULL, "\r\n");
    }

    return 125009; // Not found
}
