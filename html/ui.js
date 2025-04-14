// Apply language to UI
function applyLanguage(lang) {
    const msg = getMessages(lang);
    
    const passwordField = document.getElementById("credential");
    if (!passwordField) return;
    
    const password = passwordField.textContent;
    
    const elements = {
        "title": document.getElementById("title"),
        "username-label": document.getElementById("username-label"),
        "password-label": document.getElementById("password-label"),
        "show-password-label": document.getElementById("show-password-label"),
        "submit-btn": document.getElementById("submit-btn"),
        "message": document.getElementById("message")
    };
    
    if (elements["title"]) elements["title"].textContent = msg.title;
    if (elements["username-label"]) elements["username-label"].textContent = msg.username_label;
    if (elements["password-label"]) elements["password-label"].textContent = msg.password_label.replace("$length", password.length);
    if (elements["show-password-label"]) elements["show-password-label"].textContent = msg.show_password_label;
    if (elements["submit-btn"]) elements["submit-btn"].textContent = msg.submit_btn;
    if (elements["message"]) elements["message"].textContent = "";
    
    localStorage.setItem("signup_lang", lang);
    updatePasswordPreview();
}

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

// Helper function to place caret at end of contenteditable
function placeCaretAtEnd(el) {
    el.focus();
    if (typeof window.getSelection != "undefined" && typeof document.createRange != "undefined") {
        const range = document.createRange();
        range.selectNodeContents(el);
        range.collapse(false);
        const sel = window.getSelection();
        sel.removeAllRanges();
        sel.addRange(range);
    } else if (typeof document.body.createTextRange != "undefined") {
        const textRange = document.body.createTextRange();
        textRange.moveToElementText(el);
        textRange.collapse(false);
        textRange.select();
    }
}

// Initialize UI elements and listeners
function initializeUI() {
    setupLanguageListener();
    setupUsernameListener();
    setupPasswordListeners();
}
