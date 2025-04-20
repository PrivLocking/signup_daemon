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
    setupClearButtonListeners();
    setupPasswordListeners();
}
