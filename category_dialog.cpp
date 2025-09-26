#include "category_dialog.h"

wxBEGIN_EVENT_TABLE(CategoryDialog, wxDialog)
    EVT_BUTTON(wxID_SAVE, CategoryDialog::OnSaveButton)
    EVT_BUTTON(wxID_CANCEL, CategoryDialog::OnCancelButton)
wxEND_EVENT_TABLE()

CategoryDialog::CategoryDialog(wxWindow* parent, CategoryManager* categoryManager, 
                             int userId, Category* category)
    : wxDialog(parent, wxID_ANY, (category ? "Edit Category" : "Add Category"), 
              wxDefaultPosition, wxSize(400, 300),
              wxDEFAULT_DIALOG_STYLE | wxCENTER),
      categoryManager(categoryManager), userId(userId), 
      categoryId(category ? category->id : -1) {
    
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Form
    wxFlexGridSizer* formSizer = new wxFlexGridSizer(3, 2, 10, 10);
    formSizer->AddGrowableCol(1);
    
    // Name
    formSizer->Add(new wxStaticText(this, wxID_ANY, "Name:"), 0, wxALIGN_CENTER_VERTICAL);
    nameCtrl = new wxTextCtrl(this, wxID_ANY);
    formSizer->Add(nameCtrl, 0, wxEXPAND);
    
    // Color
    formSizer->Add(new wxStaticText(this, wxID_ANY, "Color:"), 0, wxALIGN_CENTER_VERTICAL);
    colorCtrl = new wxColourPickerCtrl(this, wxID_ANY);
    formSizer->Add(colorCtrl, 0, wxEXPAND);
    
    // Description
    formSizer->Add(new wxStaticText(this, wxID_ANY, "Description:"), 0, wxALIGN_CENTER_VERTICAL);
    descriptionCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, 
                                    wxSize(-1, 80), wxTE_MULTILINE);
    formSizer->Add(descriptionCtrl, 0, wxEXPAND);
    
    mainSizer->Add(formSizer, 0, wxALL | wxEXPAND, 20);
    
    // Buttons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxButton* saveButton = new wxButton(this, wxID_SAVE, "Save");
    wxButton* cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
    
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(saveButton, 0, wxRIGHT, 5);
    buttonSizer->Add(cancelButton, 0);
    
    mainSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, 20);
    
    SetSizer(mainSizer);
    Centre();
    
    // Set initial values if editing
    if (category) {
        nameCtrl->SetValue(category->name);
        
        // Parse color string to wxColour
        wxColour color;
        color.Set(category->color);
        colorCtrl->SetColour(color);
        
        descriptionCtrl->SetValue(category->description);
    }
    else {
        // Default color for new categories
        colorCtrl->SetColour(wxColour(0, 120, 215));
    }
    
    nameCtrl->SetFocus();
    saveButton->SetDefault();
}

CategoryDialog::~CategoryDialog() {
}

void CategoryDialog::OnSaveButton(wxCommandEvent& event) {
    wxString name = nameCtrl->GetValue().Trim();
    wxColour color = colorCtrl->GetColour();
    wxString colorStr = color.GetAsString(wxC2S_HTML_SYNTAX);
    wxString description = descriptionCtrl->GetValue().Trim();
    
    if (name.IsEmpty()) {
        wxMessageBox("Category name cannot be empty.", 
                    "Validation Error", wxOK | wxICON_ERROR, this);
        nameCtrl->SetFocus();
        return;
    }
    
    bool success = false;
    
    if (categoryId == -1) {
        // Add new category
        success = categoryManager->AddCategory(name, colorStr, description, userId);
    }
    else {
        // Update existing category
        success = categoryManager->UpdateCategory(categoryId, name, colorStr, description);
    }
    
    if (success) {
        EndModal(wxID_OK);
    }
}

void CategoryDialog::OnCancelButton(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}
