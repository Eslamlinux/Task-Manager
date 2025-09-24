#ifndef CATEGORY_DIALOG_H
#define CATEGORY_DIALOG_H

#include <wx/wx.h>
#include <wx/colourdata.h>
#include <wx/clrpicker.h>
#include "category_manager.h"

class CategoryDialog : public wxDialog {
private:
    CategoryManager* categoryManager;
    int userId;
    int categoryId;
    
    wxTextCtrl* nameCtrl;
    wxColourPickerCtrl* colorCtrl;
    wxTextCtrl* descriptionCtrl;
    
    void OnSaveButton(wxCommandEvent& event);
    void OnCancelButton(wxCommandEvent& event);
    
public:
    CategoryDialog(wxWindow* parent, CategoryManager* categoryManager, 
                  int userId, Category* category = nullptr);
    virtual ~CategoryDialog();
    
    wxDECLARE_EVENT_TABLE();
};

#endif // CATEGORY_DIALOG_H
