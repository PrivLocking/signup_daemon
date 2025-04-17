#include "common.h"
#include <openssl/md5.h>
#include <time.h>

void gen_a_new_md5sum_hex_32byte(char *output) {
    // Ensure output buffer is at least 33 bytes for the 32 hex chars + null terminator
    unsigned char data[64];
    MD5_CTX md5context;
    unsigned char digest[16];
    
    // Generate random data for MD5
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    
    // Mix time values with some random data
    memcpy(data, &ts, sizeof(ts));
    if (RAND_bytes(data + sizeof(ts), sizeof(data) - sizeof(ts)) != 1) {
        // Fallback if RAND_bytes fails
        for (size_t i = sizeof(ts); i < sizeof(data); i++) {
            data[i] = (unsigned char)rand();
        }
    }
    
    // Generate MD5 hash
    MD5_Init(&md5context);
    MD5_Update(&md5context, data, sizeof(data));
    MD5_Final(digest, &md5context);
    
    // Convert to hex string
    for (int i = 0; i < 16; i++) {
        snprintf(output + (i * 2), 3, "%02x", digest[i]); // a-f0-9 only, not A-F
    }
    output[32] = '\0';
}
