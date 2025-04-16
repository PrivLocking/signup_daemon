// Store actual password in hidden element
let realPassword = "";


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

