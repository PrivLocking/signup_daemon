// Handle form submission
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

    // Client-side validation
    const [formatValid, lengthValid] = isValidUsername(username);
    if (!formatValid) {
        if (messageElement) messageElement.textContent = msg.invalid_username_format;
        return;
    }
    if (!lengthValid) {
        if (messageElement) messageElement.textContent = msg.invalid_username_length;
        return;
    }

    if (password.length < 14) {
        if (messageElement) messageElement.textContent = msg.password_too_short;
        return;
    }

    if (!isValidPasswordChars(password)) {
        if (messageElement) messageElement.textContent = msg.invalid_password_chars;
        return;
    }
    
    // Check for repeating characters
    if (hasRepeatingChars(password)) {
        if (messageElement) messageElement.textContent = msg.password_repeating_chars;
        return;
    }
    
    // Check for sequential characters
    if (hasSequentialChars(password)) {
        if (messageElement) messageElement.textContent = msg.password_sequence_chars;
        return;
    }

    const passwdHash = await sha256(password);
    if (!passwdHash || passwdHash.match(/[^0-9a-f]/) || passwdHash.length !== 64) {
        if (messageElement) messageElement.textContent = msg.invalid_password;
        return;
    }

    // Disable submit button and visually indicate it's processing
    if (submitButton) {
        submitButton.disabled = true;
        submitButton.textContent = "Processing...";
        submitButton.setAttribute("aria-busy", "true");
    }

    // Send JSON to /signup
    try {
        const response = await fetch("signup", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ username, passwd: passwdHash })
        });

        let extraCode = 0;
        const status = response.status;
        if (status === 422) {
            const body = await response.text();
            extraCode = parseInt(body, 10) || 0;
        }

        if (status === 200) {
            // Success! Update button to "Success" state
            if (submitButton) {
                //submitButton.textContent = "Success!";
                submitButton.textContent = msg.success2 ;
                submitButton.classList.add("success-state");
                submitButton.setAttribute("aria-busy", "false");
            }
            
            // Show success message with countdown
            if (messageElement) {
                messageElement.textContent = msg.success;
                let timeLeft = 90;
                messageElement.innerHTML = `${msg.success} (<span id="countdown">${timeLeft}</span>s)`;
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
            // Keep submit button disabled during countdown
            // No need to re-enable - it stays disabled until redirect
        } else {
            // Show status and extra code
            const errorMsg = msg.error_status
                .replace("$status", status)
                .replace("$extraCode", extraCode);
            if (messageElement) messageElement.textContent = errorMsg;
            
            // Re-enable submit button on error and restore original text
            if (submitButton) {
                submitButton.disabled = false;
                //submitButton.textContent = "Sign Up";
                submitButton.textContent = msg.submit_btn ;
                submitButton.setAttribute("aria-busy", "false");
            }
        }
    } catch (e) {
        const errorMsg = msg.error_status
            .replace("$status", "0")
            .replace("$extraCode", "0");
        if (messageElement) messageElement.textContent = errorMsg;
        
        // Re-enable submit button on error and restore original text
        if (submitButton) {
            submitButton.disabled = false;
            //submitButton.textContent = "Sign Up";
            submitButton.textContent = msg.submit_btn ;
            submitButton.setAttribute("aria-busy", "false");
        }
    }
}

// Setup form submission
function setupFormSubmit() {
    const submitButton = document.getElementById("submit-btn");
    if (submitButton) {
        submitButton.addEventListener("click", handleFormSubmit);
    }
    
    // Also handle form submission via Enter key in username or password field
    const fields = [document.getElementById("identity"), document.getElementById("credential")];
    fields.forEach(field => {
        if (field) {
            field.addEventListener("keydown", function(e) {
                if (e.key === "Enter") {
                    e.preventDefault();
                    handleFormSubmit();
                }
            });
        }
    });
}

// Initialize the form
document.addEventListener("DOMContentLoaded", () => {
    initializeUI();
    setupPasswordListeners();
    setupFormSubmit();
    
    // Initialize password field
    initializePassword();
    
    // Add CSS for success state
    const style = document.createElement('style');
    style.textContent = `
        .success-state {
            background-color: #28a745 !important;
            border-color: #218838 !important;
        }
        button[aria-busy="true"] {
            pointer-events: none;
            position: relative;
        }
        button[aria-busy="true"]::after {
            content: "";
            animation: button-loading-spinner 1s linear infinite;
            position: absolute;
            right: 10px;
            top: 50%;
            width: 16px;
            height: 16px;
            margin-top: -8px;
            border: 2px solid rgba(255, 255, 255, 0.5);
            border-radius: 50%;
            border-top-color: white;
        }
        @keyframes button-loading-spinner {
            from {
                transform: rotate(0turn);
            }
            to {
                transform: rotate(1turn);
            }
        }
    `;
    document.head.appendChild(style);
});
