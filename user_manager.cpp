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
      
   
