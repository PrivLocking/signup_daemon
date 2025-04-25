/**
 * Gets or creates a signup session
 * @param {HTMLElement} messageElement - Element to display messages
 * @param {HTMLElement} submitButton - Submit button element
 * @param {Object} messages - UI messages
 * @returns {Promise<string|null>} - Session ID or null if failed
 */
async function getOrCreateSignupSession(messageElement, submitButton, messages) {
    // Check if signup_session cookie exists
    let signupSession = getCookie(`${funcName}_session`);
    
    // If no cookie exists, request one
    if (!signupSession) {
        signupSession = await requestSignupSession(messageElement, submitButton, messages);
        
        // If we still don't have a session, show error
        if (!signupSession) {
            if (messageElement) messageElement.textContent = messages.too_more_tries + ":" + requestSignupSession_status + ":" + requestSignupSession_body ;
            if (submitButton) {
                submitButton.disabled = false;
                submitButton.textContent = messages.submit_btn;
                submitButton.setAttribute("aria-busy", "false");
            }
            return null;
        }
    }
    
    return signupSession;
}

