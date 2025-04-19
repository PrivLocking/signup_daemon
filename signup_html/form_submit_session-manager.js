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

/**
 * Requests a new signup session from the server
 * @returns {Promise<string|null>} - Session ID or null if failed
 */
async function requestSignupSession(messageElement, submitButton, messages) {
    try {
        const sessionResponse = await fetch("signup", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({})
        });
        
        if (sessionResponse.ok) {
            // Get response content
            const responseContent = await sessionResponse.text();
            let sessionId = null;
            
            try {
                // Try to parse as JSON
                const jsonResponse = JSON.parse(responseContent);
                
                // Check JSON version first
                if (jsonResponse.ver !== 1) {
                    console.log("Unsupported JSON version:", jsonResponse.ver);
                    handleSessionError(messageElement, submitButton, messages);
                    return null;
                }
                
                // Validate signup_sess format
                if (jsonResponse.signup_sess && 
                    jsonResponse.signup_sess.length === 32 && 
                    /^[0-9a-f]+$/.test(jsonResponse.signup_sess)) {
                    console.log("Got valid session from JSON response:", jsonResponse.signup_sess);
                    sessionId = jsonResponse.signup_sess;
                } else {
                    console.log("JSON response contained invalid session format:", jsonResponse.signup_sess);
                    handleSessionError(messageElement, submitButton, messages);
                    return null;
                }
            } catch (jsonError) {
                // If JSON parsing fails, return error immediately
                console.error("Failed to parse response as JSON:", jsonError);
                handleSessionError(messageElement, submitButton, messages);
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
        } else {
            handleSessionError(messageElement, submitButton, messages);
            return null;
        }
    } catch (e) {
        console.error("Error requesting signup session:", e);
        handleSessionError(messageElement, submitButton, messages);
        return null;
    }
}

