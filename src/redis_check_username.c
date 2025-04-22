#include "common.h"

int redis_check_username(const char *username, struct redis_config *conf) {
    if ( NULL == username || NULL == conf ) {
        DXprint_debug(" username/conf NULL met" );
        return 122001 ;
    }

    int rt ; 
    long tmpLong ; 
    rt = redis_get_int(conf, DatabaseIdx_UserName, &tmpLong, "user:%s", username ) ;
    if ( rt ) {
        DXprint_debug(" get username met : rt -> %d" , rt);
        return 122010 ;
    }

    if ( 1 == tmpLong ) {
        DXprint_debug(" get username in database %d , succeed, get '1'" , DatabaseIdx_UserName );
        return 0 ;
    }

    DXprint_debug(" get username in database %d , error, get '%ld'" , DatabaseIdx_UserName, tmpLong );
    return 122030 ;
}
