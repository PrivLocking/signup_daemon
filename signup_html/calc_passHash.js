
async function calc_passHash() {
    // Generate Argon2 hash using signup_session + username + sha256(passwd)
    const argonInput = `${formTmpSession}:${username}:${password}`;
    
    try {
        // Check if Argon2 is loaded, if not, load it
        if (typeof argon2_calc !== 'function') {
            if (messageElement) messageElement.textContent = msg.password_module_not_loaded;
            if (submitButton) {
                submitButton.disabled = false;
                submitButton.textContent = msg.submit_btn;
                submitButton.setAttribute("aria-busy", "false");
            }
            return null;
        }
        
        // Use Argon2 to hash the input
        const argonHash = argon2_calc(argonInput);
        if (!argonHash) {
            if (messageElement) messageElement.textContent = msg.password_calc_failed;
            if (submitButton) {
                submitButton.disabled = false;
                submitButton.textContent = msg.submit_btn;
                submitButton.setAttribute("aria-busy", "false");
            }
            return;
        }
        console.log("argon2_calc from [", argonInput, "] to (", argonHash, ")" );

        const passwdHash = await sha256(argonHash);
        if (!passwdHash || passwdHash.match(/[^0-9a-f]/) || passwdHash.length !== 64) {
            if (messageElement) messageElement.textContent = msg.invalid_password;
            return;
        }

        const jsonPass = JSON.stringify({ "username": username, "signup_salt":formTmpSession , "passwd": passwdHash });

        return jsonPass;

    } catch (e) {
        console.error("Error in submission process:", e);
        handleSignupError(0, 0, messageElement, submitButton, msg);
        return null;
    }
}
