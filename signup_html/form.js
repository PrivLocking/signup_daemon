// Setup form submission
function setupFormSubmit() {
    const submitButton = document.getElementById("submit-btn");
    if (submitButton) {
        submitButton.addEventListener("click", handleFormSubmit);
    }
    
    // Also handle form submission via Enter key in username or password field
    const fields = [document.getElementById("identity"), document.getElementById("credential")];
    fields.forEach(field => {
        if (field) {
            field.addEventListener("keydown", function(e) {
                if (e.key === "Enter") {
                    e.preventDefault();
                    handleFormSubmit();
                }
            });
        }
    });
}

// Check if cookie exists
function getCookie(name) {
    const value = `; ${document.cookie}`;
    const parts = value.split(`; ${name}=`);
    if (parts.length === 2) return parts.pop().split(';').shift();
    return null;
}

// Request session cookie if not exists
async function checkSessionCookie() {
    const messageElement = document.getElementById("message");
    const submitButton = document.getElementById("submit-btn");
    
    if (!getCookie('signup_session')) {
        // Disable submit button while getting session
        if (submitButton) {
            submitButton.disabled = true;
        }
        
        const msg = getMessages(getCurrentLanguage());
        // Wait a bit before making the request
        setTimeout(async () => {
            try {
                const response = await fetch("signup", {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({})
                });
                
                if (response.ok) {
                    // Successfully set cookie, refresh after a short delay
                    setTimeout(() => {
                        window.location.reload();
                    }, 100);
                } else {
                    if (messageElement) {
                        messageElement.textContent = msg.password_sequence_chars ;
                    }
                }
            } catch (e) {
                if (messageElement) {
                    messageElement.textContent = msg.password_sequence_chars ;
                }
            }
        }, 100);
    }
}

// Initialize Argon2
function initializeArgon2() {
    if (typeof argon2Ready === 'function') {
        argon2Ready(function() {
            console.log("Argon2 initialized successfully");
        });
    } else {
        console.error("Argon2 module not found");
    }
}

// Initialize the form
document.addEventListener("DOMContentLoaded", () => {
    initializeUI();
    setupPasswordListeners();
    setupFormSubmit();
    
    // Initialize password field
    initializePassword();
    
    // Initialize Argon2
    initializeArgon2();
    
    // Check for session cookie
    checkSessionCookie();
    
    // Add CSS for success state
    const style = document.createElement('style');
    style.textContent = `
        .success-state {
            background-color: #28a745 !important;
            border-color: #218838 !important;
        }
        button[aria-busy="true"] {
            pointer-events: none;
            position: relative;
        }
        button[aria-busy="true"]::after {
            content: "";
            animation: button-loading-spinner 1s linear infinite;
            position: absolute;
            right: 10px;
            top: 50%;
            width: 16px;
            height: 16px;
            margin-top: -8px;
            border: 2px solid rgba(255, 255, 255, 0.5);
            border-radius: 50%;
            border-top-color: white;
        }
        @keyframes button-loading-spinner {
            from {
                transform: rotate(0turn);
            }
            to {
                transform: rotate(1turn);
            }
        }
    `;
    document.head.appendChild(style);
});
