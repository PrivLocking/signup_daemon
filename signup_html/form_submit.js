
/**
 * Main form submission handler
 */
async function handleFormSubmit() {
    const lang = getCurrentLanguage();
    const msg = getMessages(lang);
    const username = document.getElementById("identity").textContent.trim();
    const password = realPassword; // Use the global realPassword variable
    const submitButton = document.getElementById("submit-btn");

    // Clear previous message
    const messageElement = document.getElementById("message");
    if (messageElement) {
        messageElement.textContent = "";
    }

    // Validate form inputs
    if (!validateFormInputs(username, password, messageElement, msg)) {
        return;
    }

    // Disable submit button and visually indicate it's processing
    if (submitButton) {
        submitButton.disabled = true;
        submitButton.textContent = "Processing..";
        submitButton.setAttribute("aria-busy", "true");
    }

    // Get or create signup session
    const signupSession = await getOrCreateSignupSession(messageElement, submitButton, msg);
    if (!signupSession) {
        return;
    }

    // Generate Argon2 hash using signup_session + username + sha256(passwd)
    const argonInput = `${signupSession}:${username}:${password}`;
    
    try {
        // Check if Argon2 is loaded, if not, load it
        if (typeof argon2_calc !== 'function') {
            if (messageElement) messageElement.textContent = msg.password_module_not_loaded;
            if (submitButton) {
                submitButton.disabled = false;
                submitButton.textContent = msg.submit_btn;
                submitButton.setAttribute("aria-busy", "false");
            }
            return;
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

        const jsonPass = JSON.stringify({ "username": username, "signup_salt":signupSession , "passwd": passwdHash });

        // Send JSON to /signup
        const response = await fetch("signup", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: jsonPass
        });
        console.log("real POST body (", jsonPass, ")" );

        let extraCode = 0;
        const status = response.status;
        if (status === 422) {
            const body = await response.text();
            extraCode = parseInt(body, 10) || 0;
        }

        if (status === 200) {
            handleSignupSuccess(submitButton, messageElement, msg);
        } else {
            handleSignupError(status, extraCode, messageElement, submitButton, msg);
        }
    } catch (e) {
        console.error("Error in submission process:", e);
        handleSignupError(0, 0, messageElement, submitButton, msg);
    }
}
