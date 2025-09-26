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

