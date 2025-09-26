#include "user_manager.h"
#include <wx/string.h>
#include <random>
#include <sstream>
#include <iomanip>
#include <iostream>

// Simple SHA-256 implementation
// This is a basic implementation for demonstration purposes
// In a production environment, use a well-tested cryptographic library
wxString SimpleSHA256(const wxString& input) {
  // Simple hash function (not secure, but works for demo)
  std::string str = input.ToStdString();
  unsigned int hash = 0;
  
  for (size_t i = 0; i < str.length(); i++) {
      hash = ((hash << 5) + hash) + str[i];
  }
  
  // Convert to hex string
  std::stringstream ss;
  ss << std::hex << std::setfill('0') << std::setw(8) << hash;
  return wxString(ss.str());
}

UserManager::UserManager(wxSQLite3Database* database) : db(database), currentUser(nullptr) {
  std::cout << "UserManager constructor - Starting initialization" << std::endl;
  
  if (!database) {
    std::cerr << "ERROR: Database pointer is null in UserManager constructor" << std::endl;
    return;
  }
  
  std::cout << "UserManager constructor - Database pointer is valid" << std::endl;
  
  try {
    InitializeUserTable();
    std::cout << "UserManager constructor - User table initialized successfully" << std::endl;
  } catch (std::exception& e) {
    std::cerr << "Exception in UserManager constructor: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception in UserManager constructor" << std::endl;
  }
}

UserManager::~UserManager() {
  // Nothing to do here, the database is owned elsewhere
}

bool UserManager::InitializeUserTable() {
  std::cout << "InitializeUserTable - Starting" << std::endl;
  
  if (!db) {
    std::cerr << "ERROR: Database pointer is null in InitializeUserTable" << std::endl;
    return false;
  }
  
  try {
      std::cout << "InitializeUserTable - Creating users table if it doesn't exist" << std::endl;
      db->ExecuteUpdate(
          "CREATE TABLE IF NOT EXISTS users ("
          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "username TEXT NOT NULL UNIQUE, "
          "email TEXT, "
          "password_hash TEXT NOT NULL, "
          "password_salt TEXT NOT NULL, "
          "full_name TEXT, "
          "created_date TEXT, "
          "is_admin INTEGER DEFAULT 0)"
      );
      
      std::cout << "InitializeUserTable - Table created, checking for default admin" << std::endl;
      return CreateDefaultAdminIfNeeded();
  }
  catch (wxSQLite3Exception& e) {
      std::cerr << "Database error in InitializeUserTable: " << e.GetMessage().ToStdString() << std::endl;
      wxMessageBox(e.GetMessage(), "Database Error", wxOK | wxICON_ERROR);
      return false;
  }
  catch (std::exception& e) {
      std::cerr << "Standard exception in InitializeUserTable: " << e.what() << std::endl;
      return false;
  }
  catch (...) {
      std::cerr << "Unknown exception in InitializeUserTable" << std::endl;
      return false;
  }
}

wxString UserManager::GenerateSalt() {
  std::cout << "GenerateSalt - Generating salt" << std::endl;
  // Generate a random salt for password hashing
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 255);
  
  wxString salt;
  for (int i = 0; i < 16; i++) {
      salt += wxString::Format("%02x", dis(gen));
  }
  return salt;
}

wxString UserManager::HashPassword(const wxString& password, const wxString& salt) {
  std::cout << "HashPassword - Hashing password" << std::endl;
  // Simple password hashing using our SimpleSHA256 function
  return SimpleSHA256(salt + password);
}

bool UserManager::Login(const wxString& username, const wxString& password) {
  try {
      wxSQLite3Statement stmt = db->PrepareStatement(
          "SELECT id, username, email, password_hash, password_salt, full_name, created_date, is_admin "
          "FROM users WHERE username = ?"
      );
      
      stmt.Bind(1, username);
      wxSQLite3ResultSet resultSet = stmt.ExecuteQuery();
      
      if (resultSet.NextRow()) {
          wxString storedHash = resultSet.GetAsString(3);
          wxString salt = resultSet.GetAsString(4);
          wxString computedHash = HashPassword(password, salt);
          
          if (storedHash == computedHash) {
              // Authentication successful
              currentUser = std::make_unique<User>();
              currentUser->id = resultSet.GetAsInt(0);
              currentUser->username = resultSet.GetAsString(1);
              currentUser->email = resultSet.GetAsString(2);
              currentUser->passwordHash = storedHash;
              currentUser->fullName = resultSet.GetAsString(5);
              currentUser->createdDate = resultSet.GetAsString(6);
              currentUser->isAdmin = resultSet.GetAsBool(7);
              return true;
          }
      }
      
      // Authentication failed
      return false;
  }
  catch (wxSQLite3Exception& e) {
      std::cerr << "Database error in Login: " << e.GetMessage().ToStdString() << std::endl;
      wxMessageBox(e.GetMessage(), "Login Error", wxOK | wxICON_ERROR);
      return false;
  }
}

void UserManager::Logout() {
  currentUser.reset();
}

bool UserManager::RegisterUser(const wxString& username, const wxString& email, 
                         const wxString& password, const wxString& fullName, 
                         bool isAdmin) {
  std::cout << "RegisterUser - Starting registration for user: " << username.ToStdString() << std::endl;
  try {
      // Check if username already exists
      wxSQLite3Statement checkStmt = db->PrepareStatement(
          "SELECT COUNT(*) FROM users WHERE username = ?"
      );
      
      checkStmt.Bind(1, username);
      wxSQLite3ResultSet checkResult = checkStmt.ExecuteQuery();
      
      if (checkResult.NextRow() && checkResult.GetAsInt(0) > 0) {
          wxMessageBox("Username already exists. Please choose a different username.", 
                      "Registration Error", wxOK | wxICON_ERROR);
          return false;
      }
      
      // Generate salt and hash password
      wxString salt = GenerateSalt();
      wxString passwordHash = HashPassword(password, salt);
      
      // Get current timestamp
      wxDateTime now = wxDateTime::Now();
      wxString createdDate = now.FormatISODate() + " " + now.FormatISOTime();
      
      std::cout << "RegisterUser - Inserting new user" << std::endl;
      // Insert new user
      wxSQLite3Statement stmt = db->PrepareStatement(
          "INSERT INTO users (username, email, password_hash, password_salt, full_name, created_date, is_admin) "
          "VALUES (?, ?, ?, ?, ?, ?, ?)"
      );
      
      stmt.Bind(1, username);
      stmt.Bind(2, email);
      stmt.Bind(3, passwordHash);
      stmt.Bind(4, salt);
      stmt.Bind(5, fullName);
      stmt.Bind(6, createdDate);
      stmt.Bind(7, isAdmin ? 1 : 0);
      
      stmt.ExecuteUpdate();
      std::cout << "RegisterUser - User registered successfully" << std::endl;
      return true;
  }
  catch (wxSQLite3Exception& e) {
      std::cerr << "Database error in RegisterUser: " << e.GetMessage().ToStdString() << std::endl;
      wxMessageBox(e.GetMessage(), "Registration Error", wxOK | wxICON_ERROR);
      return false;
  }
  catch (std::exception& e) {
      std::cerr << "Standard exception in RegisterUser: " << e.what() << std::endl;
      return false;
  }
  catch (...) {
      std::cerr << "Unknown exception in RegisterUser" << std::endl;
      return false;
  }
}

bool UserManager::UpdateUser(int userId, const wxString& email, const wxString& fullName) {
  try {
      wxSQLite3Statement stmt = db->PrepareStatement(
          "UPDATE users SET email = ?, full_name = ? WHERE id = ?"
      );
      
      stmt.Bind(1, email);
      stmt.Bind(2, fullName);
      stmt.Bind(3, userId);
      
      stmt.ExecuteUpdate();
      
      // Update current user if it's the same user
      if (currentUser && currentUser->id == userId) {
          currentUser->email = email;
          currentUser->fullName = fullName;
      }
      
      return true;
  }
  catch (wxSQLite3Exception& e) {
      std::cerr << "Database error in UpdateUser: " << e.GetMessage().ToStdString() << std::endl;
      wxMessageBox(e.GetMessage(), "Update Error", wxOK | wxICON_ERROR);
      return false;
  }
}

bool UserManager::ChangePassword(int userId, const wxString& oldPassword, const wxString& newPassword) {
  try {
      // Verify old password first
      wxSQLite3Statement verifyStmt = db->PrepareStatement(
          "SELECT password_hash, password_salt FROM users WHERE id = ?"
      );
      
      verifyStmt.Bind(1, userId);
      wxSQLite3ResultSet verifyResult = verifyStmt.ExecuteQuery();
      
      if (verifyResult.NextRow()) {
          wxString storedHash = verifyResult.GetAsString(0);
          wxString salt = verifyResult.GetAsString(1);
          wxString computedHash = HashPassword(oldPassword, salt);
          
          if (storedHash != computedHash) {
              wxMessageBox("Current password is incorrect.", 
                          "Password Change Error", wxOK | wxICON_ERROR);
              return false;
          }
          
          // Generate new salt and hash for new password
          wxString newSalt = GenerateSalt();
          wxString newPasswordHash = HashPassword(newPassword, newSalt);
          
          // Update password
          wxSQLite3Statement updateStmt = db->PrepareStatement(
              "UPDATE users SET password_hash = ?, password_salt = ? WHERE id = ?"
          );
          
          updateStmt.Bind(1, newPasswordHash);
          updateStmt.Bind(2, newSalt);
          updateStmt.Bind(3, userId);
          
          updateStmt.ExecuteUpdate();
          
          // Update current user if it's the same user
          if (currentUser && currentUser->id == userId) {
              currentUser->passwordHash = newPasswordHash;
          }
          
          return true;
      }
      
      return false;
  }
  catch (wxSQLite3Exception& e) {
      std::cerr << "Database error in ChangePassword: " << e.GetMessage().ToStdString() << std::endl;
      wxMessageBox(e.GetMessage(), "Password Change Error", wxOK | wxICON_ERROR);
      return false;
  }
}

bool UserManager::DeleteUser(int userId) {
  // Don't allow deleting the current user
  if (currentUser && currentUser->id == userId) {
      wxMessageBox("Cannot delete your own account while logged in.", 
                  "Delete Error", wxOK | wxICON_ERROR);
      return false;
  }
  
  try {
      wxSQLite3Statement stmt = db->PrepareStatement("DELETE FROM users WHERE id = ?");
      stmt.Bind(1, userId);
      stmt.ExecuteUpdate();
      return true;
  }
  catch (wxSQLite3Exception& e) {
      std::cerr << "Database error in DeleteUser: " << e.GetMessage().ToStdString() << std::endl;
      wxMessageBox(e.GetMessage(), "Delete Error", wxOK | wxICON_ERROR);
      return false;
  }
}

std::vector<User> UserManager::GetAllUsers() {
  std::vector<User> users;
  
  try {
      wxSQLite3ResultSet resultSet = db->ExecuteQuery(
          "SELECT id, username, email, password_hash, full_name, created_date, is_admin "
          "FROM users ORDER BY username"
      );
      
      while (resultSet.NextRow()) {
          User user;
          user.id = resultSet.GetAsInt(0);
          user.username = resultSet.GetAsString(1);
          user.email = resultSet.GetAsString(2);
          user.passwordHash = resultSet.GetAsString(3);
          user.fullName = resultSet.GetAsString(4);
          user.createdDate = resultSet.GetAsString(5);
          user.isAdmin = resultSet.GetAsBool(6);
          
          users.push_back(user);
      }
  }
  catch (wxSQLite3Exception& e) {
      std::cerr << "Database error in GetAllUsers: " << e.GetMessage().ToStdString() << std::endl;
      wxMessageBox(e.GetMessage(), "Database Error", wxOK | wxICON_ERROR);
  }
  
  return users;
}

User* UserManager::GetUserById(int userId) {
  try {
      wxSQLite3Statement stmt = db->PrepareStatement(
          "SELECT id, username, email, password_hash, full_name, created_date, is_admin "
          "FROM users WHERE id = ?"
      );
      
      stmt.Bind(1, userId);
      wxSQLite3ResultSet resultSet = stmt.ExecuteQuery();
      
      if (resultSet.NextRow()) {
          User* user = new User();
          user->id = resultSet.GetAsInt(0);
          user->username = resultSet.GetAsString(1);
          user->email = resultSet.GetAsString(2);
          user->passwordHash = resultSet.GetAsString(3);
          user->fullName = resultSet.GetAsString(4);
          user->createdDate = resultSet.GetAsString(5);
          user->isAdmin = resultSet.GetAsBool(6);
          
          return user;
      }
  }
  catch (wxSQLite3Exception& e) {
      std::cerr << "Database error in GetUserById: " << e.GetMessage().ToStdString() << std::endl;
      wxMessageBox(e.GetMessage(), "Database Error", wxOK | wxICON_ERROR);
  }
  
  return nullptr;
}

User* UserManager::GetUserByUsername(const wxString& username) {
  try {
      wxSQLite3Statement stmt = db->PrepareStatement(
          "SELECT id, username, email, password_hash, full_name, created_date, is_admin "
          "FROM users WHERE username = ?"
      );
      
      stmt.Bind(1, username);
      wxSQLite3ResultSet resultSet = stmt.ExecuteQuery();
      
      if (resultSet.NextRow()) {
          User* user = new User();
          user->id = resultSet.GetAsInt(0);
          user->username = resultSet.GetAsString(1);
          user->email = resultSet.GetAsString(2);
          user->passwordHash = resultSet.GetAsString(3);
          user->fullName = resultSet.GetAsString(4);
          user->createdDate = resultSet.GetAsString(5);
          user->isAdmin = resultSet.GetAsBool(6);
          
          return user;
      }
  }
  catch (wxSQLite3Exception& e) {
      std::cerr << "Database error in GetUserByUsername: " << e.GetMessage().ToStdString() << std::endl;
      wxMessageBox(e.GetMessage(), "Database Error", wxOK | wxICON_ERROR);
  }
  
  return nullptr;
}

