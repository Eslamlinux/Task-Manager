#include "category_list_dialog.h"
#include "category_dialog.h"

wxBEGIN_EVENT_TABLE(CategoryListDialog, wxDialog)
    EVT_BUTTON(wxID_ADD, CategoryListDialog::OnAddButton)
    EVT_BUTTON(wxID_EDIT, CategoryListDialog::OnEditButton)
    EVT_BUTTON(wxID_DELETE, CategoryListDialog::OnDeleteButton)
    EVT_BUTTON(wxID_CLOSE, CategoryListDialog::OnCloseButton)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, CategoryListDialog::OnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, CategoryListDialog::OnListItemActivated)
wxEND_EVENT_TABLE()

CategoryListDialog::CategoryListDialog(wxWindow* parent, CategoryManager* categoryManager, 
                                     UserManager* userManager)
    : wxDialog(parent, wxID_ANY, "Manage Categories", wxDefaultPosition, wxSize(500, 400),
              wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      categoryManager(categoryManager), userManager(userManager) {
    
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Category list
    categoryList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                 wxLC_REPORT | wxLC_SINGLE_SEL);
    
    categoryList->InsertColumn(0, "ID", wxLIST_FORMAT_LEFT, 50);
    categoryList->InsertColumn(1, "Name", wxLIST_FORMAT_LEFT, 150);
    categoryList->InsertColumn(2, "Color", wxLIST_FORMAT_LEFT, 100);
    categoryList->InsertColumn(3, "Description", wxLIST_FORMAT_LEFT, 200);
    
    mainSizer->Add(categoryList, 1, wxALL | wxEXPAND, 10);
    
