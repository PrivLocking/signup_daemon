// session-manager.js - Handles session creation and management

/**
 * Resets the UI elements when an error occurs
 * @param {HTMLElement} messageElement - Element to display error message
 * @param {HTMLButtonElement} submitButton - Button to reset
 * @param {Object} messages - Message texts
 */
function handleSessionError(messageElement, submitButton, messages) {
    if (messageElement) messageElement.textContent = messages.error_get_signup_session;
    if (submitButton) {
        submitButton.disabled = false;
        submitButton.textContent = messages.submit_btn;
        submitButton.setAttribute("aria-busy", "false");
    }
}

function jsonGetHex32(jsonSrc, name) {
    return jsonGetHexLen(jsonSrc, name, 32) ;
}
function jsonGetHexLen(jsonSrc, name, len) {
    try {
        if (!jsonSrc || typeof jsonSrc !== 'object') {
            console.log('Invalid JSON source');
            return null;
        }
        if (typeof name !== 'string') {
            console.log('Name must be a string');
            return null ;
        }

        // Check JSON version first
        if (jsonSrc.ver !== 1) {
            console.log("Unsupported JSON version:", jsonSrc.ver);
            return null;
        }

        const sess = jsonSrc[`${name}`];

        if (!sess || typeof sess !== 'string' || sess.length !== len || !/^[0-9a-f]+$/.test(sess)) {
            console.log('Value is not a valid 32-byte hex string');
            return null ;
        }
        return sess;

    } catch (jsonError) {
        // If JSON parsing fails, return error immediately
        console.error("Failed to parse response as JSON:", jsonError);
        handleSessionError(messageElement, submitButton, messages);
        requestSignupSession_body = "Failed to parse response as JSON" ;
        return null;
    }
}

/**
 * Requests a new signup session from the server
 * @returns {Promise<string|null>} - Session ID or null if failed
 */
let requestSignupSession_body = "" ;
let requestSignupSession_status = "" ;
let requestSignupSession_header = "" ;
let requestSignupSession_json = null ;
async function askForNewTmpSession(messageElement, submitButton, messages) {
    requestSignupSession_body = "" ;
    requestSignupSession_status = "" ;
    requestSignupSession_header = "" ;
    requestSignupSession_json = null ;
    try {
        let bodyX ;
        if ( "login" === funcName ){
            bodyX = JSON.stringify({
                ver: 1,
                username: realUsername
            }) ;
        } else {
            bodyX = JSON.stringify({ ver:1 }) ;
        }
        const sessionResponse = await fetch(funcName, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: bodyX
        });
        requestSignupSession_status = sessionResponse.status ;
        requestSignupSession_header = sessionResponse.headers ;
        
        if (sessionResponse.ok) {
            // Get response content
            const responseContent = await sessionResponse.text();
            let sessionId = null;
            
            try {
                // Try to parse as JSON
                const jsonResponse = JSON.parse(responseContent);

                requestSignupSession_json = jsonResponse ;
                return jsonResponse ;
                
                // Check JSON version first
                if (jsonResponse.ver !== 1) {
                    console.log("Unsupported JSON version:", jsonResponse.ver);
                    handleSessionError(messageElement, submitButton, messages);
                    requestSignupSession_body = "Unsupported JSON version" ;
                    return null;
                }
                
                const sess = jsonResponse[`${funcName}_sess`];
                // Validate signup_sess format
                if (sess && 
                    sess.length === 32 && 
                    /^[0-9a-f]+$/.test(sess)) {
                    console.log("Got valid session from JSON response:", sess);
                    sessionId = sess;
                } else {
                    console.log("JSON response contained invalid session format:", sess);
                    handleSessionError(messageElement, submitButton, messages);
                    requestSignupSession_body = "JSON response contained invalid session format" ;
                    return null;
                }
            } catch (jsonError) {
                // If JSON parsing fails, return error immediately
                console.error("Failed to parse response as JSON:", jsonError);
                handleSessionError(messageElement, submitButton, messages);
                requestSignupSession_body = "Failed to parse response as JSON" ;
                return null;
            }
            
            // If we couldn't get valid session from response, try cookies as fallback
            /*
            if (!sessionId) {
                const sessionFromCookie = getCookie('signup_session');
                console.log("Got session from cookie after request:", sessionFromCookie);
                sessionId = sessionFromCookie;
            }
            */
            
            return sessionId || null;
        } else { // sessionResponse not ok, maybe , 422
            handleSessionError(messageElement, submitButton, messages);
            requestSignupSession_body = await sessionResponse.text();
            requestSignupSession_status = sessionResponse.status ;
            requestSignupSession_header = sessionResponse.headers ;
            return null;
        }
    } catch (e) {
        console.error(`Error requesting ${funcName} session:`, e);
        handleSessionError(messageElement, submitButton, messages);
        requestSignupSession_body = `Error requesting ${funcName} session` ;
        return null;
    }
}

