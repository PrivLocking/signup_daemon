// SHA256 implementation
async function sha256(str) {
    try {
        const msgBuffer = new TextEncoder().encode(str);
        const hashBuffer = await crypto.subtle.digest("SHA-256", msgBuffer);
        const hashArray = Array.from(new Uint8Array(hashBuffer));
        return hashArray.map(b => b.toString(16).padStart(2, "0")).join("");
    } catch (e) {
        return null;
    }
}

// Get current language
function getCurrentLanguage() {
    const languageSelector = document.getElementById("language");
    return languageSelector ? languageSelector.value : "en";
}

// Get language messages
function getMessages(lang) {
    return messages[lang] || messages["en"]; // Fallback to English if language not found
}
