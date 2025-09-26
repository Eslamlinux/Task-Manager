#include "register_dialog.h"

wxBEGIN_EVENT_TABLE(RegisterDialog, wxDialog)
    EVT_BUTTON(wxID_OK, RegisterDialog::OnRegisterButton)
    EVT_BUTTON(wxID_CANCEL, RegisterDialog::OnCancelButton)
wxEND_EVENT_TABLE()

RegisterDialog::RegisterDialog(wxWindow* parent, UserManager* userManager)
    : wxDialog(parent, wxID_ANY, "Register New User", wxDefaultPosition, wxSize(400, 350),
              wxDEFAULT_DIALOG_STYLE | wxCENTER),
      userManager(userManager) {
    
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Title
    wxStaticText* titleText = new wxStaticText(this, wxID_ANY, "Create New Account");
    wxFont titleFont = titleText->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 2);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleText->SetFont(titleFont);
    
    mainSizer->Add(titleText, 0, wxALL | wxALIGN_CENTER, 15);
    
    // Form
    wxFlexGridSizer* formSizer = new wxFlexGridSizer(5, 2, 10, 10);
    formSizer->AddGrowableCol(1);
    
    // Username
    formSizer->Add(new wxStaticText(this, wxID_ANY, "Username:"), 0, wxALIGN_CENTER_VERTICAL);
    usernameCtrl = new wxTextCtrl(this, wxID_ANY);
    formSizer->Add(usernameCtrl, 0, wxEXPAND);
    
    // Full Name
    formSizer->Add(new wxStaticText(this, wxID_ANY, "Full Name:"), 0, wxALIGN_CENTER_VERTICAL);
    fullNameCtrl = new wxTextCtrl(this, wxID_ANY);
    formSizer->Add(fullNameCtrl, 0, wxEXPAND);
    
    // Email
    formSizer->Add(new wxStaticText(this, wxID_ANY, "Email:"), 0, wxALIGN_CENTER_VERTICAL);
    emailCtrl = new wxTextCtrl(this, wxID_ANY);
    formSizer->Add(emailCtrl, 0, wxEXPAND);
    
    // Password
    formSizer->Add(new wxStaticText(this, wxID_ANY, "Password:"), 0, wxALIGN_CENTER_VERTICAL);
    passwordCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, 
                                 wxDefaultSize, wxTE_PASSWORD);
    formSizer->Add(passwordCtrl, 0, wxEXPAND);
    
