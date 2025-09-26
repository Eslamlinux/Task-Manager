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
    
    // Confirm Password
    formSizer->Add(new wxStaticText(this, wxID_ANY, "Confirm Password:"), 0, wxALIGN_CENTER_VERTICAL);
    confirmPasswordCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, 
                                        wxDefaultSize, wxTE_PASSWORD);
    formSizer->Add(confirmPasswordCtrl, 0, wxEXPAND);
    
    mainSizer->Add(formSizer, 0, wxALL | wxEXPAND, 15);
    
    // Buttons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxButton* registerButton = new wxButton(this, wxID_OK, "Register");
    wxButton* cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
    
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(registerButton, 0, wxRIGHT, 5);
    buttonSizer->Add(cancelButton, 0);
    
    mainSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, 15);
    
    SetSizer(mainSizer);
    Centre();
    
    usernameCtrl->SetFocus();
    registerButton->SetDefault();
}

RegisterDialog::~RegisterDialog() {
}

void RegisterDialog::OnRegisterButton(wxCommandEvent& event) {
    // Validate inputs
    wxString username = usernameCtrl->GetValue().Trim();
    wxString fullName = fullNameCtrl->GetValue().Trim();
    wxString email = emailCtrl->GetValue().Trim();
    wxString password = passwordCtrl->GetValue();
    wxString confirmPassword = confirmPasswordCtrl->GetValue();
    
    if (username.IsEmpty()) {
        wxMessageBox("Username cannot be empty.", "Validation Error", wxOK | wxICON_ERROR, this);
        usernameCtrl->SetFocus();
        return;
    }
    
    if (fullName.IsEmpty()) {
        wxMessageBox("Full name cannot be empty.", "Validation Error", wxOK | wxICON_ERROR, this);
        fullNameCtrl->SetFocus();
        return;
    }
    
    if (email.IsEmpty()) {
        wxMessageBox("Email cannot be empty.", "Validation Error", wxOK | wxICON_ERROR, this);
        emailCtrl->SetFocus();
        return;
    }
    
    if (password.IsEmpty()) {
        wxMessageBox("Password cannot be empty.", "Validation Error", wxOK | wxICON_ERROR, this);
        passwordCtrl->SetFocus();
        return;
    }
    
    if (password != confirmPassword) {
        wxMessageBox("Passwords do not match.", "Validation Error", wxOK | wxICON_ERROR, this);
        confirmPasswordCtrl->SetFocus();
        return;
    }
    
    // Attempt to register user
    if (userManager->RegisterUser(username, email, password, fullName)) {
        wxMessageBox("Registration successful! You can now log in.", 
                    "Registration Complete", wxOK | wxICON_INFORMATION, this);
        this->username = username;
        EndModal(wxID_OK);
    }
}

void RegisterDialog::OnCancelButton(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}
