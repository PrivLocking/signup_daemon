// Global variables
let adminPermission = 0;
let adminVersion = 0;

// Permission bit flags
const PERMISSION = {
    VIEW_USERS: 1,
    APPROVE_USERS: 2,
    MODIFY_BASIC_LEVELS: 4,
    MODIFY_ADVANCED_LEVELS: 8,
    DELETE_USERS: 16,
    RESET_PASSWORDS: 32,
    VIEW_AUDIT_LOGS: 64,
    MANAGE_WHITELIST: 128,
    CREATE_ADMINS: 256,
    MODIFY_PERMISSIONS: 512,
    VIEW_STATISTICS: 1024,
    CONFIGURE_LIMITS: 2048
};

// Navigation items with their respective permission requirements
const NAV_ITEMS = [
    { title: 'User List', url: 'view_users.html', permission: PERMISSION.VIEW_USERS },
    { title: 'Approve Users', url: 'approve_users.html', permission: PERMISSION.APPROVE_USERS },
    { title: 'Modify Basic Levels', url: 'modify_basic_levels.html', permission: PERMISSION.MODIFY_BASIC_LEVELS },
    { title: 'Modify Advanced Levels', url: 'modify_advanced_levels.html', permission: PERMISSION.MODIFY_ADVANCED_LEVELS },
    { title: 'Delete/Suspend Users', url: 'delete_users.html', permission: PERMISSION.DELETE_USERS },
    { title: 'Reset Passwords', url: 'reset_passwords.html', permission: PERMISSION.RESET_PASSWORDS },
    { title: 'Audit Logs', url: 'view_audit_logs.html', permission: PERMISSION.VIEW_AUDIT_LOGS },
    { title: 'IP Whitelist', url: 'manage_whitelist.html', permission: PERMISSION.MANAGE_WHITELIST },
    { title: 'Create Admin', url: 'create_admins.html', permission: PERMISSION.CREATE_ADMINS },
    { title: 'Modify Permissions', url: 'modify_permissions.html', permission: PERMISSION.MODIFY_PERMISSIONS },
    { title: 'System Statistics', url: 'view_statistics.html', permission: PERMISSION.VIEW_STATISTICS },
    { title: 'Rate Limits', url: 'configure_limits.html', permission: PERMISSION.CONFIGURE_LIMITS }
];

// Utility functions
function showError(message) {
    const errorContainer = document.getElementById('errorContainer');
    const errorMessage = document.getElementById('errorMessage');
    
    errorMessage.textContent = message;
    errorContainer.style.display = 'block';
    
    // Hide loading status
    document.getElementById('loadingStatus').style.display = 'none';
}

function hideError() {
    document.getElementById('errorContainer').style.display = 'none';
}

function hasPermission(permissionFlag) {
    return (adminPermission & permissionFlag) === permissionFlag;
}

// API functions
async function fetchWithTimeout(url, options = {}, timeout = 5000) {
    const controller = new AbortController();
    const id = setTimeout(() => controller.abort(), timeout);
    
    options.signal = controller.signal;
    
    try {
        const response = await fetch(url, options);
        clearTimeout(id);
        return response;
    } catch (error) {
        clearTimeout(id);
        throw error;
    }
}

async function checkAdminPermissions() {
    try {
        const response = await fetchWithTimeout('permission_int');
        
        if (!response.ok) {
            throw new Error(`Failed to fetch permissions: ${response.status} ${response.statusText}`);
        }
        
        const data = await response.json();
        
        // Validate version first
        if (data.admin_ver !== 1) {
            throw new Error(`Unsupported admin version: ${data.admin_ver}`);
        }
        
        // Set global permissions
        adminPermission = data.admin_permission;
        adminVersion = data.admin_ver;
        
        return data;
    } catch (error) {
        console.error('Permission check failed:', error);
        throw error;
    }
}

// Dashboard initialization functions
function initializeDashboard(permissionData) {
    // Update admin info display
    document.getElementById('loadingStatus').textContent = 
        `Admin Level: ${permissionData.admin_permission} (v${permissionData.admin_ver})`;
    
    // Set up navigation based on permissions
    setupNavigation();
    
    // Show navigation
    document.getElementById('adminNav').style.display = 'block';
    
    // If we're on the index page, show a dashboard overview
    if (window.location.pathname.endsWith('index.html') || 
        window.location.pathname.endsWith('/')) {
        showDashboardOverview();
    }
}

function setupNavigation() {
    const navList = document.getElementById('navList');
    navList.innerHTML = '';
    
    NAV_ITEMS.forEach(item => {
        if (hasPermission(item.permission)) {
            const li = document.createElement('li');
            const a = document.createElement('a');
            
            a.href = item.url;
            a.textContent = item.title;
            
            // Highlight current page if applicable
            if (window.location.href.includes(item.url)) {
                a.classList.add('active');
            }
            
            li.appendChild(a);
            navList.appendChild(li);
        }
    });
    
    // Always add a link back to the dashboard
    const dashboardLi = document.createElement('li');
    const dashboardA = document.createElement('a');
    dashboardA.href = 'index.html';
    dashboardA.textContent = 'Dashboard';
    
    if (window.location.href.endsWith('index.html') || 
        window.location.pathname.endsWith('/')) {
        dashboardA.classList.add('active');
    }
    
    dashboardLi.appendChild(dashboardA);
    navList.appendChild(dashboardLi);
}

function showDashboardOverview() {
    const mainContent = document.getElementById('mainContent');
    
    // Create dashboard content
    let dashboardHTML = `
        <h2>Admin Dashboard</h2>
        <p>Welcome to the User Admin System. Please select a function from the navigation menu.</p>
        
        <div class="card">
            <div class="card-header">Available Modules</div>
            <ul>
    `;
    
    // List available modules based on permissions
    NAV_ITEMS.forEach(item => {
        if (hasPermission(item.permission)) {
            dashboardHTML += `<li><a href="${item.url}">${item.title}</a></li>`;
        }
    });
    
    dashboardHTML += `
            </ul>
        </div>
    `;
    
    mainContent.innerHTML = dashboardHTML;
}

function handlePermissionError(error) {
    showError(`Permission error: ${error.message}. Please contact your system administrator.`);
    
    // Hide navigation
    document.getElementById('adminNav').style.display = 'none';
    
    // Clear main content
    const mainContent = document.getElementById('mainContent');
    mainContent.innerHTML = `
        <div class="card">
            <div class="card-header">Access Denied</div>
            <p>You don't have permission to access this system.</p>
            <p>Error details: ${error.message}</p>
        </div>
    `;
}

// Module permission check
function checkModulePermission(requiredPermission) {
    // This function should be called at the start of each module page
    if (!hasPermission(requiredPermission)) {
        window.location.href = './';
        return false;
    }
    return true;
}

// Data fetching utilities for modules
async function fetchModuleData(endpoint, params = {}) {
    try {
        // Build URL with query params
        const url = new URL(endpoint, window.location.origin);
        Object.keys(params).forEach(key => {
            if (params[key] !== undefined && params[key] !== null) {
                url.searchParams.append(key, params[key]);
            }
        });
        
        const response = await fetchWithTimeout(url);
        
        if (!response.ok) {
            throw new Error(`API error: ${response.status} ${response.statusText}`);
        }
        
        return await response.json();
    } catch (error) {
        console.error(`Failed to fetch data from ${endpoint}:`, error);
        throw error;
    }
}

async function submitModuleData(endpoint, data) {
    try {
        const response = await fetchWithTimeout(endpoint, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(data)
        });
        
        if (!response.ok) {
            const errorText = await response.text();
            throw new Error(`API error: ${response.status} ${response.statusText} - ${errorText}`);
        }
        
        return await response.json();
    } catch (error) {
        console.error(`Failed to submit data to ${endpoint}:`, error);
        throw error;
    }
}

// Pagination utility
function createPagination(currentPage, totalPages, onPageChange) {
    const pagination = document.createElement('div');
    pagination.className = 'pagination';
    
    // Previous button
    const prevButton = document.createElement('button');
    prevButton.textContent = '← Previous';
    prevButton.disabled = currentPage <= 1;
    prevButton.onclick = () => onPageChange(currentPage - 1);
    pagination.appendChild(prevButton);
    
    // Page numbers
    const maxButtons = 5;
    const startPage = Math.max(1, currentPage - Math.floor(maxButtons / 2));
    const endPage = Math.min(totalPages, startPage + maxButtons - 1);
    
    for (let i = startPage; i <= endPage; i++) {
        const pageButton = document.createElement('button');
        pageButton.textContent = i;
        pageButton.className = i === currentPage ? 'active' : '';
        pageButton.onclick = () => onPageChange(i);
        pagination.appendChild(pageButton);
    }
    
    // Next button
    const nextButton = document.createElement('button');
    nextButton.textContent = 'Next →';
    nextButton.disabled = currentPage >= totalPages;
    nextButton.onclick = () => onPageChange(currentPage + 1);
    pagination.appendChild(nextButton);
    
    return pagination;
}
