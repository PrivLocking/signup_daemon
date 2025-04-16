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

