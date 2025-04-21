#include "common.h"

int redis_check_ip(const char *ip, struct redis_config *conf) {
    DBprint_debug("===== %s: start", __func__ );
    if (!redis_connect_thread(conf, 5)) return 111001;

    redisReply *reply ;
    reply = redisCommand(ctx, "EXISTS signupOK:%s", ip);
    if (conf->debug_mode) {
        DBprint_debug("Sent: EXISTS signupOK:%s | Received: %lld", ip, reply ? reply->integer : 0);
    }
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        DBprint_debug("Redis EXISTS signupOK:%s failed", ip);
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return 111009;
    }
    bool exists = reply->integer == 1;
    freeReplyObject(reply);
    if (exists) {
        reply = redisCommand(ctx, "INCR signupFailed:%s:count", ip);
        if (conf->debug_mode) {
            DBprint_debug("Sent: INCR signupFailed:%s:count | Received: %lld", ip, reply ? reply->integer : 0);
        }
        if (reply && reply->type == REDIS_REPLY_INTEGER) {
            reply = redisCommand(ctx, "EXPIRE signupFailed:%s:count %d", ip, SIGNUP_FAILED_TTL);
            if (conf->debug_mode) {
                DBprint_debug("Sent: EXPIRE signupFailed:%s:count %d | Received: %lld", ip, SIGNUP_FAILED_TTL, reply ? reply->integer : 0);
            }
            if (!reply || reply->type != REDIS_REPLY_INTEGER) {
                DBprint_debug("Redis EXPIRE signupFailed:%s:count failed", ip);
            }
            if (reply) freeReplyObject(reply);
        } else {
            DBprint_debug("Redis INCR signupFailed:%s:count failed", ip);
            if (reply) freeReplyObject(reply);
        }
        redisFree(ctx);
        return 111011; // signupOK found. in 3600s don't allow more than once success regiester. please retry 3600s later
    }

    reply = redisCommand(ctx, "GET signupFailed:%s:count", ip);
    if (conf->debug_mode) {
        DBprint_debug("checking the failed count ============ Sent: GET signupFailed:%s:count | Received: %s", ip, reply && reply->str ? reply->str : "null");
    }
    int count = 0;
    if (reply && reply->type == REDIS_REPLY_STRING) {
        count = atoi(reply->str);
    } else if (!reply || reply->type == REDIS_REPLY_ERROR) {
        DBprint_debug("Redis GET signupFailed:%s:count failed", ip);
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return 111013;
    }
    freeReplyObject(reply);

    if (count >= 5) {
        reply = redisCommand(ctx, "INCR signupFailed:%s:count", ip);
        if (conf->debug_mode) {
            DBprint_debug("Sent: INCR signupFailed:%s:count | Received: %lld", ip, reply ? reply->integer : 0);
        }
        if (!reply || reply->type != REDIS_REPLY_INTEGER) {
            DBprint_debug("Redis INCR signupFailed:%s:count failed", ip);
            if (reply) freeReplyObject(reply);
            redisFree(ctx);
            return 111015;
        }
        freeReplyObject(reply);

        reply = redisCommand(ctx, "EXPIRE signupFailed:%s:count %d", ip, SIGNUP_FAILED_TTL);
        if (conf->debug_mode) {
            DBprint_debug("Sent: EXPIRE signupFailed:%s:count %d | Received: %lld", ip, SIGNUP_FAILED_TTL, reply ? reply->integer : 0);
        }
        if (!reply || reply->type != REDIS_REPLY_INTEGER) {
            DBprint_debug("Redis EXPIRE signupFailed:%s:count failed", ip);
            if (reply) freeReplyObject(reply);
            redisFree(ctx);
            return 111017;
        }
        freeReplyObject(reply);

        redisFree(ctx);
        return 111019; // counter > 5 
    }

    redisFree(ctx);
    return 0;
}

