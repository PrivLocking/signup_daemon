#include "common.h"

void send_response(int client_fd, int status, const char *status_text, const char *return_fmt, ...) {
    char response[4096];  // Increased buffer size to handle larger formatted content
    char body[3072];      // Buffer for the body content
    va_list args;
    int content_length = 0;

    // Format the body content using vsnprintf if a format string is provided
    if (return_fmt != NULL) {
        va_start(args, return_fmt);
        content_length = vsnprintf(body, sizeof(body), return_fmt, args);
        va_end(args);
    } else {
        body[0] = '\0';  // Empty body if no format provided
    }

    // Format the HTTP response with headers and body
    snprintf(response, sizeof(response),
             "HTTP/1.1 %d %s\r\n"
             "Content-Length: %d\r\n"
             "Cache-Control: no-cache, no-store\r\n"
             "\r\n"
             "%s",
             status, status_text,
             content_length,
             body);

    // Send the response
    write(client_fd, response, strlen(response));

    DXprint_debug_rn( "%s", response );
}

void print_debug_rn( const char *fmt, ...) {
    if ( NULL == fmt ) return ;

    char tmpBuf[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(tmpBuf, 4096, fmt, args);
    va_end(args);

    // Debug output with escaped characters
    char debug_str[8192]; // Larger buffer to accommodate escaped characters
    char *dst = debug_str;
    for (const char *src = tmpBuf; *src != '\0' && dst < debug_str + sizeof(debug_str) - 3; src++) {
        if (*src == '\r') {
            *dst++ = '\\';
            *dst++ = 'r';
        } else if (*src == '\n') {
            *dst++ = '\\';
            *dst++ = 'n';
        } else {
            *dst++ = *src;
        }
    }
    *dst = '\0'; // Null-terminate the debug string

    print_debug("(%d)[%s]", strlen(tmpBuf), debug_str);
}
