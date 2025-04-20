#include "common.h"
/*
#include "md5_binary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
*/

// Internal helper function
static int calculate_file_md5(const char *filename, unsigned char *md5_digest) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return 0;
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        fclose(file);
        fprintf(stderr, "Failed to create MD5 context\n");
        return 0;
    }

    if (EVP_DigestInit_ex(mdctx, EVP_md5(), NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        fprintf(stderr, "Failed to initialize MD5 digest\n");
        return 0;
    }

    unsigned char buffer[4096];
    size_t bytes;
    
    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) != 0) {
        if (EVP_DigestUpdate(mdctx, buffer, bytes) != 1) {
            EVP_MD_CTX_free(mdctx);
            fclose(file);
            fprintf(stderr, "Failed to update digest\n");
            return 0;
        }
    }

    unsigned int md5_len;
    if (EVP_DigestFinal_ex(mdctx, md5_digest, &md5_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        fprintf(stderr, "Failed to finalize digest\n");
        return 0;
    }

    EVP_MD_CTX_free(mdctx);
    fclose(file);
    return 1;
}

int get_executable_md5(unsigned char *md5_digest) {
    // Get the path to the current executable
    char path[1024];
    
    // Different methods to get the executable path depending on the OS
    #if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
        // Linux, BSD
        if (readlink("/proc/self/exe", path, sizeof(path) - 1) == -1) {
            fprintf(stderr, "Failed to get executable path\n");
            return 0;
        }
    #elif defined(__APPLE__)
        // macOS
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) != 0) {
            fprintf(stderr, "Failed to get executable path\n");
            return 0;
        }
    #elif defined(_WIN32)
        // Windows
        if (GetModuleFileName(NULL, path, sizeof(path)) == 0) {
            fprintf(stderr, "Failed to get executable path\n");
            return 0;
        }
    #else
        // Fallback method using argv[0]
        // Note: This requires the main program to pass argv[0] to this function
        // which we're not doing in this implementation
        fprintf(stderr, "Unsupported platform for getting executable path\n");
        return 0;
    #endif
    
    // Calculate MD5 of the executable
    return calculate_file_md5(path, md5_digest);
}

char execBinaryMd5[33] = "errorExecBinaryMD5";
char *get_executable_md5_hex(void) {
    static unsigned char md5_digest[EVP_MAX_MD_SIZE];
    char * p = execBinaryMd5 ;
    if (get_executable_md5(md5_digest)) {
        for (int i = 0; i < 16; i++) {
            sprintf(p, "%02x", md5_digest[i]);
            p += 2 ;
        }
        execBinaryMd5[32] = 0 ;
    }
    DBprint_debug("Executable file's md5 is [%s]\n", execBinaryMd5 );
    return execBinaryMd5;
}
