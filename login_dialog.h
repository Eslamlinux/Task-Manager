#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <wx/wx.h>
#include "user_manager.h"

class LoginDialog : public wxDialog {
private:
    UserManager* userManager;
    
    wxTextCtrl* usernameCtrl;
    wxTextCtrl* passwordCtrl;
    wxButton* loginButton;
    wxButton* cancelButton;
    wxButton* registerButton;
    
    void OnLoginButton(wxCommandEvent& event);
    void OnRegisterButton(wxCommandEvent& event);
    void OnCancelButton(wxCommandEvent& event);
    
public:
    LoginDialog(wxWindow* parent, UserManager* userManager);
    virtual ~LoginDialog();
    
    bool PerformLogin();
    
    wxDECLARE_EVENT_TABLE();
};

#endif // LOGIN_DIALOG_H
