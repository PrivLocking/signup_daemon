
async function calc_passHash( jsonOBJ ) {
    let salt1 ;
    const messageElement = document.getElementById("message");
    const submitButton = document.getElementById("submit-btn");

    if ( funcName === "signup" ) {
        salt1 = jsonGetHex32( jsonOBJ, "signup_sess" );
    } else {
        salt1 = jsonGetHex32( jsonOBJ, "login_salt" );
    }

    // Generate Argon2 hash using signup_session + username + sha256(passwd)
    const argonInput1 = `${salt1}:${realUsername}:${realPassword}`;
    const passwdHash1 = await calc_passHash_ArgonSha256( argonInput1 );
    if ( !passwdHash1 ) {
        return null ;
    }

    if ( funcName === "signup" ) { // signup , only calc once
        //return passwdHash1 ;
        return JSON.stringify({ "ver": 1, "username": realUsername, "signup_salt":salt1 , "passwd": passwdHash1 });
    }

    if ( funcName !== "login" ) { // not signup or login, don't know how to calc
        console.log("not signup or login, don't know how to calc");
        handleSignupError(0, 0, messageElement, submitButton, msg);
    }

    const salt2 = jsonGetHex32( jsonOBJ, "login_sess" );

    const argonInput2 = `${salt2}:${realUsername}:${passwdHash1}`;
    const passwdHash2 = await calc_passHash_ArgonSha256( argonInput2 );
    if ( !passwdHash2 ) {
        return null ;
    }

    //return passwdHash2 ;
    return JSON.stringify({ "ver": 1, "username": realUsername, "login_salt":salt2 , "passwd": passwdHash2 });
}


async function calc_passHash_ArgonSha256( input ) {
    const lang = getCurrentLanguage();
    const msg = getMessages(lang);
    const submitButton = document.getElementById("submit-btn");
    const messageElement = document.getElementById("message");
    try {
        // Check if Argon2 is loaded, if not, load it
        if (typeof argon2_calc !== 'function') {
            if (messageElement) messageElement.textContent = msg.password_module_not_loaded;
            if (submitButton) {
                submitButton.disabled = false;
                submitButton.textContent = msg.submit_btn;
                submitButton.setAttribute("aria-busy", "false");
            }
            return null ;
        }
        
        // Use Argon2 to hash the input
        const argonHashX = argon2_calc(input);
        if (!argonHashX) {
            if (messageElement) messageElement.textContent = msg.password_calc_failed;
            if (submitButton) {
                submitButton.disabled = false;
                submitButton.textContent = msg.submit_btn;
                submitButton.setAttribute("aria-busy", "false");
            }
            return;
        }
        console.log("argon2_sha256_calc argon2 from [", input, "] to (", argonHashX, ")" );

        const passwdHashX = await sha256(argonHashX);
        if (!passwdHashX ){
            if (messageElement) messageElement.textContent = msg.invalid_password;
            return null ;
        }
        if (typeof passwdHashX !== 'string' ){
            if (messageElement) messageElement.textContent = msg.invalid_password;
            return null ;
        }
        if (passwdHashX.length !== 64 ){
            if (messageElement) messageElement.textContent = msg.invalid_password;
            return null ;
        }
        if (!/^[0-9a-f]+$/i.test(passwdHashX) ) {
            if (messageElement) messageElement.textContent = msg.invalid_password;
            return null ;
        }
        if (!passwdHashX || typeof passwdHashX !== 'string' ||  passwdHashX.length !== 64 || !/^[0-9a-f]+$/i.test(passwdHashX) ) {
            if (messageElement) messageElement.textContent = msg.invalid_password;
            return null ;
        }
        console.log("argon2_sha256_calc sha256 from [", argonHashX, "] to (", passwdHashX, ")" );
        return passwdHashX ;

    } catch (e) {
        console.error("Error in submission process:", e);
        handleSignupError(0, 0, messageElement, submitButton, msg);
        return null;
    }
}
