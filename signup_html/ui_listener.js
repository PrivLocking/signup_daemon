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

