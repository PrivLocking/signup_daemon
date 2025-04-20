// Setup language switch event listener
function setupLanguageListener() {
    const languageSelector = document.getElementById("language");
    if (languageSelector) {
        languageSelector.addEventListener("change", (e) => {
            applyLanguage(e.target.value);
        });
    }
    
    // Initialize language
    const savedLang = localStorage.getItem("signup_lang") || "en";
    if (languageSelector) {
        languageSelector.value = savedLang;
        applyLanguage(savedLang);
    }
}

// Setup username lowercase conversion
function setupUsernameListener() {
    const usernameField = document.getElementById("identity");
    if (usernameField) {
        usernameField.addEventListener("input", function() {
            const text = this.textContent.toLowerCase();
            if (text !== this.textContent) {
                // Only update if different to avoid cursor jumping
                this.textContent = text;
                // Place cursor at end
                placeCaretAtEnd(this);
            }
        });
        
        // Handle paste to strip formatting
        usernameField.addEventListener("paste", function(e) {
            e.preventDefault();
            const text = (e.originalEvent || e).clipboardData.getData('text/plain').toLowerCase();
            document.execCommand("insertText", false, text);
        });
    }
}

// Setup clear button listeners
function setupClearButtonListeners() {
    const clearUsername = document.getElementById("clear-username");
    const clearPassword = document.getElementById("clear-password");
    const usernameField = document.getElementById("identity");
    const passwordField = document.getElementById("credential");

    if (clearUsername && usernameField) {
        clearUsername.addEventListener("click", function() {
            usernameField.textContent = "";
            usernameField.focus();
        });
    }

    if (clearPassword && passwordField) {
        clearPassword.addEventListener("click", function() {
            passwordField.textContent = "";
            realPassword = "";
            updatePasswordLabelLength();
            renderPassword();
            passwordField.focus();
        });
    }
}
