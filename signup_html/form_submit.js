
/**
 * Main form submission handler
 */
let realUsername = "";
async function handleFormSubmit() {
    const lang = getCurrentLanguage();
    const msg = getMessages(lang);
    const username = document.getElementById("identity").textContent.trim();
    realUsername = username ;
    const password = realPassword; // Use the global realPassword variable
    const submitButton = document.getElementById("submit-btn");

    // Clear previous message
    const messageElement = document.getElementById("message");
    if (messageElement) {
        messageElement.textContent = "";
    }

    // Validate form inputs
    if (!validateFormInputs(username, password, messageElement, msg)) {
        return;
    }

    // Disable submit button and visually indicate it's processing
    if (submitButton) {
        submitButton.disabled = true;
        submitButton.textContent = "Processing..";
        submitButton.setAttribute("aria-busy", "true");
    }

    // Get or create signup session // askForNewTmpSession
    const tmpJson = await getOrAskForAtmpSession(messageElement, submitButton, msg);
    if (!tmpJson) {
        return;
    }

    
    //const jsonPass = await JSON.stringify({ "username": username, "signup_salt":tmpJson , "passwd": passwdHash });
    const jsonPass = await calc_passHash( tmpJson );

    if ( !jsonPass ) {
        console.error("Error in passHash calcing." );
        handleSignupError(1, 1, messageElement, submitButton, msg);
        return ;
    }

    try {

        // Send JSON to /signup
        //const response = await fetch("signup", {
        const response = await fetch(funcName, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: jsonPass
        });
        console.log("real POST body (", jsonPass, ")" );

        let extraCode = 0;
        const status = response.status;
        if (status === 422) {
            const body = await response.text();
            //extraCode = parseInt(body, 10) || 0;
            extraCode = body ;
        }

        if (status === 200) {
            handleSignupSuccess(submitButton, messageElement, msg);
        } else {
            handleSignupError(status, extraCode, messageElement, submitButton, msg);
        }
    } catch (e) {
        console.error("Error in submission process:", e);
        handleSignupError(0, 0, messageElement, submitButton, msg);
    }
}
