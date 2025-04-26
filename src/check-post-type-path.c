#include "common.h"

/**
 * Checks if the buffer contains a POST request where the path ends with one of the 
 * strings in postReqAhead after the last slash, with optimized memory usage.
 * 
 * @param buffer The received buffer to check
 * @param n The size of the buffer
 */
int check_post_type_path(char *buffer, size_t n) {
    // Check minimum buffer size to avoid potential issues
    if (buffer == NULL || n < 8) { // "POST/x " minimum length
        return -1410001;
    }
    
    // Check if it starts with "POST" (case-sensitive)
    if (strncmp(buffer, "POST", 4) != 0) {
        return -1410011;
    }
    
    // Skip "POST" and any spaces that follow (or no spaces)
    char *path_start = buffer + 4;
    
    // Check if there's a space after POST
    if (path_start < buffer + n && *path_start == ' ') {
        // Skip all spaces
        while (path_start < buffer + n && *path_start == ' ') {
            path_start++;
        }
    }
    
    // If we've reached the end or there's no path starting with '/'
    if (path_start >= buffer + n || *path_start != '/') {
        return -1410021;
    }
    
    // Find the HTTP version indicator (case insensitive) without using strcasestr
    char *http_version = NULL;
    for (size_t i = 0; i < n - (path_start - buffer) - 4; i++) {
        if ((path_start[i] == 'H' || path_start[i] == 'h') &&
            (path_start[i+1] == 'T' || path_start[i+1] == 't') &&
            (path_start[i+2] == 'T' || path_start[i+2] == 't') &&
            (path_start[i+3] == 'P' || path_start[i+3] == 'p') &&
            path_start[i+4] == '/') {
            http_version = path_start + i;
            break;
        }
    }
    
    // If no HTTP version found, the request format is invalid
    if (http_version == NULL) {
        return -1410031;
    }
    
    // Find the start of HTTP version by looking for the last non-space character before "HTTP/"
    char *path_end = http_version;
    while (path_end > path_start && *(path_end - 1) == ' ') {
        path_end--;
        //*path_end = 0 ; // term it with \0
    }
    
    // Find the last '/' in the path
    char *last_slash = NULL;
    for (char *p = path_end - 1; p >= path_start; p--) {
        if (*p == '/') {
            last_slash = p;
            break;
        }
    }
    
    // If no slash found, the path format is invalid
    if (last_slash == NULL) {
        return -1410041;
    }
    
    // Extract the last part of the path (after the last slash)
    char *last_part = last_slash + 1;
    size_t last_part_len = path_end - last_part;
    
    DXhttp_print_debug("last_part(%d)[%.20s]", last_part_len, last_part );

    // Match against patterns in postReqAhead
    for (int i = 0; postReqAhead[i] != NULL; i++) {
        size_t pattern_len = strlen(postReqAhead[i]);
        
        // Check if the last part matches the current pattern
        if (pattern_len <= last_part_len && 
            strncmp(last_part, postReqAhead[i], pattern_len) == 0) {
            //strncpy(postType_str, last_part, pattern_len); postType_str[9] = 0 ;
            postType_0signup_1login_2_admin = i ;
            postType_str = postReqAhead[ i ] ;
            return i;
        }
    }
    
    // No match found
    return -1410051;
}
