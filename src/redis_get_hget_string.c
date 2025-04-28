#include "common.h"

// Usage examples:
// rt = redis_get_hget_string(redis_conf, 5, 32, dbSavedSignUpSalt, "GET signup_sess:%s", req.signup_salt);
// rt = redis_get_hget_string(redis_conf, 5, 32, dbSavedUserData, "HGET user_data:%s email", req.user_id);
int redis_get_hget_string(struct redis_config *conf, int databaseIdx, int dstLen, char *dstBuf, const char *fmt, ... ) {
    char cmdBuf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(cmdBuf, 1024, fmt, args);
    va_end(args);

    if (!redis_connect_thread(conf, databaseIdx)) return 125001;

    redisReply *reply;
    reply = redisCommand(ctx, cmdBuf);
    if (!reply) {
        DXprint_debug("Redis 125005 Command [%s] error: !reply", cmdBuf);
        redisFree(ctx); ctx = NULL;
        return 125006;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        DXprint_debug("Redis 125010 ERROR: [%s] -> [%s]", cmdBuf, reply->str);
        freeReplyObject(reply);
        return 125011;
    }

    if (reply->type != REDIS_REPLY_STRING) {
        DXprint_debug("Redis 125020 Command error: reply->type(%d) != REDIS_REPLY_STRING : [%s]: REDIS_TYPE", 
                     reply->type, cmdBuf);
        freeReplyObject(reply);
        return 125021;
    }
    if (!reply->str) {
        DXprint_debug("Redis 125030 Command error: reply->str == null : [%s]", cmdBuf);
        freeReplyObject(reply);
        return 125031;
    }

    DXprint_debug("Sent: %s | Received: %s", cmdBuf, reply->str ? reply->str : "null, ERROR ?");

    // Check the string length first
    size_t copy_len = strlen(reply->str);
    if (copy_len != dstLen) {
        DXprint_debug("%s : 125040 str copy [%s] failed. Required length %d but got %zu", 
                    __func__, cmdBuf, dstLen, copy_len);
        freeReplyObject(reply);
        return 125041;
    }
    
    // Now safely copy the string and null-terminate
    strncpy(dstBuf, reply->str, dstLen);
    dstBuf[dstLen] = 0;
    
    freeReplyObject(reply);
    return 0;
}
