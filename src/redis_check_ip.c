#include "common.h"

int redis_check_ip(const char *ip, struct redis_config *conf, int dbIdx) {
    DXprint_debug("===== %s: start", __func__ );
    if (!redis_connect_thread(conf, dbIdx)) return 111001;

    long tmpLong ;
    int rt ;

    //rt = redis_get_int( conf, dbIdx, &tmpLong, "EXISTS \"signupOK:%s\"", ip );
    rt = redis_get_int( conf, dbIdx, &tmpLong, "EXISTS signupOK:%s", ip );
    if ( rt != 0 ) { DXprint_debug("Met error: return rt : %lld", rt); return rt; }
    DXprint_debug("EXISTS signupOK:[%s], get %ld", ip, tmpLong );

    if (tmpLong != 0 ) {

        rt = redis_increment( conf, dbIdx, SIGNUP_FAILED_TTL, &tmpLong, "signupFailed:%s:count", ip);
        if ( rt != 0 ) { DXprint_debug("Met error: return rt : %lld", rt); return rt; }
        DXprint_debug("signupOK found, incr faile count to %ld, by [%s]", tmpLong, ip );

        return 111011; // signupOK found. in 3600s don't allow more than once success regiester. please retry 3600s later
    }

    rt = redis_increment( conf, dbIdx, SIGNUP_FAILED_TTL, &tmpLong, "signupFailed:%s:count", ip);
    if ( rt != 0 ) { DXprint_debug("Met error: return rt : %lld", rt); return rt; }
    DXprint_debug("no signupOK found, anyway, incr faile count to %ld, by [%s]", tmpLong, ip );

    if (tmpLong > 5) {
        DXprint_debug("Sent: incr signupFailed:%s:count and set TTL to %d | Received: %lld, exceed 5", ip, SIGNUP_FAILED_TTL, tmpLong );
        return 111019; // counter > 5 
    }

    DXprint_debug("succeed get:%s, faile counter %ld", ip, tmpLong );
    return 0;
}

