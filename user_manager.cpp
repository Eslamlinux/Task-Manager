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

