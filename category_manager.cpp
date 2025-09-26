#include "category_manager.h"

CategoryManager::CategoryManager(wxSQLite3Database* database) : db(database) {
    InitializeCategoryTable();
}

CategoryManager::~CategoryManager() {
    // Nothing to do here, the database is owned elsewhere
}

bool CategoryManager::InitializeCategoryTable() {
    try {
        db->ExecuteUpdate(
            "CREATE TABLE IF NOT EXISTS categories ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "name TEXT NOT NULL, "
            "color TEXT, "
            "description TEXT, "
            "user_id INTEGER, "
            "FOREIGN KEY(user_id) REFERENCES users(id))"
        );
        
        return true;
    }
    catch (wxSQLite3Exception& e) {
        wxMessageBox(e.GetMessage(), "Database Error", wxOK | wxICON_ERROR);
        return false;
    }
}

