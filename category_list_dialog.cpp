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
    
    // Buttons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxButton* addButton = new wxButton(this, wxID_ADD, "Add");
    wxButton* editButton = new wxButton(this, wxID_EDIT, "Edit");
    wxButton* deleteButton = new wxButton(this, wxID_DELETE, "Delete");
    wxButton* closeButton = new wxButton(this, wxID_CLOSE, "Close");
    
    buttonSizer->Add(addButton, 0, wxRIGHT, 5);
    buttonSizer->Add(editButton, 0, wxRIGHT, 5);
    buttonSizer->Add(deleteButton, 0, wxRIGHT, 5);
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(closeButton, 0);
    
    mainSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, 10);
    
    SetSizer(mainSizer);
    
    // Initially disable edit and delete buttons
    editButton->Disable();
    deleteButton->Disable();
    
    // Load categories
    LoadCategories();
}

CategoryListDialog::~CategoryListDialog() {
}

void CategoryListDialog::LoadCategories() {
    categoryList->DeleteAllItems();
    
    std::vector<Category> categories = categoryManager->GetAllCategories(userManager->GetCurrentUser()->id);
    
    for (size_t i = 0; i < categories.size(); ++i) {
        const Category& category = categories[i];
        
        long itemIndex = categoryList->InsertItem(i, wxString::Format("%d", category.id));
        categoryList->SetItem(itemIndex, 1, category.name);
        categoryList->SetItem(itemIndex, 2, category.color);
        categoryList->SetItem(itemIndex, 3, category.description);
        
        // Store category ID as item data
        categoryList->SetItemData(itemIndex, category.id);
        
        // Set background color for the item based on category color
        wxColour color;
        color.Set(category.color);
        categoryList->SetItemBackgroundColour(itemIndex, color);
        
        // Set text color to ensure readability
        int brightness = (color.Red() * 299 + color.Green() * 587 + color.Blue() * 114) / 1000;
        if (brightness > 128) {
            categoryList->SetItemTextColour(itemIndex, *wxBLACK);
        } else {
            categoryList->SetItemTextColour(itemIndex, *wxWHITE);
        }
    }
}

void CategoryListDialog::EditCategory(int index) {
    if (index == -1) return;
    
    int categoryId = categoryList->GetItemData(index);
    Category* category = categoryManager->GetCategoryById(categoryId);
    
    if (category) {
        CategoryDialog dlg(this, categoryManager, userManager->GetCurrentUser()->id, category);
        if (dlg.ShowModal() == wxID_OK) {
            LoadCategories();
        }
        delete category;
    }
}

void CategoryListDialog::OnAddButton(wxCommandEvent& event) {
    CategoryDialog dlg(this, categoryManager, userManager->GetCurrentUser()->id);
    if (dlg.ShowModal() == wxID_OK) {
        LoadCategories();
    }
}

void CategoryListDialog::OnEditButton(wxCommandEvent& event) {
    EditCategory(categoryList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED));
}

void CategoryListDialog::OnDeleteButton(wxCommandEvent& event) {
    int index = categoryList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (index == -1) return;
    
    int categoryId = categoryList->GetItemData(index);
    wxString categoryName = categoryList->GetItemText(index, 1);
    
    wxMessageDialog confirmDlg(this, 
                             wxString::Format("Are you sure you want to delete the category '%s'?\n"
                                             "Tasks with this category will be set to 'No Category'.", 
                                             categoryName),
                             "Confirm Delete", wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
    
    if (confirmDlg.ShowModal() == wxID_YES) {
        if (categoryManager->DeleteCategory(categoryId)) {
            LoadCategories();
        }
    }
}

void CategoryListDialog::OnCloseButton(wxCommandEvent& event) {
    EndModal(wxID_OK);
}

void CategoryListDialog::OnListItemSelected(wxListEvent& event) {
    wxWindow* editButton = FindWindow(wxID_EDIT);
    wxWindow* deleteButton = FindWindow(wxID_DELETE);
    
    if (editButton && deleteButton) {
        editButton->Enable();
        deleteButton->Enable();
    }
}
