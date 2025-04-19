// utils.js - Contains utility functions

/**
 * Computes SHA-256 hash of input
 * @param {string} input - String to hash
 * @returns {Promise<string>} - Hex-encoded SHA-256 hash
 */
async function sha256(input) {
    // Note: Implementation not shown in the original code
    // This would compute a SHA-256 hash
    return "0123456789abcdef"; // Placeholder
}

/**
 * Gets a cookie by name
 * @param {string} name - Cookie name
 * @returns {string|null} - Cookie value or null if not found
 */
function getCookie(name) {
    // Note: Implementation not shown in the original code
    // This would retrieve a cookie value
    return null; // Placeholder
}

/**
 * Gets current UI language
 * @returns {string} - Current language code
 */
function getCurrentLanguage() {
    // Note: Implementation not shown in the original code
    // This would return the current language setting
    return "en"; // Placeholder
}

/**
 * Gets UI messages for the specified language
 * @param {string} lang - Language code
 * @returns {Object} - Object containing UI messages
 */
function getMessages(lang) {
    // Note: Implementation not shown in the original code
    // This would return messages for the specified language
    return {
        invalid_username_format: "Invalid username format",
        invalid_username_length: "Username length is invalid",
        password_too_short: "Password must be at least 14 characters",
        invalid_password_chars: "Password contains invalid characters",
        password_repeating_chars: "Password contains repeating characters",
        password_sequence_chars: "Password contains sequential characters",
        invalid_password: "Invalid password",
        missing_signup_session: "Missing signup session",
        error_get_signup_session: "Error getting signup session",
        password_module_not_loaded: "Password module not loaded",
        password_calc_failed: "Password calculation failed",
        submit_btn: "Submit",
        success: "Registration successful! Redirecting to login page",
        success2: "Success!",
        error_status: "Error: status $status (code $extraCode)"
    }; // Placeholder
}
