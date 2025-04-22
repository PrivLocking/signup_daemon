#include "common.h"

// rt = redis_get_string(redis_conf, 5, 32, dbSavedSignUpSalt, "signup_sess:%s", req.signup_salt ) ;
int redis_get_string(struct redis_config *conf, int databaseIdx, int dstLen, char *dstBuf, const char *fmt, ... ) {
    char srcBuf[1024] ;
    va_list args;
    va_start(args, fmt);
    vsnprintf(srcBuf,1024, fmt, args);
    va_end(args);


    if (!redis_connect_thread(conf, databaseIdx)) return 125001;

    redisReply *reply ;
    reply = redisCommand(ctx, "GET %s", srcBuf);
    if (!reply) {
        DBprint_debug("Redis 125005 GET [%s] error: !reply" , srcBuf);
        redisFree(ctx); ctx = NULL ;
        return 125006;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        DXprint_debug("Redis 125010 ERROR: [%s] -> [%s]", srcBuf, reply->str);  // ← THIS WILL SHOW THE ACTUAL ERROR
        freeReplyObject(reply);
        return 125011;
    }

    if (reply->type != REDIS_REPLY_STRING) {
        DBprint_debug("Redis 129020 GET error: reply->type(%d) != REDIS_REPLY_STRING : [GET %s]: " REDIS_TYPE , reply->type, srcBuf );
        freeReplyObject(reply);
        return 125021;
    }
    if (!reply->str) {
        DBprint_debug("Redis 129030 GET error: reply->str == null : [GET %s]", srcBuf );
        freeReplyObject(reply);
        return 125031;
    }

    DBprint_debug("Sent: GET %s | Received: %s", srcBuf, reply && reply->str ? reply->str : "null, ERROR ?");

    strncpy( dstBuf, reply->str, dstLen ) ;
    dstBuf[dstLen] = 0 ;
    int actual_len = strlen( dstBuf ) ;
    if ( dstLen != actual_len ) {
        DBprint_debug("%s : 129040 str copy [%s] failed. %d vs %d ", __func__, srcBuf, dstLen, actual_len);
        freeReplyObject(reply);
        return 125041;
    }
    freeReplyObject(reply);
    return 0;
}

