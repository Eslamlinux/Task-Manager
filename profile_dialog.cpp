#include "profile_dialog.h"

wxBEGIN_EVENT_TABLE(ProfileDialog, wxDialog)
    EVT_BUTTON(wxID_SAVE, ProfileDialog::OnSaveProfileButton)
    EVT_BUTTON(wxID_APPLY, ProfileDialog::OnChangePasswordButton)
    EVT_BUTTON(wxID_CANCEL, ProfileDialog::OnCancelButton)
wxEND_EVENT_TABLE()

ProfileDialog::ProfileDialog(wxWindow* parent, UserManager* userManager)
    : wxDialog(parent, wxID_ANY, "User Profile", wxDefaultPosition, wxSize(450, 450),
              wxDEFAULT_DIALOG_STYLE | wxCENTER),
      userManager(userManager) {
    
    user = userManager->GetCurrentUser();
    if (!user) {
        wxMessageBox("Error: No user is logged in.", "Error", wxOK | wxICON_ERROR);
        EndModal(wxID_CANCEL);
        return;
    }
    
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // Profile Information
    wxStaticBoxSizer* profileSizer = new wxStaticBoxSizer(
        new wxStaticBox(this, wxID_ANY, "Profile Information"), wxVERTICAL);
    
    wxFlexGridSizer* profileFormSizer = new wxFlexGridSizer(3, 2, 10, 10);
    profileFormSizer->AddGrowableCol(1);
    
    // Username (readonly)
    profileFormSizer->Add(new wxStaticText(profileSizer->GetStaticBox(), wxID_ANY, "Username:"), 
                         0, wxALIGN_CENTER_VERTICAL);
    usernameCtrl = new wxTextCtrl(profileSizer->GetStaticBox(), wxID_ANY, user->username, 
                                 wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    profileFormSizer->Add(usernameCtrl, 0, wxEXPAND);
    
    // Full Name
    profileFormSizer->Add(new wxStaticText(profileSizer->GetStaticBox(), wxID_ANY, "Full Name:"), 
                         0, wxALIGN_CENTER_VERTICAL);
    fullNameCtrl = new wxTextCtrl(profileSizer->GetStaticBox(), wxID_ANY, user->fullName);
    profileFormSizer->Add(fullNameCtrl, 0, wxEXPAND);
    
    // Email
    profileFormSizer->Add(new wxStaticText(profileSizer->GetStaticBox(), wxID_ANY, "Email:"), 
                         0, wxALIGN_CENTER_VERTICAL);
    emailCtrl = new wxTextCtrl(profileSizer->GetStaticBox(), wxID_ANY, user->email);
    profileFormSizer->Add(emailCtrl, 0, wxEXPAND);
    
    profileSizer->Add(profileFormSizer, 0, wxALL | wxEXPAND, 10);
    
    wxButton* saveProfileButton = new wxButton(profileSizer->GetStaticBox(), wxID_SAVE, "Save Profile");
    profileSizer->Add(saveProfileButton, 0, wxALL | wxALIGN_RIGHT, 10);
    
    mainSizer->Add(profileSizer, 0, wxALL | wxEXPAND, 10);
    
    // Change Password
    wxStaticBoxSizer* passwordSizer = new wxStaticBoxSizer(
        new wxStaticBox(this, wxID_ANY, "Change Password"), wxVERTICAL);
    
    wxFlexGridSizer* passwordFormSizer = new wxFlexGridSizer(3, 2, 10, 10);
    passwordFormSizer->AddGrowableCol(1);
    
    // Current Password
    passwordFormSizer->Add(new wxStaticText(passwordSizer->GetStaticBox(), wxID_ANY, "Current Password:"), 
                          0, wxALIGN_CENTER_VERTICAL);
    currentPasswordCtrl = new wxTextCtrl(passwordSizer->GetStaticBox(), wxID_ANY, wxEmptyString, 
                                        wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    passwordFormSizer->Add(currentPasswordCtrl, 0, wxEXPAND);
    
    // New Password
    passwordFormSizer->Add(new wxStaticText(passwordSizer->GetStaticBox(), wxID_ANY, "New Password:"), 
                          0, wxALIGN_CENTER_VERTICAL);
    newPasswordCtrl = new wxTextCtrl(passwordSizer->GetStaticBox(), wxID_ANY, wxEmptyString, 
                                    wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    passwordFormSizer->Add(newPasswordCtrl, 0, wxEXPAND);
    
    // Confirm New Password
    passwordFormSizer->Add(new wxStaticText(passwordSizer->GetStaticBox(), wxID_ANY, "Confirm New Password:"), 
                          0, wxALIGN_CENTER_VERTICAL);
    confirmPasswordCtrl = new wxTextCtrl(passwordSizer->GetStaticBox(), wxID_ANY, wxEmptyString, 
                                        wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    passwordFormSizer->Add(confirmPasswordCtrl, 0, wxEXPAND);
    
    passwordSizer->Add(passwordFormSizer, 0, wxALL | wxEXPAND, 10);
    
    wxButton* changePasswordButton = new wxButton(passwordSizer->GetStaticBox(), wxID_APPLY, "Change Password");
    passwordSizer->Add(changePasswordButton, 0, wxALL | wxALIGN_RIGHT, 10);
    
    mainSizer->Add(passwordSizer, 0, wxALL | wxEXPAND, 10);
    
    // Close Button
    wxButton* closeButton = new wxButton(this, wxID_CANCEL, "Close");
    mainSizer->Add(closeButton, 0, wxALL | wxALIGN_RIGHT, 10);
    
    SetSizer(mainSizer);
    Centre();
}

ProfileDialog::~ProfileDialog() {
}

void ProfileDialog::OnSaveProfileButton(wxCommandEvent& event) {
    wxString fullName = fullNameCtrl->GetValue().Trim();
    wxString email = emailCtrl->GetValue().Trim();
    
    if (fullName.IsEmpty()) {
        wxMessageBox("Full name cannot be empty.", 
                    "Validation Error", wxOK | wxICON_ERROR, this);
        fullNameCtrl->SetFocus();
        return;
    }
    
    if (email.IsEmpty()) {
        wxMessageBox("Email cannot be empty.", 
                    "Validation Error", wxOK | wxICON_ERROR, this);
        emailCtrl->SetFocus();
        return;
    }
    
    if (userManager->UpdateUser(user->id, email, fullName)) {
        wxMessageBox("Profile updated successfully.", 
                    "Profile Updated", wxOK | wxICON_INFORMATION, this);
    }
}

void ProfileDialog::OnChangePasswordButton(wxCommandEvent& event) {
    wxString currentPassword = currentPasswordCtrl->GetValue();
    wxString newPassword = newPasswordCtrl->GetValue();
    wxString confirmPassword = confirmPasswordCtrl->GetValue();
    
    if (currentPassword.IsEmpty()) {
        wxMessageBox("Current password cannot be empty.", 
                    "Validation Error", wxOK | wxICON_ERROR, this);
        currentPasswordCtrl->SetFocus();
        return;
    }
    
    if (newPassword.IsEmpty()) {
        wxMessageBox("New password cannot be empty.", 
                    "Validation Error", wxOK | wxICON_ERROR, this);
        newPasswordCtrl->SetFocus();
        return;
    }
    
    if (newPassword != confirmPassword) {
        wxMessageBox("New passwords do not match.", 
                    "Validation Error", wxOK | wxICON_ERROR, this);
        confirmPasswordCtrl->SetFocus();
        return;
    }
    
    if (userManager->ChangePassword(user->id, currentPassword, newPassword)) {
        wxMessageBox("Password changed successfully.", 
                    "Password Changed", wxOK | wxICON_INFORMATION, this);
        currentPasswordCtrl->Clear();
        newPasswordCtrl->Clear();
        confirmPasswordCtrl->Clear();
    }
}

void ProfileDialog::OnCancelButton(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}
