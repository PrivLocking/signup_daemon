#include "common.h"

void gen_a_new_md5sum_hex_32byte(char *output) {
    static EVP_MD_CTX *mdctx = NULL;
    static bool initialized = false;
    static unsigned char static_seed[32]; // binary seed, not null-terminated

    if (!initialized) {
        // Initialize seed with secure random bytes instead of zeros
        if (RAND_bytes(static_seed, sizeof(static_seed)) != 1) {
            fprintf(stderr, "Failed to generate secure random bytes for seed\n");
            // Fallback to a less secure initialization if RAND_bytes fails
            time_t fallback_seed = time(NULL);
            memcpy(static_seed, &fallback_seed, sizeof(time_t));
            for (size_t i = sizeof(time_t); i < sizeof(static_seed); i++) {
                static_seed[i] = rand() & 0xFF;
            }
        }

        mdctx = EVP_MD_CTX_new();
        if (!mdctx) {
            fprintf(stderr, "Failed to create EVP_MD_CTX\n");
            output[0] = '\0';
            return;
        }
        initialized = true;
        atexit((void (*)(void))EVP_MD_CTX_free); // auto free at exit
    }

    // Compose binary input: seed (32) + time (8) + rand (4) + additional random bytes (16) = 60 bytes
    unsigned char input_buf[32 + sizeof(time_t) + sizeof(int) + 16];
    memcpy(input_buf, static_seed, 32);

    time_t now = time(NULL);
    memcpy(input_buf + 32, &now, sizeof(time_t));

    int r = rand();
    memcpy(input_buf + 32 + sizeof(time_t), &r, sizeof(int));

    // Add additional random bytes for enhanced security
    unsigned char extra_random[16];
    if (RAND_bytes(extra_random, sizeof(extra_random)) != 1) {
        // If RAND_bytes fails, use less secure fallback
        for (size_t i = 0; i < sizeof(extra_random); i++) {
            extra_random[i] = rand() & 0xFF;
        }
    }
    memcpy(input_buf + 32 + sizeof(time_t) + sizeof(int), extra_random, 16);

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;

    // Try to compute the digest, with auto-reinit on failure
    int attempt = 0;
    int max_attempts = 3;
    bool success = false;
    
    while (!success && attempt < max_attempts) {
        if (EVP_DigestInit_ex(mdctx, EVP_md5(), NULL) == 1 &&
            EVP_DigestUpdate(mdctx, input_buf, sizeof(input_buf)) == 1 &&
            EVP_DigestFinal_ex(mdctx, digest, &digest_len) == 1) {
            success = true;
        } else {
            // Re-init logic on failure
            attempt++;
            if (attempt < max_attempts) {
                EVP_MD_CTX_reset(mdctx);
                fprintf(stderr, "MD5 computation failed, retrying (attempt %d)...\n", attempt);
            }
        }
    }

    if (!success) {
        fprintf(stderr, "EVP MD5 computation failed after %d attempts\n", max_attempts);
        output[0] = '\0';
        return;
    }

    // Update the static seed with MD5 result (16 bytes)
    // Note: MD5 produces 16 bytes, not 32
    memcpy(static_seed, digest, 16);
    // Fill remaining 16 bytes with additional randomness
    if (RAND_bytes(static_seed + 16, 16) != 1) {
        // If RAND_bytes fails, copy the first 16 bytes as fallback
        memcpy(static_seed + 16, digest, 16);
    }

    // Format the MD5 hash as hex (16 bytes = 32 hex chars)
    for (unsigned int i = 0; i < 16; i++) {
        snprintf(output + (i * 2), 3, "%02x", digest[i]);
    }
    output[32] = '\0';
}
