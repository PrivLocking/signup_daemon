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
        DBprint_debug("Redis GET [%s] error: !reply" , srcBuf);
        redisFree(ctx); ctx = NULL ;
        return 125008;
    }
    if (reply->type != REDIS_REPLY_STRING) {
        DBprint_debug("Redis GET error: reply->type(%d) != REDIS_REPLY_STRING : [GET %s]: " REDIS_TYPE , reply->type, srcBuf );
        freeReplyObject(reply);
        return 125010;
    }
    if (!reply->str) {
        DBprint_debug("Redis GET error: reply->str == null : [GET %s]", srcBuf );
        freeReplyObject(reply);
        return 125011;
    }

    DBprint_debug("Sent: GET %s | Received: %s", srcBuf, reply && reply->str ? reply->str : "null, ERROR ?");

    strncpy( dstBuf, reply->str, dstLen ) ;
    dstBuf[dstLen] = 0 ;
    int actual_len = strlen( dstBuf ) ;
    if ( dstLen != actual_len ) {
        DBprint_debug("%s : str copy [%s] failed. %d vs %d ", __func__, srcBuf, dstLen, actual_len);
        freeReplyObject(reply);
        return 125021;
    }
    freeReplyObject(reply);
    return 0;
}

