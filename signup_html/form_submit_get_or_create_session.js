/**
 * Gets or creates a signup session
 * @param {HTMLElement} messageElement - Element to display messages
 * @param {HTMLElement} submitButton - Submit button element
 * @param {Object} messages - UI messages
 * @returns {Promise<string|null>} - Session ID or null if failed
 */
async function getOrAskForAtmpSession(messageElement, submitButton, messages, username) {
    tmpJson = await askForNewTmpSession(messageElement, submitButton, messages, username);

    if (!tmpJson) {
        if (messageElement) messageElement.textContent = messages.too_more_tries + ":" + requestSignupSession_status + ":" + requestSignupSession_body ;
        if (submitButton) {
            submitButton.disabled = false;
            submitButton.textContent = messages.submit_btn;
            submitButton.setAttribute("aria-busy", "false");
        }
        return null;
    }
    return tmpJson
}

