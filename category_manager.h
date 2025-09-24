#ifndef CATEGORY_MANAGER_H
#define CATEGORY_MANAGER_H

#include <wx/wx.h>
#include "wxsqlite3/wxsqlite3.h"
#include <vector>
#include <string>

// Category data structure
struct Category {
    int id;
    wxString name;
    wxString color;
    wxString description;
    int userId;  // Owner of the category
};

class CategoryManager {
private:
    wxSQLite3Database* db;

public:
    CategoryManager(wxSQLite3Database* database);
    ~CategoryManager();
    
    // Setup the categories table
    bool InitializeCategoryTable();
    
    // Category management
    bool AddCategory(const wxString& name, const wxString& color, 
                    const wxString& description, int userId);
    bool UpdateCategory(int categoryId, const wxString& name, 
                       const wxString& color, const wxString& description);
    bool DeleteCategory(int categoryId);
    
    // Category retrieval
    std::vector<Category> GetAllCategories(int userId);
    Category* GetCategoryById(int categoryId);
};

#endif // CATEGORY_MANAGER_H
