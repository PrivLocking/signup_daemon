// Update password strength indicator
function updateStrength(password) {
    const indicator = document.getElementById("strength-indicator");
    if (!indicator) return;
    
    if (password.length === 0) {
        indicator.textContent = "";
        indicator.className = "";
        return;
    }
    
    const msg = getMessages(getCurrentLanguage());
    // Updated thresholds: weak < 14, medium 14-19, strong >= 20
    if (password.length < 14) {
        //indicator.textContent = "Weak";
        indicator.textContent = msg.passwd_weak ;
        indicator.className = "weak";
    } else if (password.length < 20) {
        //indicator.textContent = "Medium";
        indicator.textContent = msg.passwd_medium ;
        indicator.className = "medium";
    } else {
        //indicator.textContent = "Strong";
        indicator.textContent = msg.passwd_strong ;
        indicator.className = "strong";
    }
}

// Update password label length display
function updatePasswordLabelLength() {
    const lang = getCurrentLanguage();
    const msg = getMessages(lang);
    
    const passwordLabel = document.getElementById("password-label");
    
    if (!msg || !passwordLabel) return;
    
    passwordLabel.textContent = msg.password_label.replace("$length", realPassword.length);
}

// Update password preview
function updatePasswordPreview() {
    const preview = document.getElementById("password-preview");
    const showPasswordCheckbox = document.getElementById("show-password");
    
    if (!preview) return;
    
    // Show preview if "Show Password" is checked, otherwise show "." to keep element visible
    if (showPasswordCheckbox && showPasswordCheckbox.checked) {
        const grouped = realPassword.replace(/(.{4})/g, "$1 ").trim();
        preview.textContent = grouped;
    } else {
        //preview.textContent = ".";
        const length = realPassword.length;
        const masked = '*'.repeat(length);
        const grouped = masked.replace(/(.{4})/g, "$1 ").trim();
        preview.textContent = grouped;

    }
    preview.setAttribute("aria-hidden", "true");
}

// Toggle password visibility
function togglePasswordVisibility() {
    const passwordField = document.getElementById("credential");
    const showPasswordCheckbox = document.getElementById("show-password");
    
    if (passwordField && showPasswordCheckbox) {
        renderPassword();
        updatePasswordPreview();
    }
}


