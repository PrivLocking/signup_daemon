#include "common.h"

// Usage examples:
// rt = redis_get_int(redis_conf, 5, &dstInt, "GET counter:%s", user_id);
// rt = redis_get_int(redis_conf, 5, &dstInt, "HGET user_stats:%s score", user_id);
// rt = redis_get_int(redis_conf, 5, &dstInt, "EXISTS user_stats:%s score", user_id);
// rt = redis_get_int(redis_conf, 5, &dstInt, "EXPIRE user_stats:%s %d", user_id, 300 );
int redis_get_int(struct redis_config *conf, int databaseIdx, long *dstInt, const char *fmt, ... ) {
    char cmdBuf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(cmdBuf, 1024, fmt, args);
    va_end(args);

    if (!redis_connect_thread(conf, databaseIdx)) return 129001;

    redisReply *reply;
    reply = redisCommand(ctx, cmdBuf);
    if (!reply) {
        DXprint_debug("Redis 129007 (%d)[%s] error: !reply", databaseIdx, cmdBuf);
        redisFree(ctx); ctx = NULL;
        return 129008;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        DXprint_debug("Redis 129010 ERROR: (%d)[%s] -> [%s]", databaseIdx, cmdBuf, reply->str);
        freeReplyObject(reply);
        return 129011;
    }

    // Handle direct integer reply
    if (reply->type == REDIS_REPLY_INTEGER) {
        *dstInt = reply->integer;
        DXprint_debug("Redis 129015 Command int succeed: reply->type(%d) : get [%ld] by (%d)[%s]: REDIS_TYPE", 
                    reply->type, *dstInt, databaseIdx, cmdBuf);
        freeReplyObject(reply);
        return 0;
    }

    // Handle string replies that need conversion to int
    if (reply->type == REDIS_REPLY_STRING) {
        if (!reply->str) {
            DXprint_debug("Redis 129025 Command int error: reply->str == null : (%d)[%s]", databaseIdx, cmdBuf);
            freeReplyObject(reply);
            return 129026;
        }

        char *endptr;
        errno = 0;  // Reset errno before calling strtol, thread safe
        long value = strtol(reply->str, &endptr, 10);  // Base 10
        
        // Check for conversion errors
        if (errno != 0 || *endptr != '\0' || reply->str == endptr) {
            DXprint_debug("Error: 129035 Redis reply cannot be converted to a valid integer! (%d)[%s]", databaseIdx, cmdBuf);
            freeReplyObject(reply);
            return 129036;
        }

        *dstInt = value;
        DXprint_debug("Sent: 129045 Command int: (%d)[%s] | Received(long int): %ld", databaseIdx, cmdBuf, *dstInt);
        
        freeReplyObject(reply);
        return 0;
    }

    // Handle case where reply is neither an integer nor a string
    DXprint_debug("Redis 129050 Command error: reply->type(%d) not INTEGER or STRING : (%d)[%s]", 
                reply->type, databaseIdx, cmdBuf);
    freeReplyObject(reply);
    return 129051;
}
