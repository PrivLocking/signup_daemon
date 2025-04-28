// Wait for DOM to be fully loaded
document.addEventListener('DOMContentLoaded', function() {
  // Get DOM elements
  const statusElement = document.getElementById('status');
  const calculateSignupButton = document.getElementById('calculate-signup');
  const calculateLoginButton = document.getElementById('calculate-login');
  const tabs = document.querySelectorAll('.tab');
  const tabContents = document.querySelectorAll('.tab-content');
  const generateSessionButton = document.getElementById('generate-session');
  const generateLoginSessionButton = document.getElementById('generate-login-session');
  const clearButtons = document.querySelectorAll('.clear-button');
  
  // Setup clear buttons
  clearButtons.forEach(button => {
    const inputId = button.getAttribute('data-for');
    const input = document.getElementById(inputId);
    
    // Initially hide the clear button if the input is empty
    button.style.display = input.value ? 'block' : 'none';
    
    // Show/hide clear button based on input content
    input.addEventListener('input', () => {
      button.style.display = input.value ? 'block' : 'none';
    });
    
    // Clear input when button is clicked
    button.addEventListener('click', () => {
      input.value = '';
      button.style.display = 'none';
      input.focus(); // Optional: refocus on the input
    });
  });
  
  // Argon2 configuration
  const getArgon2Config = () => {
    return {
      time: parseInt(document.getElementById('time').value),
      mem: parseInt(document.getElementById('mem').value),
      hashLen: parseInt(document.getElementById('hash-length').value),
      parallelism: parseInt(document.getElementById('parallelism').value),
      type: 2  // Argon2id
    };
  };

  // Initialize Argon2
  argon2Ready(function() {
    statusElement.textContent = "Argon2 ready";
    statusElement.classList.remove('loading');
    calculateSignupButton.disabled = false;
    calculateLoginButton.disabled = false;
    
    // Show generate button once Argon2 is ready
    generateSessionButton.style.display = 'none';
  });

  // Generate random hex string
  function generateRandomHex(length) {
    const array = new Uint8Array(length / 2);
    window.crypto.getRandomValues(array);
    return Array.from(array)
      .map(b => b.toString(16).padStart(2, '0'))
      .join('');
  }

  // Convert string to ArrayBuffer for Web Crypto API
  async function stringToArrayBuffer(str) {
    const encoder = new TextEncoder();
    return encoder.encode(str);
  }

  // Calculate SHA-256 hash using Web Crypto API instead of CryptoJS
  async function calculateSHA256(data) {
    let dataBuffer;
    
    if (typeof data === 'string') {
      dataBuffer = await stringToArrayBuffer(data);
    } else {
      dataBuffer = data;
    }
    
    const hashBuffer = await window.crypto.subtle.digest('SHA-256', dataBuffer);
    const hashArray = Array.from(new Uint8Array(hashBuffer));
    return hashArray.map(b => b.toString(16).padStart(2, '0')).join('');
  }

  // Tab functionality
  tabs.forEach(tab => {
    tab.addEventListener('click', () => {
      tabs.forEach(t => t.classList.remove('active'));
      tabContents.forEach(tc => tc.classList.remove('active'));

      tab.classList.add('active');
      document.getElementById(`${tab.dataset.tab}-tab`).classList.add('active');
      statusElement.textContent = ".";
    });
  });

  // Generate session buttons
  generateSessionButton.addEventListener('click', () => {
    const signupSessionInput = document.getElementById('signup-session');
    signupSessionInput.value = generateRandomHex(32);
    // Show the clear button when value is set
    document.querySelector(`[data-for="signup-session"]`).style.display = 'block';
  });

  generateLoginSessionButton.addEventListener('click', () => {
    const loginSessionInput = document.getElementById('login-session');
    loginSessionInput.value = generateRandomHex(32);
    // Show the clear button when value is set
    document.querySelector(`[data-for="login-session"]`).style.display = 'block';
  });

  // Calculate signup hash
  calculateSignupButton.addEventListener('click', async function() {
    const username = document.getElementById('signup-username').value;
    const password = document.getElementById('signup-password').value;
    const signupSession = document.getElementById('signup-session').value;

    if (!username || !password || !signupSession) {
      statusElement.textContent = "Please fill in all fields";
      statusElement.classList.add('error');
      return;
    }

    statusElement.textContent = "SignUp Calculating...starting";
    statusElement.classList.add('loading');
    statusElement.classList.remove('error');

    try {
      // Step 1: Combine salt + username + password
      const saltUsernamePwd = `${signupSession}:${username}:${password}`;
      document.getElementById('salt-username-pwd').textContent = saltUsernamePwd;

      // Step 2: Calculate Argon2 hash
      const argon2Hash = argon2_calc(saltUsernamePwd, getArgon2Config());
      document.getElementById('argon2-hash').textContent = argon2Hash;

      // Step 3: Calculate SHA256 of Argon2 hash using Web Crypto API
      const sha256Hash = await calculateSHA256(argon2Hash);
      document.getElementById('sha256-hash').textContent = sha256Hash;

      // Create final JSON payload
      const jsonPayload = {
        "username": username,
        "signup_salt": signupSession,
        "passwd": sha256Hash
      };

      document.getElementById('signup-json').textContent = JSON.stringify(jsonPayload, null, 2);

      statusElement.textContent = "SignUp Hash calculated successfully";
      statusElement.classList.remove('loading');
    } catch (error) {
      statusElement.textContent = "Error: " + error.message;
      statusElement.classList.remove('loading');
      statusElement.classList.add('error');
    }
  });

  // Calculate login hash
  calculateLoginButton.addEventListener('click', async function() {
    const username = document.getElementById('login-username').value;
    const password = document.getElementById('login-password').value;
    const signupSession = document.getElementById('signup-session-login').value;
    const loginSession = document.getElementById('login-session').value;

    if (!username || !password || !signupSession || !loginSession) {
      statusElement.textContent = "Please fill in all login fields";
      statusElement.classList.add('error');
      return;
    }

    statusElement.textContent = "Calculating login hash...starting";
    statusElement.classList.add('loading');
    statusElement.classList.remove('error');

    try {
      // Step 1: Combine signup salt + username + password
      const saltUsernamePwd = `${signupSession}:${username}:${password}`;
      document.getElementById('login-salt-username-pwd').textContent = saltUsernamePwd;

      // Step 2: Calculate Argon2 hash of saltUsernamePwd
      const argon2Hash = argon2_calc(saltUsernamePwd, getArgon2Config());
      document.getElementById('login-argon2-hash').textContent = argon2Hash;

      // Step 3: Calculate SHA256 of Argon2 hash (this is the signup hash)
      const sha256Hash = await calculateSHA256(argon2Hash);
      document.getElementById('login-sha256-hash').textContent = sha256Hash;

      // Step 4: Combine login session with signup hash
      const loginSessionHash = `${loginSession}:${username}:${sha256Hash}`;
      document.getElementById('login-session-hash').textContent = loginSessionHash;

      // Step 5: Calculate Argon2 hash of loginSessionHash
      const loginArgon2Hash = argon2_calc(loginSessionHash, getArgon2Config());
      document.getElementById('login-argon2-final').textContent = loginArgon2Hash;

      // Step 6: Calculate SHA256 of login Argon2 hash
      const loginSha256Hash = await calculateSHA256(loginArgon2Hash);
      document.getElementById('login-sha256-final').textContent = loginSha256Hash;

      // Create final JSON payload
      const jsonPayload = {
        "username": username,
        //"login_salt": signupSession,
        "login_salt": loginSession,
        "passwd": loginSha256Hash
      };

      document.getElementById('login-json').textContent = JSON.stringify(jsonPayload, null, 2);

      statusElement.textContent = "Login hash calculated successfully";
      statusElement.classList.remove('loading');
    } catch (error) {
      statusElement.textContent = "Error: " + error.message;
      statusElement.classList.remove('loading');
      statusElement.classList.add('error');
    }
  });
});
