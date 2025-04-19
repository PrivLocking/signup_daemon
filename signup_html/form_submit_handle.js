/**
 * Handles successful signup
 * @param {HTMLElement} submitButton - Submit button element
 * @param {HTMLElement} messageElement - Message display element
 * @param {Object} messages - UI messages object
 */
function handleSignupSuccess(submitButton, messageElement, messages) {
    // Update button to "Success" state
    if (submitButton) {
        submitButton.textContent = messages.success2;
        submitButton.classList.add("success-state");
        submitButton.setAttribute("aria-busy", "false");
    }
    
    // Show success message with countdown
    if (messageElement) {
        let timeLeft = 90;
        messageElement.innerHTML = `${messages.success} (<span id="countdown">${timeLeft}</span>s)`;
        const countdown = setInterval(() => {
            timeLeft--;
            const countdownElement = document.getElementById("countdown");
            if (countdownElement) countdownElement.textContent = timeLeft;
            if (timeLeft <= 0) {
                clearInterval(countdown);
                window.location.href = "/login/";
            }
        }, 1000);
    }
}

/**
 * Handles form submission errors
 * @param {number} status - HTTP status code
 * @param {number} extraCode - Extra error code
 * @param {HTMLElement} messageElement - Message display element
 * @param {HTMLElement} submitButton - Submit button element
 * @param {Object} messages - UI messages object
 */
function handleSignupError(status, extraCode, messageElement, submitButton, messages) {
    // Show status and extra code
    const errorMsg = messages.error_status
        .replace("$status", status)
        .replace("$extraCode", extraCode);
    
    if (messageElement) messageElement.textContent = errorMsg;
    
    // Re-enable submit button on error and restore original text
    if (submitButton) {
        submitButton.disabled = false;
        submitButton.textContent = messages.submit_btn;
        submitButton.setAttribute("aria-busy", "false");
    }
}

/**
 * Validates form inputs
 * @param {string} username - Username
 * @param {string} password - Password
 * @param {HTMLElement} messageElement - Message display element
 * @param {Object} messages - UI messages object
 * @returns {boolean} - Whether validation passed
 */
function validateFormInputs(username, password, messageElement, messages) {
    // Client-side validation
    const [formatValid, lengthValid] = isValidUsername(username);
    if (!formatValid) {
        if (messageElement) messageElement.textContent = messages.invalid_username_format;
        return false;
    }
    if (!lengthValid) {
        if (messageElement) messageElement.textContent = messages.invalid_username_length;
        return false;
    }

    if (password.length < 14) {
        if (messageElement) messageElement.textContent = messages.password_too_short;
        return false;
    }

    if (!isValidPasswordChars(password)) {
        if (messageElement) messageElement.textContent = messages.invalid_password_chars;
        return false;
    }
    
    // Check for repeating characters
    if (hasRepeatingChars(password)) {
        if (messageElement) messageElement.textContent = messages.password_repeating_chars;
        return false;
    }
    
    // Check for sequential characters
    if (hasSequentialChars(password)) {
        if (messageElement) messageElement.textContent = messages.password_sequence_chars;
        return false;
    }
    
    return true;
}

