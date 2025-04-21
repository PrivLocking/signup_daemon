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
    reply = redisCommand(ctx, "GET %s", srcBuf);
    if (!reply) {
        DBprint_debug("Redis 129007 GET [%s] error: !reply" , srcBuf);
        redisFree(ctx); ctx = NULL ;
        return 129008;
    }
    if (reply->type != REDIS_REPLY_STRING) {
        DBprint_debug("Redis 129010 GET int error: reply->type(%d) != REDIS_REPLY_STRING : [GET %s]: " REDIS_TYPE , reply->type, srcBuf );
        freeReplyObject(reply);
        return 129011;
    }

    if (!reply->str) {
        DBprint_debug("Redis GET error: reply->str == null : [GET %s]", srcBuf );
        freeReplyObject(reply);
        return 129013;
    }

    char *endptr;
    errno = 0;  // Reset errno before calling strtol , thread safe
    long value = strtol(reply->str, &endptr, 10);  // Base 10
    // Check for conversion errors
    if (errno != 0 || *endptr != '\0' || reply->str == endptr) {
        DBprint_debug("Error: 129014 Redis received a valid integer!\n");
        freeReplyObject(reply);
        return 129015 ;
    }

    *dstInt = value ;
    DBprint_debug("Sent: 129019 GET %s | Received(long int): %ld", srcBuf, *dstInt );

    return 0;
}

