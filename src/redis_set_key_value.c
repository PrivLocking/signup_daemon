#include "common.h"

// rt = redis_set_key_value(redis_conf, DatabaseIdx_SignUp, "SET signup_sess:%s %s EX %d", signup_sesv, signup_sess, 300);
int redis_set_key_value(struct redis_config *conf, int databaseIdx, const char *fmt, ... ) {
    char srcBuf[1024] ;
    va_list args;
    va_start(args, fmt);
    vsnprintf(srcBuf,1024, fmt, args);
    va_end(args);

    if (!redis_connect_thread(conf, databaseIdx)) return 135001;

    redisReply *reply ;
    reply = redisCommand(ctx, srcBuf);
    if (!reply) {
        DXprint_debug("Redis 135007 [%s] error: !reply" , srcBuf);
        redisFree(ctx); ctx = NULL ;
        return 135008;
    }
    if (reply->type == REDIS_REPLY_ERROR) {
        DXprint_debug("Redis 135010 ERROR: [%s] -> [%s]", srcBuf, reply->str);  // ← THIS WILL SHOW THE ACTUAL ERROR
        freeReplyObject(reply);
        return 135011;
    }

    if (reply->type == REDIS_REPLY_STATUS) {
        DXprint_debug("Redis 135015 SET succeed: reply->type(%d) : get [%s] by [%s]: " REDIS_TYPE , reply->type, reply->str, srcBuf );
        freeReplyObject(reply);
        return 0;
    }

    DXprint_debug("Redis 135025 SET error: reply->type(%d) : get [%s] by [%s]: " REDIS_TYPE , reply->type, reply->str, srcBuf );
    freeReplyObject(reply);
    return 135026;
}

