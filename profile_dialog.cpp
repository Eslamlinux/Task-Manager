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
    
