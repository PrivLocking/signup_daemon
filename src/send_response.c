#include "common.h"

#define RESPONSE_BUFFER_SIZE 4096
void print_debug_rn( const char *fmt, ...) {
    if ( NULL == fmt ) return ;

    char tmpBuf[RESPONSE_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(tmpBuf, RESPONSE_BUFFER_SIZE, fmt, args);
    va_end(args);

    // Debug output with escaped characters
    char debug_str[RESPONSE_BUFFER_SIZE + RESPONSE_BUFFER_SIZE]; // Larger buffer to accommodate escaped characters
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

// example : send_response(client_fd, 422, "Unprocessable Entity", NULL, "10:%d", rt);
void send_response(int client_fd, int status, const char *status_text, char * cookieArr[], const char *fmt, ...) {
    char response[RESPONSE_BUFFER_SIZE];  // Increased buffer size to handle larger formatted content
    va_list args;
    int offset = 0;

    offset = snprintf(response, RESPONSE_BUFFER_SIZE,
            "HTTP/1.1 %d %s\r\n"
            "Cache-Control: no-cache, no-store\r\n"
            ,
            status, status_text
            );

    if ( NULL != cookieArr ) {
        for (int i = 0; cookieArr[i] != NULL; i++) {  // Check each string for NULL
            offset += snprintf(response + offset, RESPONSE_BUFFER_SIZE - offset,
                    "Set-Cookie: %s\r\n", cookieArr[i]);
        }
    }

    // Format the body content using vsnprintf if a format string is provided
    if (fmt != NULL) {
        int content_length = 0;
        char body[3072];      // Buffer for the body content
        va_start(args, fmt);
        content_length = vsnprintf(body, sizeof(body), fmt, args);
        va_end(args);
        offset += snprintf(response + offset, RESPONSE_BUFFER_SIZE - offset,
                "Content-Length: %d\r\n"
                "\r\n"
                "%s",
                content_length,
                body);
    } else {
        offset += snprintf(response + offset, RESPONSE_BUFFER_SIZE - offset,
                "Content-Length: 0\r\n"
                "\r\n"
                );
    }

    // Send the response
    write(client_fd, response, offset);

    DXprint_debug_rn( "(%d)%s", offset, response );
}

