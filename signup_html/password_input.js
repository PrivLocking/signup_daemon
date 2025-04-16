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

