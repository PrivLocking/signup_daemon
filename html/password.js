// Store actual password in hidden element
let realPassword = "";

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

// Handle password input events
function handlePasswordInput(e) {
    const passwordField = document.getElementById("credential");
    const realPasswordField = document.getElementById("credential-real");
    const showPasswordCheckbox = document.getElementById("show-password");
    
    // Save caret position
    let startOffset = 0;
    try {
        const selection = window.getSelection();
        if (selection.rangeCount > 0) {
            const range = selection.getRangeAt(0);
            startOffset = range.startOffset;
        }
    } catch (error) {
        console.log("Selection error:", error);
        // Default to end of content if there's an error
        startOffset = passwordField.textContent.length;
    }
    
    // Get the text content from the password field
    const currentContent = passwordField.textContent;
    
    // Process the input
    if (e.inputType === "insertText") {
        // Insert the character at the cursor position
        realPassword = realPassword.substring(0, startOffset - 1) + 
                      e.data + 
                      realPassword.substring(startOffset - 1);
    } else if (e.inputType === "deleteContentBackward") {
        // Handle backspace
        if (startOffset <= realPassword.length) {
            realPassword = realPassword.substring(0, startOffset) + 
                          realPassword.substring(startOffset + 1);
        }
    } else if (e.inputType === "deleteContentForward") {
        // Handle delete key
        realPassword = realPassword.substring(0, startOffset) + 
                      realPassword.substring(startOffset + 1);
    } else if (e.inputType === "insertFromPaste") {
        // Handle paste - this gets complex as we need to figure out what changed
        // For now, just replace the entire content
        realPassword = currentContent;
    } else {
        // For other operations, just take the current content
        realPassword = currentContent;
    }
    
    // Store the real password
    if (realPasswordField) {
        realPasswordField.textContent = realPassword;
    }
    
    // Update the UI
    renderPassword();
    updateStrength(realPassword);
    updatePasswordLabelLength();
    updatePasswordPreview();
}

// Render the password field based on show/hide state
function renderPassword() {
    const passwordField = document.getElementById("credential");
    const showPasswordCheckbox = document.getElementById("show-password");
    
    if (!passwordField) return;
    
    const isShowingPassword = showPasswordCheckbox && showPasswordCheckbox.checked;
    
    // Get selection before modifying content
    let cursorPos = 0;
    try {
        const selection = window.getSelection();
        if (selection.rangeCount > 0) {
            const range = selection.getRangeAt(0);
            cursorPos = range.startOffset;
        } else {
            // No range, default to end
            cursorPos = passwordField.textContent.length;
        }
    } catch (error) {
        console.log("Selection error in renderPassword:", error);
        // Default to end of content if there's an error
        cursorPos = passwordField.textContent.length;
    }
    
    // Update the visual representation
    if (isShowingPassword) {
        // Show actual password
        passwordField.textContent = realPassword;
    } else {
        // Show asterisks
        passwordField.textContent = '*'.repeat(realPassword.length);
    }
    
    // Restore cursor position
    if (passwordField === document.activeElement) {
        placeCaretAtPosition(passwordField, Math.min(cursorPos, passwordField.textContent.length));
    }
}

// Place caret at specific position in contenteditable
function placeCaretAtPosition(element, position) {
    if (!element) return;
    
    // Make sure position is valid
    position = Math.max(0, Math.min(position, element.textContent.length));
    
    try {
        element.focus();
        const selection = window.getSelection();
        const range = document.createRange();
        
        if (element.firstChild) {
            // If there's a text node
            range.setStart(element.firstChild, position);
        } else {
            // If the element is empty
            range.setStart(element, 0);
        }
        
        range.collapse(true);
        selection.removeAllRanges();
        selection.addRange(range);
    } catch (error) {
        console.log("Error placing caret:", error);
    }
}

// Setup password-related event listeners
function setupPasswordListeners() {
    const passwordField = document.getElementById("credential");
    const showPasswordCheckbox = document.getElementById("show-password");
    
    if (passwordField) {
        // Use input event to capture all changes
        passwordField.addEventListener("input", handlePasswordInput);
        
        // Handle key events for special keys
        passwordField.addEventListener("keydown", function(e) {
            let cursorPos = 0;
            try {
                const selection = window.getSelection();
                if (selection.rangeCount > 0) {
                    const range = selection.getRangeAt(0);
                    cursorPos = range.startOffset;
                }
            } catch (error) {
                console.log("Selection error in keydown:", error);
                return; // Skip special handling if we can't get selection
            }
            
            // Special handling for arrow keys to navigate through asterisks
            if (e.key === "ArrowLeft" || e.key === "ArrowRight") {
                // Let default behavior happen
                return;
            }
            
            // Handle delete key
            if (e.key === "Delete") {
                e.preventDefault();
                
                if (cursorPos < realPassword.length) {
                    realPassword = realPassword.substring(0, cursorPos) + 
                                 realPassword.substring(cursorPos + 1);
                    renderPassword();
                    updateStrength(realPassword);
                    updatePasswordLabelLength();
                    updatePasswordPreview();
                    placeCaretAtPosition(passwordField, cursorPos);
                }
            }
        });
        
        // Handle paste event
        passwordField.addEventListener("paste", function(e) {
            e.preventDefault();
            const pastedText = (e.originalEvent || e).clipboardData.getData('text/plain');
            
            // Get current selection
            let startPos = 0;
            let endPos = 0;
            try {
                const selection = window.getSelection();
                if (selection.rangeCount > 0) {
                    const range = selection.getRangeAt(0);
                    startPos = range.startOffset;
                    endPos = range.endOffset;
                }
            } catch (error) {
                console.log("Selection error in paste:", error);
                // Default to end of content
                startPos = endPos = passwordField.textContent.length;
            }
            
            // Update real password with pasted text
            realPassword = realPassword.substring(0, startPos) + 
                         pastedText + 
                         realPassword.substring(endPos);
            
            // Update the UI
            renderPassword();
            updateStrength(realPassword);
            updatePasswordLabelLength();
            updatePasswordPreview();
            
            // Place cursor after pasted text
            placeCaretAtPosition(passwordField, startPos + pastedText.length);
        });
    }
    
    if (showPasswordCheckbox && passwordField) {
        showPasswordCheckbox.addEventListener("change", togglePasswordVisibility);
    }
}

// Initialize password field
function initializePassword() {
    const passwordField = document.getElementById("credential");
    if (passwordField) {
        // Clear any existing content
        passwordField.textContent = "";
        realPassword = "";
        
        // Initial rendering
        renderPassword();
        updatePasswordLabelLength();
    }
}
