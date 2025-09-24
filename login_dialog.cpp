#include "login_dialog.h"
#include "register_dialog.h"

wxBEGIN_EVENT_TABLE(LoginDialog, wxDialog)
    EVT_BUTTON(wxID_OK, LoginDialog::OnLoginButton)
    EVT_BUTTON(wxID_CANCEL, LoginDialog::OnCancelButton)
    EVT_BUTTON(wxID_HELP, LoginDialog::OnRegisterButton)
wxEND_EVENT_TABLE()

LoginDialog::LoginDialog(wxWindow* parent, UserManager* userManager)
    : wxDialog(parent, wxID_ANY, "Login", wxDefaultPosition, wxSize(350, 250),
              wxDEFAULT_DIALOG_STYLE | wxCENTER),
      userManager(userManager) {
    
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Logo or app title
    wxStaticText* titleText = new wxStaticText(this, wxID_ANY, "Task Manager");
    wxFont titleFont = titleText->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 6);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleText->SetFont(titleFont);
    
    mainSizer->Add(titleText, 0, wxALL | wxALIGN_CENTER, 20);
    
    // Input fields
    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(2, 2, 10, 10);
    gridSizer->AddGrowableCol(1);
    
    // Username
    gridSizer->Add(new wxStaticText(this, wxID_ANY, "Username:"), 0, wxALIGN_CENTER_VERTICAL);
    usernameCtrl = new wxTextCtrl(this, wxID_ANY);
    gridSizer->Add(usernameCtrl, 0, wxEXPAND);
    
    // Password
    gridSizer->Add(new wxStaticText(this, wxID_ANY, "Password:"), 0, wxALIGN_CENTER_VERTICAL);
    passwordCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, 
                                 wxDefaultSize, wxTE_PASSWORD);
    gridSizer->Add(passwordCtrl, 0, wxEXPAND);
    
    mainSizer->Add(gridSizer, 0, wxALL | wxEXPAND, 20);
    
    // Buttons
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    
    loginButton = new wxButton(this, wxID_OK, "Login Now");
    cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
    registerButton = new wxButton(this, wxID_HELP, "Register");
    
    buttonSizer->Add(registerButton, 0, wxRIGHT, 5);
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(loginButton, 0, wxRIGHT, 5);
    buttonSizer->Add(cancelButton, 0);
    
    mainSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, 20);
    
    SetSizer(mainSizer);
    Centre();
    
    usernameCtrl->SetFocus();
    loginButton->SetDefault();
    
    // Set initial values for demo purposes
    // usernameCtrl->SetValue("EslamLinux");
    // passwordCtrl->SetValue("eslam123");

    // Comment these out or remove them for a production application
    // usernameCtrl->SetValue("EslamLinux");
    // passwordCtrl->SetValue("eslam123");
}

LoginDialog::~LoginDialog() {
}

bool LoginDialog::PerformLogin() {
    wxString username = usernameCtrl->GetValue();
    wxString password = passwordCtrl->GetValue();
    
    if (username.IsEmpty() || password.IsEmpty()) {
        wxMessageBox("Please enter both username and password.", 
                    "Login Error", wxOK | wxICON_ERROR, this);
        return false;
    }
    
    if (!userManager->Login(username, password)) {
        wxMessageBox("Invalid username or password.", 
                    "Login Failed", wxOK | wxICON_ERROR, this);
        return false;
    }
    
    return true;
}

void LoginDialog::OnLoginButton(wxCommandEvent& event) {
    if (PerformLogin()) {
        EndModal(wxID_OK);
    }
}

void LoginDialog::OnRegisterButton(wxCommandEvent& event) {
    RegisterDialog registerDlg(this, userManager);
    if (registerDlg.ShowModal() == wxID_OK) {
        // Auto-fill the username if registration was successful
        usernameCtrl->SetValue(registerDlg.GetUsername());
        passwordCtrl->SetValue("");
        passwordCtrl->SetFocus();
    }
}

void LoginDialog::OnCancelButton(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}
