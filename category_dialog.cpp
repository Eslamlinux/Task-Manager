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
    
