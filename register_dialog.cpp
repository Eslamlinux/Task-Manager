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
    
