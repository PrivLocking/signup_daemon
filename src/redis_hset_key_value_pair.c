#include "common.h"

// rt = redis_hset_key_value_pair( redis_conf, DatabaseIdx_UserName, &tmpLong, NEW_USER_TTL_30d, "HSET user:%s hash %s salt %s level 0 status active", req.username, req.passwd, req.signup_salt);
int redis_hset_key_value_pair(struct redis_config *conf, int databaseIdx, long *updateAmount, int ttl, const char *fmt, ... ) {
    char srcBuf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(srcBuf, 1024, fmt, args);
    va_end(args);

    if (!redis_connect_thread(conf, databaseIdx)) return 136001;

    // Parse the command to extract key name
    char keyName[256] = {0};
    char *ptr = srcBuf;
    char cmd[16] = {0};
    // Skip leading whitespace
    while (*ptr && isspace(*ptr)) ptr++;
    // Get command (first word)
    int i = 0;
    while (*ptr && !isspace(*ptr) && i < sizeof(cmd) - 1) {
        cmd[i++] = *ptr++;
    }
    cmd[i] = '\0';
    // Verify it's HMSET or HSET
    if (strcasecmp(cmd, "HMSET") != 0 && strcasecmp(cmd, "HSET") != 0) {
        DXprint_debug("Redis 136004 Invalid command (not HMSET/HSET): %s", cmd);
        return 136005;
    }
    // Skip whitespace after command
    while (*ptr && isspace(*ptr)) ptr++;
    // Get key name (second word)
    i = 0;
    while (*ptr && !isspace(*ptr) && i < sizeof(keyName) - 1) {
        keyName[i++] = *ptr++;
    }
    keyName[i] = '\0';
    if (i == 0) {
        DXprint_debug("Redis 136006 Failed to parse keyName: %s", srcBuf);
        return 136007;
    }

    redisReply *reply;
    reply = redisCommand(ctx, srcBuf);
    if (!reply) {
        DXprint_debug("Redis 136008 [%s] error: !reply", srcBuf);
        redisFree(ctx); ctx = NULL;
        return 136009;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        DXprint_debug("Redis 136010 ERROR: [%s] -> [%s]", srcBuf, reply->str);
        freeReplyObject(reply);
        return 136011;
    }

    // HMSET returns OK (string), HSET returns number of fields added (integer)
    if (strcasecmp(cmd, "HMSET") == 0) {
        if (reply->type != REDIS_REPLY_STATUS) {
            DXprint_debug("Redis 136015 HMSET failed: reply->type(%d) expected REDIS_REPLY_STATUS: [%s] by [%s]", 
                          reply->type, reply->str, srcBuf);
            freeReplyObject(reply);
            return 136016;
        }
        *updateAmount = 1; // Success for HMSET
    } else { // HSET
        if (reply->type != REDIS_REPLY_INTEGER) {
            DXprint_debug("Redis 136017 HSET failed: reply->type(%d) expected REDIS_REPLY_INTEGER: [%s] by [%s]", 
                          reply->type, reply->str, srcBuf);
            freeReplyObject(reply);
            return 136018;
        }
        *updateAmount = reply->integer;
    }
    freeReplyObject(reply);

    // Only set expiry if ttl > 0
    if (ttl > 0) {
        char expireBuf[1024];
        snprintf(expireBuf, 1024, "EXPIRE %s %d", keyName, ttl);
        reply = redisCommand(ctx, expireBuf);
        if (!reply) {
            DXprint_debug("Redis 136027 [%s] error: !reply", expireBuf);
            redisFree(ctx); ctx = NULL;
            return 136028;
        }
        if (reply->type == REDIS_REPLY_ERROR) {
            DXprint_debug("Redis 136030 ERROR: [%s] -> [%s]", expireBuf, reply->str);
            freeReplyObject(reply);
            return 136031;
        }

        if (reply->type != REDIS_REPLY_INTEGER) {
            DXprint_debug("Redis 136045 EXPIRE failed: reply->type(%d): [%s] by [%s]", 
                          reply->type, reply->str, expireBuf);
            freeReplyObject(reply);
            return 136046;
        }
        
        if (reply->integer != 1) {
            DXprint_debug("Redis 136055 EXPIRE failed: reply->integer(%ld): key did not exist or TTL not set", 
                          reply->integer);
            freeReplyObject(reply);
            return 136056;
        }
        freeReplyObject(reply);
        
        DXprint_debug("Redis 136065 HSET and EXPIRE succeed: [%s] [%s]", srcBuf, expireBuf);
    } else {
        DXprint_debug("Redis 136066 HSET succeed (no TTL): [%s]", srcBuf);
    }
    
    return 0; // Success
}
