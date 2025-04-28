function get_cookie_by_name(name) {
    try {
        // Get all cookies from document.cookie
        const allCookies = document.cookie;
        if (!allCookies) return null;
        
        // Split into individual cookies
        const cookies = allCookies.split(';');
        
        // Find the cookie with matching name
        for (const cookie of cookies) {
            const [cookieName, cookieValue] = cookie.trim().split('=');
            if (cookieName.trim() === name) {
                return cookieValue;
            }
        }
        
        // Cookie not found
        return null;
    } catch (error) {
        // Error handling
        return null;
    }
}

function get_cookie_md5_6() {
    const md5 = get_cookie_by_name("MD5");
    if (!md5) return null;
    
    // Check if the length is exactly 6
    if (md5.length !== 6) return null;
    
    return md5;
}
