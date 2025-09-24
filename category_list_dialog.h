#ifndef CATEGORY_LIST_DIALOG_H
#define CATEGORY_LIST_DIALOG_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include "category_manager.h"
#include "user_manager.h"

class CategoryListDialog : public wxDialog {
private:
    CategoryManager* categoryManager;
    UserManager* userManager;
    wxListCtrl* categoryList;
    
    void OnAddButton(wxCommandEvent& event);
    void OnEditButton(wxCommandEvent& event);
    void OnDeleteButton(wxCommandEvent& event);
    void OnCloseButton(wxCommandEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);
    
    void LoadCategories();
    void EditCategory(int index);
    
public:
    CategoryListDialog(wxWindow* parent, CategoryManager* categoryManager, 
                      UserManager* userManager);
    virtual ~CategoryListDialog();
    
    wxDECLARE_EVENT_TABLE();
};

#endif // CATEGORY_LIST_DIALOG_H
