#include "common.h"

// rt = redis_get_int(redis_conf, 5, &dstInt, "signup_sess:%s", req.signup_salt ) ;
int redis_get_int(struct redis_config *conf, int databaseIdx, long *dstInt, const char *fmt, ... ) {
    char srcBuf[1024] ;
    va_list args;
    va_start(args, fmt);
    vsnprintf(srcBuf,1024, fmt, args);
    va_end(args);

    if (!redis_connect_thread(conf, databaseIdx)) return 129001;

    redisReply *reply ;
    reply = redisCommand(ctx, srcBuf);
    if (!reply) {
        DXprint_debug("Redis 129007 [%s] error: !reply" , srcBuf);
        redisFree(ctx); ctx = NULL ;
        return 129008;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        DXprint_debug("Redis 129010 ERROR: [%s] -> [%s]", srcBuf, reply->str);  // ← THIS WILL SHOW THE ACTUAL ERROR
        freeReplyObject(reply);
        return 129011;
    }

    if (reply->type == REDIS_REPLY_INTEGER) {
        *dstInt = reply-> integer ;
        DXprint_debug("Redis 129015 GET int succeed: reply->type(%d) : get [%ld] by [%s]: " REDIS_TYPE , reply->type, *dstInt, srcBuf );
        freeReplyObject(reply);
        return 0;
    }

    if (!reply->str) {
        DXprint_debug("Redis 129025 GET int error: reply->str == null : [%s]", srcBuf );
        freeReplyObject(reply);
        return 129026;
    }

    char *endptr;
    errno = 0;  // Reset errno before calling strtol , thread safe
    long value = strtol(reply->str, &endptr, 10);  // Base 10
    // Check for conversion errors
    if (errno != 0 || *endptr != '\0' || reply->str == endptr) {
        DXprint_debug("Error: 129035 Redis received a valid integer!\n");
        freeReplyObject(reply);
        return 129036 ;
    }

    *dstInt = value ;
    DXprint_debug("Sent: 129045 GET int: [%s] | Received(long int): %ld", srcBuf, *dstInt );

    freeReplyObject(reply);
    return 0;
}

