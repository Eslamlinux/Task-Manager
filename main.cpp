#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/grid.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/clipbrd.h>
#include <wx/filedlg.h>
#include <wx/textfile.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/tokenzr.h>
#include <wx/chartype.h>
#include <wx/colordlg.h>
#include <wx/combobox.h>
#include "wxsqlite3/wxsqlite3.h"
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <iostream>

#include "user_manager.h"
#include "login_dialog.h"
#include "category_manager.h"
#include "profile_dialog.h"
#include "category_list_dialog.h"

// Enhanced Task data structure
struct Task {
  int id;
  wxString title;
  wxString description;
  wxString dueDate;
  int priority;
  bool completed;
  int categoryId;
  wxString categoryName;
  wxString categoryColor;
  int userId; // Owner of the task
};

// Enhanced Database Manager class
class DatabaseManager {
private:
  wxSQLite3Database db;
  bool isConnected;

public:
  DatabaseManager() : isConnected(false) {}

  bool Connect(const wxString& dbPath) {
    try {
        std::cout << "Attempting to open database: " << dbPath.ToStdString() << std::endl;
        
        // Check if the database file exists
        bool fileExists = wxFileExists(dbPath);
        std::cout << "Database file exists: " << (fileExists ? "yes" : "no") << std::endl;
        
        // Open the database using wxSQLite3Database's method
        db.Open(dbPath);
        isConnected = true;
        std::cout << "Database connected successfully" << std::endl;
        
        // Enable foreign keys
        db.ExecuteUpdate("PRAGMA foreign_keys = ON");
        
        // Create tables if they don't exist
        std::cout << "Creating tables if they don't exist" << std::endl;
        db.ExecuteUpdate(
            "CREATE TABLE IF NOT EXISTS tasks ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "title TEXT NOT NULL, "
            "description TEXT, "
            "due_date TEXT, "
            "priority INTEGER, "
            "completed INTEGER DEFAULT 0, "
            "category_id INTEGER, "
            "user_id INTEGER, "
            "FOREIGN KEY(category_id) REFERENCES categories(id), "
            "FOREIGN KEY(user_id) REFERENCES users(id))"
        );
        std::cout << "Tables created successfully" << std::endl;
        
        return true;
    } catch (wxSQLite3Exception& e) {
        std::cerr << "Database error: " << e.GetMessage().ToStdString() << std::endl;
        wxMessageBox(e.GetMessage(), "Database Error", wxOK | wxICON_ERROR);
        isConnected = false;
        return false;
    }
}

  bool IsConnected() const {
      return isConnected;
  }

  wxSQLite3Database* GetDatabase() {
      return &db;
  }

  std::vector<Task> GetAllTasks(int userId, bool includeCompleted = true) {
      std::vector<Task> results;
      
      try {
          wxString sql = 
              "SELECT t.id, t.title, t.description, t.due_date, t.priority, t.completed, "
              "t.category_id, c.name as category_name, c.color as category_color "
              "FROM tasks t "
              "LEFT JOIN categories c ON t.category_id = c.id "
              "WHERE t.user_id = ? ";
          
          if (!includeCompleted) {
              sql += "AND t.completed = 0 ";
          }
          
          sql += "ORDER BY t.due_date, t.priority DESC";
          
          wxSQLite3Statement stmt = db.PrepareStatement(sql);
          stmt.Bind(1, userId);
          stmt.Reset();
          wxSQLite3ResultSet set = stmt.ExecuteQuery();
          
          while (set.NextRow()) {
              Task task;
              task.id = set.GetAsInt(0);
              task.title = set.GetAsString(1);
              task.description = set.GetAsString(2);
              task.dueDate = set.GetAsString(3);
              task.priority = set.GetAsInt(4);
              task.completed = set.GetAsInt(5) != 0;
              task.categoryId = set.IsNull(6) ? -1 : set.GetAsInt(6);
              task.categoryName = set.IsNull(7) ? "No Category" : set.GetAsString(7);
              task.categoryColor = set.IsNull(8) ? "#FFFFFF" : set.GetAsString(8);
              task.userId = userId;
              
              results.push_back(task);
          }
      } catch (wxSQLite3Exception& e) {
          std::cerr << "Query error: " << e.GetMessage().ToStdString() << std::endl;
          wxMessageBox(e.GetMessage(), "Query Error", wxOK | wxICON_ERROR);
      }
      
      return results;
  }

  bool AddTask(const wxString& title, const wxString& description, 
             const wxString& dueDate, int priority, int categoryId, int userId) {
      try {
          wxSQLite3Statement stmt = db.PrepareStatement(
              "INSERT INTO tasks (title, description, due_date, priority, category_id, user_id) "
              "VALUES (?, ?, ?, ?, ?, ?)"
          );
          
          stmt.Bind(1, title);
          stmt.Bind(2, description);
          stmt.Bind(3, dueDate);
          stmt.Bind(4, priority);
          
          if (categoryId > 0) {
              stmt.Bind(5, categoryId);
          } else {
              stmt.BindNull(5);
          }
          
          stmt.Bind(6, userId);
          
          stmt.ExecuteUpdate();
          return true;
      } catch (wxSQLite3Exception& e) {
          std::cerr << "Database error: " << e.GetMessage().ToStdString() << std::endl;
          wxMessageBox(e.GetMessage(), "Database Error", wxOK | wxICON_ERROR);
          return false;
      }
  }

  bool UpdateTask(int id, const wxString& title, const wxString& description, 
                const wxString& dueDate, int priority, bool completed, int categoryId) {
      try {
          wxSQLite3Statement stmt = db.PrepareStatement(
              "UPDATE tasks SET title = ?, description = ?, due_date = ?, "
              "priority = ?, completed = ?, category_id = ? WHERE id = ?"
          );
          
          stmt.Bind(1, title);
          stmt.Bind(2, description);
          stmt.Bind(3, dueDate);
          stmt.Bind(4, priority);
          stmt.Bind(5, completed ? 1 : 0);
          
          if (categoryId > 0) {
              stmt.Bind(6, categoryId);
          } else {
              stmt.BindNull(6);
          }
          
          stmt.Bind(7, id);
          
          stmt.ExecuteUpdate();
          return true;
      } catch (wxSQLite3Exception& e) {
          std::cerr << "Database error: " << e.GetMessage().ToStdString() << std::endl;
          wxMessageBox(e.GetMessage(), "Database Error", wxOK | wxICON_ERROR);
          return false;
      }
  }

  bool DeleteTask(int id) {
      try {
          wxSQLite3Statement stmt = db.PrepareStatement("DELETE FROM tasks WHERE id = ?");
          stmt.Bind(1, id);
          stmt.ExecuteUpdate();
          return true;
      } catch (wxSQLite3Exception& e) {
          std::cerr << "Database error: " << e.GetMessage().ToStdString() << std::endl;
          wxMessageBox(e.GetMessage(), "Database Error", wxOK | wxICON_ERROR);
          return false;
      }
  }

  ~DatabaseManager() {
      if (isConnected) {
          db.Close();
      }
  }
};

// Main application class
class TaskManagerApp : public wxApp {
private:
  std::unique_ptr<DatabaseManager> dbManager;
  std::unique_ptr<UserManager> userManager;
  std::unique_ptr<CategoryManager> categoryManager;

public:
  virtual bool OnInit() override;
};

// Forward declaration for TaskDetailDialog
class TaskDetailDialog;

// Main frame class
class MainFrame : public wxFrame {
private:
  wxNotebook* notebook;
  wxPanel* dashboardPanel;
  wxPanel* tasksPanel;
  wxPanel* settingsPanel;
  
  wxGrid* tasksGrid;
  wxTextCtrl* titleCtrl;
  wxTextCtrl* descriptionCtrl;
  wxDatePickerCtrl* dueDateCtrl;
  wxSpinCtrl* priorityCtrl;
  wxCheckBox* completedCtrl;
  wxComboBox* categoryCombo;
  wxButton* addButton;
  wxButton* updateButton;
  wxButton* deleteButton;
  
  wxMenuItem* logoutMenuItem;
  wxMenuItem* profileMenuItem;
  wxMenuItem* categoriesMenuItem;
  
  DatabaseManager* dbManager;
  UserManager* userManager;
  CategoryManager* categoryManager;
  
  std::vector<Task> tasks;
  std::vector<Category> categories;
  int selectedTaskId;
  
  // Dashboard statistics
  wxStaticText* totalTasksText;
  wxStaticText* completedTasksText;
  wxStaticText* pendingTasksText;
  wxStaticText* urgentTasksText;
  wxListCtrl* recentTasksList;

public:
  MainFrame(const wxString& title, DatabaseManager* dbManager, 
           UserManager* userManager, CategoryManager* categoryManager);

  void CreateMenuBar();
  void CreateDashboardPanel(wxPanel* panel);
  void CreateTasksPanel(wxPanel* panel);
  void CreateSettingsPanel(wxPanel* panel);
  
  void UpdateDashboardStatistics();
  void LoadTasks();
  void LoadCategories();
  void DisplayTasks();
  void DisplayRecentTasks();
  void ClearForm();
  
  void OnTaskSelect(wxGridEvent& event);
  void OnAddTask(wxCommandEvent& event);
  void OnUpdateTask(wxCommandEvent& event);
  void OnDeleteTask(wxCommandEvent& event);
  void OnTaskDetail(wxGridEvent& event);
  void OnExportTasks(wxCommandEvent& event);
  void OnImportTasks(wxCommandEvent& event);
  void OnLogout(wxCommandEvent& event);
  void OnProfile(wxCommandEvent& event);
  void OnManageCategories(wxCommandEvent& event);
  void OnExit(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnSearchTasks(wxCommandEvent& event);
  void OnToggleCompleted(wxCommandEvent& event);
  
  // IDs for menu items and controls
  enum {
      ID_LOGOUT = wxID_HIGHEST + 1,
      ID_PROFILE,
      ID_MANAGE_CATEGORIES,
      ID_EXPORT_TASKS,
      ID_IMPORT_TASKS,
      ID_SEARCH_TASKS,
      ID_TOGGLE_COMPLETED
  };
  
  wxDECLARE_EVENT_TABLE();
};

// Event table
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
  EVT_GRID_SELECT_CELL(MainFrame::OnTaskSelect)
  EVT_GRID_CELL_LEFT_DCLICK(MainFrame::OnTaskDetail)
  EVT_BUTTON(wxID_ADD, MainFrame::OnAddTask)
  EVT_BUTTON(wxID_SAVE, MainFrame::OnUpdateTask)
  EVT_BUTTON(wxID_DELETE, MainFrame::OnDeleteTask)
  EVT_MENU(ID_LOGOUT, MainFrame::OnLogout)
  EVT_MENU(ID_PROFILE, MainFrame::OnProfile)
  EVT_MENU(ID_MANAGE_CATEGORIES, MainFrame::OnManageCategories)
  EVT_MENU(ID_EXPORT_TASKS, MainFrame::OnExportTasks)
  EVT_MENU(ID_IMPORT_TASKS, MainFrame::OnImportTasks)
  EVT_MENU(wxID_EXIT, MainFrame::OnExit)
  EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
  EVT_MENU(ID_SEARCH_TASKS, MainFrame::OnSearchTasks)
  EVT_MENU(ID_TOGGLE_COMPLETED, MainFrame::OnToggleCompleted)
wxEND_EVENT_TABLE()

// Task Detail Dialog
class TaskDetailDialog : public wxDialog {
private:
  Task* task;
  
  wxTextCtrl* titleCtrl;
  wxTextCtrl* descriptionCtrl;
  wxDatePickerCtrl* dueDateCtrl;
  wxSpinCtrl* priorityCtrl;
  wxCheckBox* completedCtrl;
  wxComboBox* categoryCombo;
  
  DatabaseManager* dbManager;
  CategoryManager* categoryManager;
  int userId;
  std::vector<Category> categories;
  
  void OnSaveButton(wxCommandEvent& event);
  void OnCancelButton(wxCommandEvent& event);
  
  void LoadCategories();
  
public:
  TaskDetailDialog(wxWindow* parent, Task* task, DatabaseManager* dbManager, 
                  CategoryManager* categoryManager, int userId);
  virtual ~TaskDetailDialog();
  
  Task* GetTask() const { return task; }
  
  wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(TaskDetailDialog, wxDialog)
  EVT_BUTTON(wxID_SAVE, TaskDetailDialog::OnSaveButton)
  EVT_BUTTON(wxID_CANCEL, TaskDetailDialog::OnCancelButton)
wxEND_EVENT_TABLE()

TaskDetailDialog::TaskDetailDialog(wxWindow* parent, Task* task, DatabaseManager* dbManager, 
                               CategoryManager* categoryManager, int userId)
  : wxDialog(parent, wxID_ANY, "Task Details", wxDefaultPosition, wxSize(500, 400),
            wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    task(task), dbManager(dbManager), categoryManager(categoryManager), userId(userId) {
  
  // Add validation
  if (!task) {
      wxMessageBox("Error: Invalid task data", "Error", wxOK | wxICON_ERROR);
      EndModal(wxID_CANCEL);
      return;
  }
  
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  
  // Form
  wxFlexGridSizer* formSizer = new wxFlexGridSizer(6, 2, 10, 10);
  formSizer->AddGrowableCol(1);
  formSizer->AddGrowableRow(1);
  
  // Title
  formSizer->Add(new wxStaticText(this, wxID_ANY, "Title:"), 0, wxALIGN_CENTER_VERTICAL);
  titleCtrl = new wxTextCtrl(this, wxID_ANY, task->title);
  formSizer->Add(titleCtrl, 0, wxEXPAND);
  
  // Description
  formSizer->Add(new wxStaticText(this, wxID_ANY, "Description:"), 0, wxALIGN_CENTER_VERTICAL);
  descriptionCtrl = new wxTextCtrl(this, wxID_ANY, task->description, wxDefaultPosition, 
                                  wxDefaultSize, wxTE_MULTILINE);
  formSizer->Add(descriptionCtrl, 0, wxEXPAND);
  
  // Due Date
  formSizer->Add(new wxStaticText(this, wxID_ANY, "Due Date:"), 0, wxALIGN_CENTER_VERTICAL);
  wxDateTime dueDate;
  dueDate.ParseISODate(task->dueDate);
  dueDateCtrl = new wxDatePickerCtrl(this, wxID_ANY, dueDate);
  formSizer->Add(dueDateCtrl, 0, wxEXPAND);
  
  // Priority
  formSizer->Add(new wxStaticText(this, wxID_ANY, "Priority:"), 0, wxALIGN_CENTER_VERTICAL);
  priorityCtrl = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", task->priority), 
                               wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 5, task->priority);
  formSizer->Add(priorityCtrl, 0, wxEXPAND);
  
  // Category
  formSizer->Add(new wxStaticText(this, wxID_ANY, "Category:"), 0, wxALIGN_CENTER_VERTICAL);
  categoryCombo = new wxComboBox(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 
                                0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
  formSizer->Add(categoryCombo, 0, wxEXPAND);
  
  // Completed
  formSizer->Add(new wxStaticText(this, wxID_ANY, "Completed:"), 0, wxALIGN_CENTER_VERTICAL);
  completedCtrl = new wxCheckBox(this, wxID_ANY, "");
  completedCtrl->SetValue(task->completed);
  formSizer->Add(completedCtrl, 0);
  
  mainSizer->Add(formSizer, 1, wxALL | wxEXPAND, 10);
  
  // Buttons
  wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxButton* saveButton = new wxButton(this, wxID_SAVE, "Save");
  wxButton* cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  
  buttonSizer->AddStretchSpacer();
  buttonSizer->Add(saveButton, 0, wxRIGHT, 5);
  buttonSizer->Add(cancelButton, 0);
  
  mainSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, 10);
  
  SetSizer(mainSizer);
  
  // Load categories
  LoadCategories();
  
  titleCtrl->SetFocus();
}

TaskDetailDialog::~TaskDetailDialog() {
}

