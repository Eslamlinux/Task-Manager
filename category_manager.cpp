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

bool CategoryManager::AddCategory(const wxString& name, const wxString& color, 
                                const wxString& description, int userId) {
    try {
        wxSQLite3Statement stmt = db->PrepareStatement(
            "INSERT INTO categories (name, color, description, user_id) "
            "VALUES (?, ?, ?, ?)"
        );
        
        stmt.Bind(1, name);
        stmt.Bind(2, color);
        stmt.Bind(3, description);
        stmt.Bind(4, userId);
        
        stmt.ExecuteUpdate();
        return true;
    }
    catch (wxSQLite3Exception& e) {
        wxMessageBox(e.GetMessage(), "Database Error", wxOK | wxICON_ERROR);
        return false;
    }
}

bool CategoryManager::UpdateCategory(int categoryId, const wxString& name, 
                                   const wxString& color, const wxString& description) {
    try {
        wxSQLite3Statement stmt = db->PrepareStatement(
            "UPDATE categories SET name = ?, color = ?, description = ? WHERE id = ?"
        );
        
        stmt.Bind(1, name);
        stmt.Bind(2, color);
        stmt.Bind(3, description);
        stmt.Bind(4, categoryId);
        
        stmt.ExecuteUpdate();
        return true;
    }
    catch (wxSQLite3Exception& e) {
        wxMessageBox(e.GetMessage(), "Database Error", wxOK | wxICON_ERROR);
        return false;
    }
}

bool CategoryManager::DeleteCategory(int categoryId) {
  try {
      // Start a transaction
      db->ExecuteUpdate("BEGIN TRANSACTION");
      
      // First update any tasks with this category to have no category
      wxSQLite3Statement updateStmt = db->PrepareStatement(
          "UPDATE tasks SET category_id = NULL WHERE category_id = ?"
      );
      updateStmt.Bind(1, categoryId);
      updateStmt.ExecuteUpdate();
      
      // Then delete the category
      wxSQLite3Statement deleteStmt = db->PrepareStatement(
          "DELETE FROM categories WHERE id = ?"
      );
      deleteStmt.Bind(1, categoryId);
      deleteStmt.ExecuteUpdate();
      
      // Commit the transaction
      db->ExecuteUpdate("COMMIT");
      
      return true;
  }
  catch (wxSQLite3Exception& e) {
      // Rollback on error
      db->ExecuteUpdate("ROLLBACK");
      wxMessageBox(e.GetMessage(), "Database Error", wxOK | wxICON_ERROR);
      return false;
  }
}

std::vector<Category> CategoryManager::GetAllCategories(int userId) {
    std::vector<Category> categories;
    
    try {
        wxSQLite3Statement stmt = db->PrepareStatement(
            "SELECT id, name, color, description, user_id "
            "FROM categories WHERE user_id = ? OR user_id IS NULL "
            "ORDER BY name"
        );
        
        stmt.Bind(1, userId);
        wxSQLite3ResultSet resultSet = stmt.ExecuteQuery();
        
        while (resultSet.NextRow()) {
            Category category;
            category.id = resultSet.GetAsInt(0);
            category.name = resultSet.GetAsString(1);
            category.color = resultSet.GetAsString(2);
            category.description = resultSet.GetAsString(3);
            category.userId = resultSet.GetAsInt(4);
            
            categories.push_back(category);
        }
    }
    catch (wxSQLite3Exception& e) {
        wxMessageBox(e.GetMessage(), "Database Error", wxOK | wxICON_ERROR);
    }
    
    return categories;
}

