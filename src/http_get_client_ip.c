#include "common.h"

char *http_get_client_ip(int client_fd, const char *buffer) {
    DXhttp_print_debug("%d [%.25s]", strlen( buffer ), buffer );
    // First, try to extract from HTTP headers
    if (buffer) {
        // Try to get X-Real-IP
        const char *x_real_ip = strcasestr(buffer, "\nX-Real-IP: ");
        if (x_real_ip) {
            x_real_ip += 12; // Skip header name
            char *ip = malloc(16); // IPv4 max len
            if (!ip) return strdup("127.0.0.2");
            
            int i = 0;
            while (*x_real_ip && *x_real_ip != '\r' && *x_real_ip != '\n' && i < 15) {
                ip[i++] = *x_real_ip++;
            }
            ip[i] = '\0';
            DXhttp_print_debug("Found X-Real-IP: %s", ip);
            return ip;
        }
        
        // Try to get X-Forwarded-For
        const char *x_forwarded_for = strcasestr(buffer, "\nX-Forwarded-For: ");
        if (x_forwarded_for) {
            x_forwarded_for += 17; // Skip header name
            char *ip = malloc(16); // IPv4 max len
            if (!ip) return strdup("127.0.0.2");
            
            int i = 0;
            while (*x_forwarded_for && *x_forwarded_for != '\r' && *x_forwarded_for != '\n' && *x_forwarded_for != ',' && i < 15) {
                ip[i++] = *x_forwarded_for++;
            }
            ip[i] = '\0';
            DXhttp_print_debug("Found X-Forwarded-For: %s", ip);
            return ip;
        }
    }
    
    // If we couldn't extract from headers, try to get from socket
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);
    if (getpeername(client_fd, (struct sockaddr*)&addr, &addrlen) == 0) {
        if (addr.ss_family == AF_INET) {
            char *ip = malloc(16); // IPv4 max len
            if (!ip) return strdup("127.0.0.2");
            
            struct sockaddr_in *s = (struct sockaddr_in*)&addr;
            inet_ntop(AF_INET, &s->sin_addr, ip, 16);
            DXhttp_print_debug("Extracted socket IP: %s", ip);
            return ip;
        }
    }
    
    // Default IP
    DXhttp_print_debug("Using default IP: 127.0.0.2");
    return strdup("127.0.0.2");
}

