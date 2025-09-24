#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/grid.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/clipbrd.h>
#include <wx/filedlg.h>
#include <wx/textfile.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>
#include <wx/chartype.h>
#include <wx/colordlg.h>
#include <wx/combobox.h>
#include "wxsqlite3/wxsqlite3.h"
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <iostream>

#include "user_manager.h"
#include "login_dialog.h"
#include "category_manager.h"
#include "profile_dialog.h"
#include "category_list_dialog.h"

// Enhanced Task data structure
struct Task {
  int id;
  wxString title;
  wxString description;
  wxString dueDate;
  int priority;
  bool completed;
  int categoryId;
  wxString categoryName;
  wxString categoryColor;
  int userId; // Owner of the task
};

// Enhanced Database Manager class
class DatabaseManager {
private:
  wxSQLite3Database db;
  bool isConnected;

public:
  DatabaseManager() : isConnected(false) {}

  bool Connect(const wxString& dbPath) {
    try {
        std::cout << "Attempting to open database: " << dbPath.ToStdString() << std::endl;
        
        // Check if the database file exists
        bool fileExists = wxFileExists(dbPath);
        std::cout << "Database file exists: " << (fileExists ? "yes" : "no") << std::endl;
        
        // Open the database using wxSQLite3Database's method
        db.Open(dbPath);
        isConnected = true;
        std::cout << "Database connected successfully" << std::endl;
        
        // Enable foreign keys
        db.ExecuteUpdate("PRAGMA foreign_keys = ON");
        
        // Create tables if they don't exist
        std::cout << "Creating tables if they don't exist" << std::endl;
        db.ExecuteUpdate(
            "CREATE TABLE IF NOT EXISTS tasks ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "title TEXT NOT NULL, "
            "description TEXT, "
            "due_date TEXT, "
            "priority INTEGER, "
            "completed INTEGER DEFAULT 0, "
            "category_id INTEGER, "
            "user_id INTEGER, "
            "FOREIGN KEY(category_id) REFERENCES categories(id), "
            "FOREIGN KEY(user_id) REFERENCES users(id))"
        );
        std::cout << "Tables created successfully" << std::endl;
        
        return true;
