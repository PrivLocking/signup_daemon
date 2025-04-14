// Validate username
function isValidUsername(username) {
    const formatRegex = /^[a-z][a-z0-9\-_]*[a-z0-9]$/;
    const lengthRegex = /^.{8,20}$/;
    return [formatRegex.test(username), lengthRegex.test(username)];
}

// Validate password characters (ASCII dec 32–125)
function isValidPasswordChars(password) {
    const charRegex = /^[\x20-\x7D]+$/;
    return charRegex.test(password);
}

// Check for repeating characters (more than 3 of the same in a row)
function hasRepeatingChars(password) {
    return /(.)\1{3,}/.test(password);
}

// Check for sequential characters (more than 3 in sequence)
function hasSequentialChars(password) {
    // Check for ascending sequences like "1234", "abcd"
    for (let i = 0; i < password.length - 3; i++) {
        if (
            password.charCodeAt(i + 1) === password.charCodeAt(i) + 1 &&
            password.charCodeAt(i + 2) === password.charCodeAt(i) + 2 &&
            password.charCodeAt(i + 3) === password.charCodeAt(i) + 3
        ) {
            return true;
        }
    }
    
    // Check for descending sequences like "4321", "dcba"
    for (let i = 0; i < password.length - 3; i++) {
        if (
            password.charCodeAt(i + 1) === password.charCodeAt(i) - 1 &&
            password.charCodeAt(i + 2) === password.charCodeAt(i) - 2 &&
            password.charCodeAt(i + 3) === password.charCodeAt(i) - 3
        ) {
            return true;
        }
    }
    
    return false;
}
