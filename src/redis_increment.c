#include "common.h"

// redis_increment(redis_conf, 5, SIGNUP_FAILED_TTL, &tmpLong, "signupFailed:%s:count", ip );
int redis_increment( struct redis_config *conf, int dbIdx, int TTL, long *dstLong, const char *fmt, ...) {
    char srcBuf[1024] ;
    va_list args;
    va_start(args, fmt);
    vsnprintf(srcBuf,1024, fmt, args);
    va_end(args);

    if (!redis_connect_thread(conf, dbIdx)) return 131001;
    int rt ;
    long tmpLong ;

    rt = redis_get_int(conf, dbIdx, &tmpLong, "INCR %s", srcBuf );
    if ( rt ) {
        DXprint_debug("131015, [INCR %s] error, get : %ld" , srcBuf, rt);
        return rt;
    }

    *dstLong = tmpLong ;
    //DXprint_debug("131016, [INCR/EXPIRE %s TTL %d] succeed, get : %ld" , srcBuf, TTL, tmpLong );

    rt = redis_get_int(conf, dbIdx, &tmpLong, "EXPIRE %s %d", srcBuf, TTL);
    if ( rt ) {
        DXprint_debug("131020, [EXPIRE %s %d] error, get : %ld" , srcBuf, TTL, rt);
        return rt;
    }

    DXprint_debug("131045, [INCR/EXPIRE %s TTL %d] succeed, get : %ld" , srcBuf, TTL, *dstLong );
    return 0;
}

