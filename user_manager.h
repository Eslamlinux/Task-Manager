#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <wx/wx.h>
#include "wxsqlite3/wxsqlite3.h"
#include <memory>
#include <string>

// User data structure
struct User {
    int id;
    wxString username;
    wxString email;
    wxString passwordHash;
    wxString fullName;
    wxString createdDate;
    bool isAdmin;
};

class UserManager {
private:
    wxSQLite3Database* db;
    
    // Current user session
    std::unique_ptr<User> currentUser;
    
    // Helper method to hash password
    wxString HashPassword(const wxString& password, const wxString& salt);
    wxString GenerateSalt();

public:
    UserManager(wxSQLite3Database* database);
    ~UserManager();
    
    // Setup the users table
    bool InitializeUserTable();
    
    // User authentication
    bool Login(const wxString& username, const wxString& password);
    void Logout();
    
    // User management
    bool RegisterUser(const wxString& username, const wxString& email, 
                     const wxString& password, const wxString& fullName, 
                     bool isAdmin = false);
    bool UpdateUser(int userId, const wxString& email, 
                   const wxString& fullName);
    bool ChangePassword(int userId, const wxString& oldPassword, 
                       const wxString& newPassword);
    bool DeleteUser(int userId);
    
    // User retrieval
    std::vector<User> GetAllUsers();
    User* GetUserById(int userId);
    User* GetUserByUsername(const wxString& username);
    
    // Session management
    bool IsLoggedIn() const;
    User* GetCurrentUser() const;
    bool IsAdmin() const;
    
    // Helper for creating default admin user if no users exist
    bool CreateDefaultAdminIfNeeded();
};

#endif // USER_MANAGER_H
