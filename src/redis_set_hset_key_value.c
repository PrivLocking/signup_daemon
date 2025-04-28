#include "common.h"

// Usage examples:
// rt = redis_set_hset_key_value(redis_conf, 5, 300, "SET signup_sess:%s %s", signup_sesv, signup_sess);
// rt = redis_set_hset_key_value(redis_conf, 5, 300, "HSET user:%s email %s", user_id, user_email);
int redis_set_hset_key_value(struct redis_config *conf, int databaseIdx, int TTL, const char *fmt, ... ) {
    char cmdBuf[1024];
    va_list args;
    
    // First, format the command string
    va_start(args, fmt);
    vsnprintf(cmdBuf, 1024, fmt, args);
    va_end(args);
    
    // Check if it's a SET command (where we can append EX directly)
    // or HSET/HMSET (where we need a separate EXPIRE)
    bool isHsetCommand = true;
    
    if (strncmp(cmdBuf, "SET ", 4) == 0) {
        isHsetCommand = false;
        
        // If TTL is specified for SET, append it to the command
        if (TTL > 0) {
            size_t cmdLen = strlen(cmdBuf);
            snprintf(cmdBuf + cmdLen, 1024 - cmdLen, " EX %d", TTL);
        }
    } 
    
    // Connect to Redis
    if (!redis_connect_thread(conf, databaseIdx)) return 135001;
    
    // Execute the main command
    redisReply *reply;
    reply = redisCommand(ctx, cmdBuf);
    if (!reply) {
        DXprint_debug("Redis 135007 (%d)[%s] error: !reply", databaseIdx, cmdBuf);
        redisFree(ctx); ctx = NULL;
        return 135008;
    }
    
    if (reply->type == REDIS_REPLY_ERROR) {
        DXprint_debug("Redis 135010 ERROR: (%d)[%s] -> [%s]", databaseIdx, cmdBuf, reply->str);
        freeReplyObject(reply);
        return 135011;
    }
    
    // Check if the main command succeeded
    if (reply->type != REDIS_REPLY_STATUS && reply->type != REDIS_REPLY_INTEGER) {
        DXprint_debug("Redis 135025 Command error: reply->type(%d) unexpected response for (%d)[%s]", 
                    reply->type, databaseIdx, cmdBuf);
        freeReplyObject(reply);
        return 135026;
    }
    
    // Main command succeeded - log it
    if (reply->type == REDIS_REPLY_STATUS) {
        DXprint_debug("Redis 135015 Command succeeded: reply->type(%d) : got [%s] for (%d)[%s]", 
                    reply->type, reply->str, databaseIdx, cmdBuf);
    } else {
        DXprint_debug("Redis 135016 Command succeeded: reply->type(%d) : got [%ld] for (%d)[%s]", 
                    reply->type, reply->integer, databaseIdx, cmdBuf);
    }
    
    // Early return if we don't need to set TTL separately
    if (!isHsetCommand || TTL <= 0) {
        freeReplyObject(reply);
        return 0;
    }
    
    // At this point, we know it's a hash command with TTL > 0
    // Now extract the key for EXPIRE
    char keyBuf[512] = {0};
    int prefixLen = (strncmp(cmdBuf, "HSET ", 5) == 0) ? 5 : 6;
    char *keyStart = cmdBuf + prefixLen;
    char *keyEnd = strchr(keyStart, ' ');
    
    if (!keyEnd) {
        DXprint_debug("Redis 135020 Cannot extract key for EXPIRE from (%d)[%s]", databaseIdx, cmdBuf);
        freeReplyObject(reply);
        return 135027;
    }
    
    size_t keyLen = keyEnd - keyStart;
    if (keyLen >= sizeof(keyBuf)) {
        DXprint_debug("Redis 135021 Key too long for EXPIRE from (%d)[%s]", databaseIdx, cmdBuf);
        freeReplyObject(reply);
        return 135028;
    }
    
    strncpy(keyBuf, keyStart, keyLen);
    keyBuf[keyLen] = '\0';
    
    // Free the previous reply before executing next command
    freeReplyObject(reply);
    
    // Execute the EXPIRE command
    reply = redisCommand(ctx, "EXPIRE %s %d", keyBuf, TTL);
    if (!reply) {
        DXprint_debug("Redis 135012 EXPIRE [%s %d] error: !reply", keyBuf, TTL);
        redisFree(ctx); ctx = NULL;
        return 135013;
    }
    
    if (reply->type == REDIS_REPLY_ERROR) {
        DXprint_debug("Redis 135014 EXPIRE ERROR: (%d)[%s %d] -> [%s]", databaseIdx, keyBuf, TTL, reply->str);
        freeReplyObject(reply);
        return 135015;
    }
    
    if (reply->type != REDIS_REPLY_INTEGER || reply->integer != 1) {
        DXprint_debug("Redis 135017 EXPIRE failed: reply->type(%d) : got [%ld] for (%d)EXPIRE %s %d", 
                    reply->type, reply->integer, databaseIdx, keyBuf, TTL);
        freeReplyObject(reply);
        return 135018;
    }
    
    DXprint_debug("Redis 135019 EXPIRE succeeded for key (%d)[%s] with TTL %d", databaseIdx, keyBuf, TTL);
    
    // Clean up and return success
    freeReplyObject(reply);
    return 0;
}
