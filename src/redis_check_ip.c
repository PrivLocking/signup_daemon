#include "common.h"

int redis_check_ip(const char *ip, struct redis_config *conf, int dbIdx) {
    DXprint_debug("===== %s: start", __func__ );
    if (!redis_connect_thread(conf, dbIdx)) return 111001;

    long tmpLong ;
    int rt ;

    //rt = redis_get_int( conf, dbIdx, &tmpLong, "EXISTS signupOK:%s", ip );
    rt = redis_get_int( conf, dbIdx, &tmpLong, "EXISTS %sOK:%s", postType_str, ip );
    if ( rt != 0 ) { DXprint_debug("Met error: return rt : %lld", rt); return rt; }
    DXprint_debug("EXISTS %sOK:[%s], get %ld", postType_str, ip, tmpLong );

    if (tmpLong != 0) {
        if ( 
                (0 == postType_0signup_1login_2_admin )                    // any signup, allow only 1 per hour
                || (1 == postType_0signup_1login_2_admin && tmpLong > 6 )  // normal login, max allow 6 client at the same time
                || (2 == postType_0signup_1login_2_admin && tmpLong > 2 )  // admin  login, max allow 6 client at the same time
           ) {

            rt = redis_increment( conf, dbIdx, SIGNUP_FAILED_TTL, &tmpLong, "%sFailed:%s:count", postType_str, ip);
            if ( rt != 0 ) { DXprint_debug("Met error: return rt : %lld", rt); return rt; }
            DXprint_debug("%sOK found, incr faile count to %ld, by [%s]", postType_str, tmpLong, ip );

            return 111011; // signupOK found. in 3600s don't allow more than once success regiester. please retry 3600s later
        }
    }

    rt = redis_increment( conf, dbIdx, SIGNUP_FAILED_TTL, &tmpLong, "%sFailed:%s:count", postType_str, ip);
    if ( rt != 0 ) { DXprint_debug("Met error: return rt : %lld", rt); return rt; }
    DXprint_debug("no %sOK found, anyway, incr faile count to %ld, by [%s]", postType_str, tmpLong, ip );

    int maxAllowFailed = 5;
    if ( 1 == postType_0signup_1login_2_admin ) maxAllowFailed = 10;
    if ( 2 == postType_0signup_1login_2_admin ) maxAllowFailed = 2;
    if (tmpLong > maxAllowFailed) {
        DXprint_debug("Sent: incr %sFailed:%s:count and set TTL to %d | Received: %lld, exceed %d", postType_str, ip, SIGNUP_FAILED_TTL, tmpLong, maxAllowFailed );
        return 111019; // counter > 5 
    }

    DXprint_debug("succeed get:%s, faile counter %ld", ip, tmpLong );
    return 0;
}

