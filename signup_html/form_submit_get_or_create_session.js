/**
 * Gets or creates a signup session
 * @param {HTMLElement} messageElement - Element to display messages
 * @param {HTMLElement} submitButton - Submit button element
 * @param {Object} messages - UI messages
 * @returns {Promise<string|null>} - Session ID or null if failed
 */
async function getOrAskForAtmpSession(messageElement, submitButton, messages, username) {
    // Check if signup_session cookie exists
    let tmpSession = getCookie(`${funcName}_session`);
    
    // If no cookie exists, request one
    if (!tmpSession) {
        tmpSession = await askForNewTmpSession(messageElement, submitButton, messages, username);
        
        // If we still don't have a session, show error
        if (!tmpSession) {
            if (messageElement) messageElement.textContent = messages.too_more_tries + ":" + requestSignupSession_status + ":" + requestSignupSession_body ;
            if (submitButton) {
                submitButton.disabled = false;
                submitButton.textContent = messages.submit_btn;
                submitButton.setAttribute("aria-busy", "false");
            }
            return null;
        }
    }
    
    return tmpSession;
}

