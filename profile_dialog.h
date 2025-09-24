#ifndef PROFILE_DIALOG_H
#define PROFILE_DIALOG_H

#include <wx/wx.h>
#include "user_manager.h"

class ProfileDialog : public wxDialog {
private:
    UserManager* userManager;
    User* user;
    
    wxTextCtrl* fullNameCtrl;
    wxTextCtrl* emailCtrl;
    wxTextCtrl* usernameCtrl;
    
    wxTextCtrl* currentPasswordCtrl;
    wxTextCtrl* newPasswordCtrl;
    wxTextCtrl* confirmPasswordCtrl;
    
    void OnSaveProfileButton(wxCommandEvent& event);
    void OnChangePasswordButton(wxCommandEvent& event);
    void OnCancelButton(wxCommandEvent& event);
    
public:
    ProfileDialog(wxWindow* parent, UserManager* userManager);
    virtual ~ProfileDialog();
    
    wxDECLARE_EVENT_TABLE();
};

#endif // PROFILE_DIALOG_H
