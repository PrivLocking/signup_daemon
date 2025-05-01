#include "common.h"

#define inputBufLen 256
int calc_hash_for_login( char *username, char *login_sess, char *storage_hash, char *trying_hash){

    int  rt ;
    char argon_input_for_login[inputBufLen];

    if ( !username || !login_sess || !storage_hash || !trying_hash ) {
        DXhttp_print_debug("step4: 242001 : NULL met" );
        return 242001;
    }

    //DXhttp_print_debug("step4: 001" );
    argon_input_for_login[ snprintf( argon_input_for_login, inputBufLen-1, "%.32s:%s:%.64s", login_sess, username, storage_hash ) ] = 0 ;
    //DXhttp_print_debug("step4:[%s] 002" , argon_input_for_login );

    extern int compute_signup_hash2(char *input, char* salt, char **rtBuf) ;
    char *rtBuf; rt = compute_signup_hash2(argon_input_for_login, login_sess, &rtBuf );
    if ( rt ) {
        DXhttp_print_debug("step4:[%s] , but failed with %d" , argon_input_for_login , rt);
        return rt ;
    }
    DXhttp_print_debug("step4:server calc hash : [%s]", rtBuf);

    if ( 0 != strncmp( rtBuf, trying_hash, SHA256_DIGEST_LENGTH*2 ) ){
        DXhttp_print_debug("step4:client submit hash : [%s], CMP error.", trying_hash );
        return 242041;
    }
    DXhttp_print_debug("step4:[%s] 010, get [%s], cmp OK." , argon_input_for_login , rtBuf);

    return 0 ;
}
