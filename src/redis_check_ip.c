#include "common.h"

extern bool redis_connect_thread(struct redis_config *conf);

int redis_check_ip(const char *ip, struct redis_config *conf) {
    if (!redis_connect_thread(conf)) return 111001;

    redisContext *ctx = NULL;
    struct timeval timeout = { REDIS_TIMEOUT_SEC, 0 };
    ctx = redisConnectUnixWithTimeout(conf->path, timeout);
    if (!ctx || ctx->err) {
        print_debug("Redis connect failed on %s: %s", conf->path, ctx ? ctx->errstr : "null context");
        if (ctx) redisFree(ctx);
        return 111003;
    }

    redisReply *reply = redisCommand(ctx, "AUTH %s", conf->password);
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis AUTH failed: %s", reply ? reply->str : "null reply");
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return 111005;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "SELECT 5");
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SELECT 5 failed: %s", reply ? reply->str : "null reply");
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return 111007;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "EXISTS signupOK:%s", ip);
    if (conf->debug_mode) {
        print_debug("Sent: EXISTS signupOK:%s | Received: %lld", ip, reply ? reply->integer : 0);
    }
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        print_debug("Redis EXISTS signupOK:%s failed", ip);
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return 111009;
    }
    bool exists = reply->integer == 1;
    freeReplyObject(reply);
    if (exists) {
        reply = redisCommand(ctx, "INCR signupFailed:%s:count", ip);
        if (conf->debug_mode) {
            print_debug("Sent: INCR signupFailed:%s:count | Received: %lld", ip, reply ? reply->integer : 0);
        }
        if (reply && reply->type == REDIS_REPLY_INTEGER) {
            reply = redisCommand(ctx, "EXPIRE signupFailed:%s:count %d", ip, SIGNUP_FAILED_TTL);
            if (conf->debug_mode) {
                print_debug("Sent: EXPIRE signupFailed:%s:count %d | Received: %lld", ip, SIGNUP_FAILED_TTL, reply ? reply->integer : 0);
            }
            if (!reply || reply->type != REDIS_REPLY_INTEGER) {
                print_debug("Redis EXPIRE signupFailed:%s:count failed", ip);
            }
            if (reply) freeReplyObject(reply);
        } else {
            print_debug("Redis INCR signupFailed:%s:count failed", ip);
            if (reply) freeReplyObject(reply);
        }
        redisFree(ctx);
        return 111011; // signupOK exist. please retry 3600s later
    }

    reply = redisCommand(ctx, "GET signupFailed:%s:count", ip);
    if (conf->debug_mode) {
        print_debug("Sent: GET signupFailed:%s:count | Received: %s", ip, reply && reply->str ? reply->str : "null");
    }
    int count = 0;
    if (reply && reply->type == REDIS_REPLY_STRING) {
        count = atoi(reply->str);
    } else if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis GET signupFailed:%s:count failed", ip);
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return 111013;
    }
    freeReplyObject(reply);

    if (count >= 5) {
        reply = redisCommand(ctx, "INCR signupFailed:%s:count", ip);
        if (conf->debug_mode) {
            print_debug("Sent: INCR signupFailed:%s:count | Received: %lld", ip, reply ? reply->integer : 0);
        }
        if (!reply || reply->type != REDIS_REPLY_INTEGER) {
            print_debug("Redis INCR signupFailed:%s:count failed", ip);
            if (reply) freeReplyObject(reply);
            redisFree(ctx);
            return 111015;
        }
        freeReplyObject(reply);

        reply = redisCommand(ctx, "EXPIRE signupFailed:%s:count %d", ip, SIGNUP_FAILED_TTL);
        if (conf->debug_mode) {
            print_debug("Sent: EXPIRE signupFailed:%s:count %d | Received: %lld", ip, SIGNUP_FAILED_TTL, reply ? reply->integer : 0);
        }
        if (!reply || reply->type != REDIS_REPLY_INTEGER) {
            print_debug("Redis EXPIRE signupFailed:%s:count failed", ip);
            if (reply) freeReplyObject(reply);
            redisFree(ctx);
            return 111017;
        }
        freeReplyObject(reply);

        redisFree(ctx);
        return 111019;
    }

    redisFree(ctx);
    return 0;
}

