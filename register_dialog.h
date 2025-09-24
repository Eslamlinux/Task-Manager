#ifndef REGISTER_DIALOG_H
#define REGISTER_DIALOG_H

#include <wx/wx.h>
#include <wx/valtext.h>
#include "user_manager.h"

class RegisterDialog : public wxDialog {
private:
    UserManager* userManager;
    
    wxTextCtrl* usernameCtrl;
    wxTextCtrl* passwordCtrl;
    wxTextCtrl* confirmPasswordCtrl;
    wxTextCtrl* emailCtrl;
    wxTextCtrl* fullNameCtrl;
    
    wxString username;
    
    void OnRegisterButton(wxCommandEvent& event);
    void OnCancelButton(wxCommandEvent& event);
    
public:
    RegisterDialog(wxWindow* parent, UserManager* userManager);
    virtual ~RegisterDialog();
    
    wxString GetUsername() const { return username; }
    
    wxDECLARE_EVENT_TABLE();
};

#endif // REGISTER_DIALOG_H
