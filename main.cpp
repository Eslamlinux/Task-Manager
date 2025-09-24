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

void TaskDetailDialog::LoadCategories() {
  categories = categoryManager->GetAllCategories(userId);
  
  // Add 'No Category' option
  categoryCombo->Append("No Category", (void*)-1);
  
  for (const auto& category : categories) {
      categoryCombo->Append(category.name, (void*)(wxIntPtr)category.id);
  }
  
  // Select current category
  int selection = 0; // Default to 'No Category'
  for (size_t i = 0; i < categories.size(); ++i) {
      if (categories[i].id == task->categoryId) {
          selection = i + 1; // +1 because 'No Category' is at index 0
          break;
      }
  }
  categoryCombo->SetSelection(selection);
}

void TaskDetailDialog::OnSaveButton(wxCommandEvent& event) {
  wxString title = titleCtrl->GetValue().Trim();
  wxString description = descriptionCtrl->GetValue().Trim();
  wxDateTime dueDate = dueDateCtrl->GetValue();
  int priority = priorityCtrl->GetValue();
  bool completed = completedCtrl->GetValue();
  
  // Get selected category ID
  int categoryId = -1;
  int selection = categoryCombo->GetSelection();
  if (selection != wxNOT_FOUND) {
      categoryId = (int)(wxIntPtr)categoryCombo->GetClientData(selection);
  }
  
  if (title.IsEmpty()) {
      wxMessageBox("Title cannot be empty.", "Validation Error", wxOK | wxICON_ERROR, this);
      titleCtrl->SetFocus();
      return;
  }
  
  wxString dueDateStr = dueDate.FormatISODate();
  
  // Update task object
  task->title = title;
  task->description = description;
  task->dueDate = dueDateStr;
  task->priority = priority;
  task->completed = completed;
  task->categoryId = categoryId;
  
  // Update task in database if it already exists
  if (task->id > 0) {
      if (!dbManager->UpdateTask(task->id, title, description, dueDateStr, priority, completed, categoryId)) {
          wxMessageBox("Failed to update task.", "Error", wxOK | wxICON_ERROR, this);
          return;
      }
  }
  
  EndModal(wxID_OK);
}

void TaskDetailDialog::OnCancelButton(wxCommandEvent& event) {
  EndModal(wxID_CANCEL);
}

// Search Dialog
class SearchDialog : public wxDialog {
private:
  wxTextCtrl* searchCtrl;
  wxChoice* fieldChoice;
  wxChoice* priorityChoice;
  wxDatePickerCtrl* fromDateCtrl;
  wxDatePickerCtrl* toDateCtrl;
  wxCheckBox* includeCompletedCtrl;
  wxComboBox* categoryCombo;
  
  DatabaseManager* dbManager;
  CategoryManager* categoryManager;
  int userId;
  std::vector<Category> categories;
  
  std::vector<Task>* searchResults;
  
  void OnSearchButton(wxCommandEvent& event);
  void OnCancelButton(wxCommandEvent& event);
  void OnFieldChoice(wxCommandEvent& event);
  
  void LoadCategories();
  
public:
  SearchDialog(wxWindow* parent, DatabaseManager* dbManager, 
              CategoryManager* categoryManager, int userId, 
              std::vector<Task>* results);
  virtual ~SearchDialog();
  
  bool PerformSearch();
  
  wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(SearchDialog, wxDialog)
  EVT_BUTTON(wxID_FIND, SearchDialog::OnSearchButton)
  EVT_BUTTON(wxID_CANCEL, SearchDialog::OnCancelButton)
  EVT_CHOICE(wxID_ANY, SearchDialog::OnFieldChoice)
wxEND_EVENT_TABLE()

SearchDialog::SearchDialog(wxWindow* parent, DatabaseManager* dbManager, 
                       CategoryManager* categoryManager, int userId, 
                       std::vector<Task>* results)
  : wxDialog(parent, wxID_ANY, "Search Tasks", wxDefaultPosition, wxSize(450, 400),
            wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    dbManager(dbManager), categoryManager(categoryManager), 
    userId(userId), searchResults(results) {
  
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  
  // Search form
  wxFlexGridSizer* formSizer = new wxFlexGridSizer(7, 2, 10, 10);
  formSizer->AddGrowableCol(1);
  
  // Search Field
  formSizer->Add(new wxStaticText(this, wxID_ANY, "Search in:"), 0, wxALIGN_CENTER_VERTICAL);
  wxArrayString fieldChoices;
  fieldChoices.Add("Title");
  fieldChoices.Add("Description");
  fieldChoices.Add("All Fields");
  fieldChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, fieldChoices);
  fieldChoice->SetSelection(0);
  formSizer->Add(fieldChoice, 0, wxEXPAND);
  
  // Search Text
  formSizer->Add(new wxStaticText(this, wxID_ANY, "Search for:"), 0, wxALIGN_CENTER_VERTICAL);
  searchCtrl = new wxTextCtrl(this, wxID_ANY);
  formSizer->Add(searchCtrl, 0, wxEXPAND);
  
  // Priority
  formSizer->Add(new wxStaticText(this, wxID_ANY, "Priority:"), 0, wxALIGN_CENTER_VERTICAL);
  wxArrayString priorityChoices;
  priorityChoices.Add("Any");
  priorityChoices.Add("1 - Low");
  priorityChoices.Add("2");
  priorityChoices.Add("3 - Medium");
  priorityChoices.Add("4");
  priorityChoices.Add("5 - High");
  priorityChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, priorityChoices);
  priorityChoice->SetSelection(0);
  formSizer->Add(priorityChoice, 0, wxEXPAND);
  
  // Date Range
  formSizer->Add(new wxStaticText(this, wxID_ANY, "From Date:"), 0, wxALIGN_CENTER_VERTICAL);
  wxDateTime fromDate = wxDateTime::Today().Subtract(wxDateSpan::Days(30));
  fromDateCtrl = new wxDatePickerCtrl(this, wxID_ANY, fromDate);
  formSizer->Add(fromDateCtrl, 0, wxEXPAND);
  
  formSizer->Add(new wxStaticText(this, wxID_ANY, "To Date:"), 0, wxALIGN_CENTER_VERTICAL);
  wxDateTime toDate = wxDateTime::Today().Add(wxDateSpan::Days(30));
  toDateCtrl = new wxDatePickerCtrl(this, wxID_ANY, toDate);
  formSizer->Add(toDateCtrl, 0, wxEXPAND);
  
  // Category
  formSizer->Add(new wxStaticText(this, wxID_ANY, "Category:"), 0, wxALIGN_CENTER_VERTICAL);
  categoryCombo = new wxComboBox(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 
                                0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
  formSizer->Add(categoryCombo, 0, wxEXPAND);
  
  // Include Completed
  formSizer->Add(new wxStaticText(this, wxID_ANY, "Status:"), 0, wxALIGN_CENTER_VERTICAL);
  includeCompletedCtrl = new wxCheckBox(this, wxID_ANY, "Include completed tasks");
  includeCompletedCtrl->SetValue(true);
  formSizer->Add(includeCompletedCtrl, 0);
  
  mainSizer->Add(formSizer, 0, wxALL | wxEXPAND, 10);
  
  // Buttons
  wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxButton* searchButton = new wxButton(this, wxID_FIND, "Search");
  wxButton* cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  
  buttonSizer->AddStretchSpacer();
  buttonSizer->Add(searchButton, 0, wxRIGHT, 5);
  buttonSizer->Add(cancelButton, 0);
  
  mainSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, 10);
  
  SetSizer(mainSizer);
  
  // Load categories
  LoadCategories();
  
  searchCtrl->SetFocus();
  searchButton->SetDefault();
}

SearchDialog::~SearchDialog() {
}

void SearchDialog::LoadCategories() {
  categories = categoryManager->GetAllCategories(userId);
  
  // Add 'Any Category' option
  categoryCombo->Append("Any Category", (void*)-1);
  
  // Add 'No Category' option
  categoryCombo->Append("No Category", (void*)0);
  
  for (const auto& category : categories) {
      categoryCombo->Append(category.name, (void*)(wxIntPtr)category.id);
  }
  
  categoryCombo->SetSelection(0); // Default to 'Any Category'
}

void SearchDialog::OnFieldChoice(wxCommandEvent& event) {
  // You could add logic here to enable/disable search text based on field choice
}

bool SearchDialog::PerformSearch() {
  wxString searchText = searchCtrl->GetValue().Trim();
  int fieldIndex = fieldChoice->GetSelection();
  int priorityIndex = priorityChoice->GetSelection();
  wxDateTime fromDate = fromDateCtrl->GetValue();
  wxDateTime toDate = toDateCtrl->GetValue();
  bool includeCompleted = includeCompletedCtrl->GetValue();
  
  // Get selected category ID
  int categoryId = -1;
  int selection = categoryCombo->GetSelection();
  if (selection != wxNOT_FOUND) {
      categoryId = (int)(wxIntPtr)categoryCombo->GetClientData(selection);
  }
  
  // Convert dates to ISO format
  wxString fromDateStr = fromDate.FormatISODate();
  wxString toDateStr = toDate.FormatISODate();
  
  // Build SQL query
  wxString sql = 
      "SELECT t.id, t.title, t.description, t.due_date, t.priority, t.completed, "
      "t.category_id, c.name as category_name, c.color as category_color "
      "FROM tasks t "
      "LEFT JOIN categories c ON t.category_id = c.id "
      "WHERE t.user_id = ? ";
  
  // Search text condition
  if (!searchText.IsEmpty()) {
      switch(fieldIndex) {
          case 0: // Title
              sql += "AND t.title LIKE ? ";
              break;
          case 1: // Description
              sql += "AND t.description LIKE ? ";
              break;
          case 2: // All Fields
              sql += "AND (t.title LIKE ? OR t.description LIKE ?) ";
              break;
      }
  }
  
  // Priority condition
  if (priorityIndex > 0) {
      sql += "AND t.priority = ? ";
  }
  
  // Date range condition
  sql += "AND t.due_date BETWEEN ? AND ? ";
  
  // Category condition
  if (categoryId == 0) { // No Category
      sql += "AND t.category_id IS NULL ";
  } else if (categoryId > 0) { // Specific Category
      sql += "AND t.category_id = ? ";
  }
  
  // Completed status condition
  if (!includeCompleted) {
      sql += "AND t.completed = 0 ";
  }
  
  sql += "ORDER BY t.due_date, t.priority DESC";
  
  try {
      wxSQLite3Statement stmt = dbManager->GetDatabase()->PrepareStatement(sql);
      int paramIndex = 1;
      
      // Bind user ID
      stmt.Bind(paramIndex++, userId);
      
      // Bind search text
      if (!searchText.IsEmpty()) {
          wxString likePattern = "%" + searchText + "%";
          stmt.Bind(paramIndex++, likePattern);
          
          if (fieldIndex == 2) { // All Fields - need to bind twice
              stmt.Bind(paramIndex++, likePattern);
          }
      }
      
      // Bind priority
      if (priorityIndex > 0) {
          stmt.Bind(paramIndex++, priorityIndex);
      }
      
      // Bind date range
      stmt.Bind(paramIndex++, fromDateStr);
      stmt.Bind(paramIndex++, toDateStr);
      
      // Bind category ID
      if (categoryId > 0) {
          stmt.Bind(paramIndex++, categoryId);
      }
      
      wxSQLite3ResultSet resultSet = stmt.ExecuteQuery();
      
      searchResults->clear();
      
      while (resultSet.NextRow()) {
          Task task;
          task.id = resultSet.GetAsInt(0);
          task.title = resultSet.GetAsString(1);
          task.description = resultSet.GetAsString(2);
          task.dueDate = resultSet.GetAsString(3);
          task.priority = resultSet.GetAsInt(4);
          task.completed = resultSet.GetAsInt(5) != 0;
          task.categoryId = resultSet.IsNull(6) ? -1 : resultSet.GetAsInt(6);
          task.categoryName = resultSet.IsNull(7) ? "No Category" : resultSet.GetAsString(7);
          task.categoryColor = resultSet.IsNull(8) ? "#FFFFFF" : resultSet.GetAsString(8);
          task.userId = userId;
          
          searchResults->push_back(task);
      }
      
      return true;
  }
  catch (wxSQLite3Exception& e) {
      std::cerr << "Search error: " << e.GetMessage().ToStdString() << std::endl;
      wxMessageBox(e.GetMessage(), "Search Error", wxOK | wxICON_ERROR);
      return false;
  }
}

void SearchDialog::OnSearchButton(wxCommandEvent& event) {
  if (PerformSearch()) {
      EndModal(wxID_OK);
  }
}

void SearchDialog::OnCancelButton(wxCommandEvent& event) {
  EndModal(wxID_CANCEL);
}

// Application initialization
bool TaskManagerApp::OnInit() {
  if (!wxApp::OnInit())
      return false;

  std::cout << "TaskManagerApp::OnInit() - Starting application" << std::endl;

  // Initialize database
  dbManager = std::make_unique<DatabaseManager>();
  if (!dbManager->Connect("taskmanager.db")) {
      wxMessageBox("Failed to connect to database.", "Error", wxOK | wxICON_ERROR);
      return false;
  }
  
  std::cout << "Database connected successfully" << std::endl;
  
  // Initialize user manager
  std::cout << "Initializing user manager" << std::endl;
  userManager = std::make_unique<UserManager>(dbManager->GetDatabase());
  
  // Show login dialog
  std::cout << "Showing login dialog" << std::endl;
  LoginDialog loginDlg(nullptr, userManager.get());
  if (loginDlg.ShowModal() != wxID_OK) {
      // User cancelled login
      std::cout << "User cancelled login" << std::endl;
      return false;
  }
  
  std::cout << "Login successful" << std::endl;
  
  // Initialize category manager
  std::cout << "Initializing category manager" << std::endl;
  categoryManager = std::make_unique<CategoryManager>(dbManager->GetDatabase());

  // Create main frame
  std::cout << "Creating main frame" << std::endl;
  MainFrame* frame = new MainFrame("Task Manager", dbManager.get(), userManager.get(), categoryManager.get());
  frame->Show(true);
  
  std::cout << "Application initialization complete" << std::endl;
  return true;
}

// Main frame implementation
MainFrame::MainFrame(const wxString& title, DatabaseManager* dbManager, 
                 UserManager* userManager, CategoryManager* categoryManager)
  : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(900, 700)),
    dbManager(dbManager), userManager(userManager), categoryManager(categoryManager), 
    selectedTaskId(-1) {
  
  std::cout << "MainFrame constructor - Creating menu bar" << std::endl;
  // Create menu bar
  CreateMenuBar();
  
  // Create status bar
  CreateStatusBar();
  SetStatusText(wxString::Format("Logged in as: %s", userManager->GetCurrentUser()->username));
  
  std::cout << "Creating notebook with tabs" << std::endl;
  // Create notebook with tabs
  notebook = new wxNotebook(this, wxID_ANY);
  
  // Create panels for each tab
  dashboardPanel = new wxPanel(notebook);
  tasksPanel = new wxPanel(notebook);
  settingsPanel = new wxPanel(notebook);
  
  // Add panels to notebook
  notebook->AddPage(dashboardPanel, "Dashboard");
  notebook->AddPage(tasksPanel, "Tasks");
  notebook->AddPage(settingsPanel, "Settings");
  
  std::cout << "Creating content for each panel" << std::endl;
  // Create content for each panel
  CreateDashboardPanel(dashboardPanel);
  CreateTasksPanel(tasksPanel);
  CreateSettingsPanel(settingsPanel);
  
  // Create sizer for the frame
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 5);
  SetSizer(mainSizer);
  
  std::cout << "Loading categories and tasks" << std::endl;
  // Load tasks and categories
  LoadCategories();
  LoadTasks();
  UpdateDashboardStatistics();
  std::cout << "MainFrame constructor complete" << std::endl;
}

void MainFrame::CreateMenuBar() {
  wxMenuBar* menuBar = new wxMenuBar;
  
  // File menu
  wxMenu* fileMenu = new wxMenu;
  fileMenu->Append(ID_EXPORT_TASKS, "&Export Tasks...\tCtrl+E", "Export tasks to CSV file");
  fileMenu->Append(ID_IMPORT_TASKS, "&Import Tasks...\tCtrl+I", "Import tasks from CSV file");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT, "E&xit\tAlt+F4", "Quit the application");
  
  // Edit menu
  wxMenu* editMenu = new wxMenu;
  editMenu->Append(ID_SEARCH_TASKS, "&Search Tasks...\tCtrl+F", "Search for tasks");
  editMenu->Append(ID_TOGGLE_COMPLETED, "Show/Hide &Completed Tasks", "Toggle display of completed tasks");
  
  // User menu
  wxMenu* userMenu = new wxMenu;
  profileMenuItem = userMenu->Append(ID_PROFILE, "&Profile", "Edit your user profile");
  categoriesMenuItem = userMenu->Append(ID_MANAGE_CATEGORIES, "&Manage Categories", "Manage task categories");
  userMenu->AppendSeparator();
  logoutMenuItem = userMenu->Append(ID_LOGOUT, "&Logout", "Logout from the application");
  
  // Help menu
  wxMenu* helpMenu = new wxMenu;
  helpMenu->Append(wxID_ABOUT, "&About", "About Task Manager");
  
  // Add menus to menu bar
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(editMenu, "&Edit");
  menuBar->Append(userMenu, "&User");
  menuBar->Append(helpMenu, "&Help");
  
  SetMenuBar(menuBar);
}

void MainFrame::CreateDashboardPanel(wxPanel* panel) {
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  
  // Welcome section
  wxBoxSizer* welcomeSizer = new wxBoxSizer(wxHORIZONTAL);
  
  // User info and welcome message
  wxStaticText* welcomeText = new wxStaticText(panel, wxID_ANY, 
      wxString::Format("Welcome, %s!", userManager->GetCurrentUser()->fullName));
  wxFont font = welcomeText->GetFont();
  font.SetPointSize(font.GetPointSize() + 4);
  font.SetWeight(wxFONTWEIGHT_BOLD);
  welcomeText->SetFont(font);
  
  welcomeSizer->Add(welcomeText, 0, wxALL | wxALIGN_CENTER_VERTICAL, 10);
  
  mainSizer->Add(welcomeSizer, 0, wxEXPAND);
  
  // Statistics section
  wxStaticBoxSizer* statsSizer = new wxStaticBoxSizer(
      new wxStaticBox(panel, wxID_ANY, "Task Statistics"), wxVERTICAL);
  
  wxFlexGridSizer* statsGridSizer = new wxFlexGridSizer(2, 4, 10, 20);
  
  // Total Tasks
  statsGridSizer->Add(new wxStaticText(statsSizer->GetStaticBox(), wxID_ANY, "Total Tasks:"), 
                     0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  totalTasksText = new wxStaticText(statsSizer->GetStaticBox(), wxID_ANY, "0");
  wxFont boldFont = totalTasksText->GetFont();
  boldFont.SetWeight(wxFONTWEIGHT_BOLD);
  totalTasksText->SetFont(boldFont);
  statsGridSizer->Add(totalTasksText, 0, wxALIGN_CENTER_VERTICAL);
  
  // Completed Tasks
  statsGridSizer->Add(new wxStaticText(statsSizer->GetStaticBox(), wxID_ANY, "Completed:"), 
                     0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  completedTasksText = new wxStaticText(statsSizer->GetStaticBox(), wxID_ANY, "0");
  completedTasksText->SetFont(boldFont);
  completedTasksText->SetForegroundColour(wxColour(0, 128, 0)); // Green
  statsGridSizer->Add(completedTasksText, 0, wxALIGN_CENTER_VERTICAL);
  
  // Pending Tasks
  statsGridSizer->Add(new wxStaticText(statsSizer->GetStaticBox(), wxID_ANY, "Pending:"), 
                     0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  pendingTasksText = new wxStaticText(statsSizer->GetStaticBox(), wxID_ANY, "0");
  pendingTasksText->SetFont(boldFont);
  pendingTasksText->SetForegroundColour(wxColour(0, 0, 200)); // Blue
  statsGridSizer->Add(pendingTasksText, 0, wxALIGN_CENTER_VERTICAL);
  
  // Urgent Tasks (due within 7 days)
  statsGridSizer->Add(new wxStaticText(statsSizer->GetStaticBox(), wxID_ANY, "Urgent:"), 
                     0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
  urgentTasksText = new wxStaticText(statsSizer->GetStaticBox(), wxID_ANY, "0");
  urgentTasksText->SetFont(boldFont);
  urgentTasksText->SetForegroundColour(wxColour(200, 0, 0)); // Red
  statsGridSizer->Add(urgentTasksText, 0, wxALIGN_CENTER_VERTICAL);
  
  statsSizer->Add(statsGridSizer, 0, wxALL, 10);
  
  mainSizer->Add(statsSizer, 0, wxALL | wxEXPAND, 10);
  
  // Quick Actions
  wxStaticBoxSizer* actionsSizer = new wxStaticBoxSizer(
      new wxStaticBox(panel, wxID_ANY, "Quick Actions"), wxHORIZONTAL);
  
  wxButton* addTaskButton = new wxButton(actionsSizer->GetStaticBox(), wxID_ANY, "Add New Task");
  wxButton* searchTasksButton = new wxButton(actionsSizer->GetStaticBox(), wxID_ANY, "Search Tasks");
  wxButton* exportTasksButton = new wxButton(actionsSizer->GetStaticBox(), wxID_ANY, "Export Tasks");
  wxButton* manageCategoriesButton = new wxButton(actionsSizer->GetStaticBox(), wxID_ANY, "Manage Categories");
  
  actionsSizer->Add(addTaskButton, 1, wxALL, 5);
  actionsSizer->Add(searchTasksButton, 1, wxALL, 5);
  actionsSizer->Add(exportTasksButton, 1, wxALL, 5);
  actionsSizer->Add(manageCategoriesButton, 1, wxALL, 5);
  
  mainSizer->Add(actionsSizer, 0, wxALL | wxEXPAND, 10);
  
  // Recent Tasks
  wxStaticBoxSizer* recentTasksSizer = new wxStaticBoxSizer(
      new wxStaticBox(panel, wxID_ANY, "Recent Tasks"), wxVERTICAL);
  
  recentTasksList = new wxListCtrl(recentTasksSizer->GetStaticBox(), wxID_ANY, 
                                  wxDefaultPosition, wxDefaultSize, 
                                  wxLC_REPORT | wxLC_SINGLE_SEL);
  
  recentTasksList->InsertColumn(0, "Title", wxLIST_FORMAT_LEFT, 200);
  recentTasksList->InsertColumn(1, "Due Date", wxLIST_FORMAT_LEFT, 100);
  recentTasksList->InsertColumn(2, "Priority", wxLIST_FORMAT_LEFT, 80);
  recentTasksList->InsertColumn(3, "Category", wxLIST_FORMAT_LEFT, 120);
  recentTasksList->InsertColumn(4, "Status", wxLIST_FORMAT_LEFT, 100);
  
  recentTasksSizer->Add(recentTasksList, 1, wxALL | wxEXPAND, 5);
  
  mainSizer->Add(recentTasksSizer, 1, wxALL | wxEXPAND, 10);
  
  panel->SetSizer(mainSizer);
  
  // Connect events
  addTaskButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
      notebook->SetSelection(1); // Switch to Tasks tab
      ClearForm();
  });
  
  searchTasksButton->Bind(wxEVT_BUTTON, &MainFrame::OnSearchTasks, this);
  
  exportTasksButton->Bind(wxEVT_BUTTON, &MainFrame::OnExportTasks, this);
  
  manageCategoriesButton->Bind(wxEVT_BUTTON, &MainFrame::OnManageCategories, this);
  
  recentTasksList->Bind(wxEVT_LIST_ITEM_ACTIVATED, [this](wxListEvent& event) {
      // Get the task ID stored in the item data
      long taskId = event.GetItem().GetData();
      
      // Find the task
      Task* selectedTask = nullptr;
      for (auto& task : tasks) {
          if (task.id == taskId) {
              selectedTask = &task;
              break;
          }
      }
      
      if (selectedTask) {
          TaskDetailDialog dlg(this, selectedTask, dbManager, categoryManager, 
                             userManager->GetCurrentUser()->id);
          if (dlg.ShowModal() == wxID_OK) {
              LoadTasks();
              UpdateDashboardStatistics();
          }
      }
  });
}

void MainFrame::CreateTasksPanel(wxPanel* panel) {
  wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
  
  // Create tasks grid
  tasksGrid = new wxGrid(panel, wxID_ANY);
  tasksGrid->CreateGrid(0, 6);
  tasksGrid->SetColLabelValue(0, "ID");
  tasksGrid->SetColLabelValue(1, "Title");
  tasksGrid->SetColLabelValue(2, "Due Date");
  tasksGrid->SetColLabelValue(3, "Priority");
  tasksGrid->SetColLabelValue(4, "Category");
  tasksGrid->SetColLabelValue(5, "Completed");
  
  tasksGrid->SetColSize(0, 50);
  tasksGrid->SetColSize(1, 200);
  tasksGrid->SetColSize(2, 100);
  tasksGrid->SetColSize(3, 80);
  tasksGrid->SetColSize(4, 120);
  tasksGrid->SetColSize(5, 100);
  
  // Create form for adding/editing tasks
  wxStaticBox* formBox = new wxStaticBox(panel, wxID_ANY, "Task Details");
  wxStaticBoxSizer* formSizer = new wxStaticBoxSizer(formBox, wxVERTICAL);
  
  wxFlexGridSizer* gridSizer = new wxFlexGridSizer(6, 2, 5, 10);
  gridSizer->AddGrowableCol(1);
  
  // Title
  gridSizer->Add(new wxStaticText(formBox, wxID_ANY, "Title:"), 0, wxALIGN_CENTER_VERTICAL);
  titleCtrl = new wxTextCtrl(formBox, wxID_ANY);
  gridSizer->Add(titleCtrl, 0, wxEXPAND);
  
  // Description
  gridSizer->Add(new wxStaticText(formBox, wxID_ANY, "Description:"), 0, wxALIGN_CENTER_VERTICAL);
  descriptionCtrl = new wxTextCtrl(formBox, wxID_ANY, wxEmptyString, wxDefaultPosition, 
      wxSize(-1, 60), wxTE_MULTILINE);
  gridSizer->Add(descriptionCtrl, 0, wxEXPAND);
  
  // Due Date
  gridSizer->Add(new wxStaticText(formBox, wxID_ANY, "Due Date:"), 0, wxALIGN_CENTER_VERTICAL);
  dueDateCtrl = new wxDatePickerCtrl(formBox, wxID_ANY);
  gridSizer->Add(dueDateCtrl, 0, wxEXPAND);
  
  // Priority
  gridSizer->Add(new wxStaticText(formBox, wxID_ANY, "Priority:"), 0, wxALIGN_CENTER_VERTICAL);
  priorityCtrl = new wxSpinCtrl(formBox, wxID_ANY, "1", wxDefaultPosition, wxDefaultSize, 
      wxSP_ARROW_KEYS, 1, 5, 1);
  gridSizer->Add(priorityCtrl, 0, wxEXPAND);
  
  // Category
  gridSizer->Add(new wxStaticText(formBox, wxID_ANY, "Category:"), 0, wxALIGN_CENTER_VERTICAL);
  categoryCombo = new wxComboBox(formBox, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 
                                0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
  gridSizer->Add(categoryCombo, 0, wxEXPAND);
  
  // Completed
  gridSizer->Add(new wxStaticText(formBox, wxID_ANY, "Completed:"), 0, wxALIGN_CENTER_VERTICAL);
  completedCtrl = new wxCheckBox(formBox, wxID_ANY, "");
  gridSizer->Add(completedCtrl, 0, wxEXPAND);
  
  formSizer->Add(gridSizer, 0, wxALL | wxEXPAND, 10);
  
  // Buttons
  wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  
  addButton = new wxButton(formBox, wxID_ADD, "Add Task");
  updateButton = new wxButton(formBox, wxID_SAVE, "Update Task");
  deleteButton = new wxButton(formBox, wxID_DELETE, "Delete Task");
  
  buttonSizer->Add(addButton, 0, wxRIGHT, 5);
  buttonSizer->Add(updateButton, 0, wxRIGHT, 5);
  buttonSizer->Add(deleteButton, 0);
  
  formSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 10);
  
  // Add to main sizer
  mainSizer->Add(tasksGrid, 1, wxALL | wxEXPAND, 10);
  mainSizer->Add(formSizer, 0, wxALL | wxEXPAND, 10);
  
  panel->SetSizer(mainSizer);
  
  // Initially disable update and delete buttons
  updateButton->Disable();
  deleteButton->Disable();
  
  addButton->Enable();
}

void MainFrame::CreateSettingsPanel(wxPanel* panel) {
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  
  wxStaticBox* appearanceBox = new wxStaticBox(panel, wxID_ANY, "Appearance");
  wxStaticBoxSizer* appearanceSizer = new wxStaticBoxSizer(appearanceBox, wxVERTICAL);
  
  wxCheckBox* darkModeCheck = new wxCheckBox(appearanceBox, wxID_ANY, "Dark Mode");
  wxCheckBox* compactViewCheck = new wxCheckBox(appearanceBox, wxID_ANY, "Compact View");
  
  appearanceSizer->Add(darkModeCheck, 0, wxALL, 5);
  appearanceSizer->Add(compactViewCheck, 0, wxALL, 5);
  
  wxStaticBox* notificationsBox = new wxStaticBox(panel, wxID_ANY, "Notifications");
  wxStaticBoxSizer* notificationsSizer = new wxStaticBoxSizer(notificationsBox, wxVERTICAL);
  
  wxCheckBox* enableNotificationsCheck = new wxCheckBox(notificationsBox, wxID_ANY, "Enable Notifications");
  wxCheckBox* soundAlertsCheck = new wxCheckBox(notificationsBox, wxID_ANY, "Sound Alerts");
  
  notificationsSizer->Add(enableNotificationsCheck, 0, wxALL, 5);
  notificationsSizer->Add(soundAlertsCheck, 0, wxALL, 5);
  
  wxStaticBox* dataBox = new wxStaticBox(panel, wxID_ANY, "Data Management");
  wxStaticBoxSizer* dataSizer = new wxStaticBoxSizer(dataBox, wxVERTICAL);
  
  wxButton* exportButton = new wxButton(dataBox, wxID_ANY, "Export Data");
  wxButton* importButton = new wxButton(dataBox, wxID_ANY, "Import Data");
  wxButton* backupButton = new wxButton(dataBox, wxID_ANY, "Backup Database");
  
  dataSizer->Add(exportButton, 0, wxALL | wxEXPAND, 5);
  dataSizer->Add(importButton, 0, wxALL | wxEXPAND, 5);
  dataSizer->Add(backupButton, 0, wxALL | wxEXPAND, 5);
  
  wxStaticBox* accountBox = new wxStaticBox(panel, wxID_ANY, "Account");
  wxStaticBoxSizer* accountSizer = new wxStaticBoxSizer(accountBox, wxVERTICAL);
  
  wxButton* profileButton = new wxButton(accountBox, wxID_ANY, "Edit Profile");
  wxButton* changePasswordButton = new wxButton(accountBox, wxID_ANY, "Change Password");
  wxButton* logoutButton = new wxButton(accountBox, wxID_ANY, "Logout");
  
  accountSizer->Add(profileButton, 0, wxALL | wxEXPAND, 5);
  accountSizer->Add(changePasswordButton, 0, wxALL | wxEXPAND, 5);
  accountSizer->Add(logoutButton, 0, wxALL | wxEXPAND, 5);
  
  wxButton* saveSettingsButton = new wxButton(panel, wxID_ANY, "Save Settings");
  
  sizer->Add(appearanceSizer, 0, wxALL | wxEXPAND, 10);
  sizer->Add(notificationsSizer, 0, wxALL | wxEXPAND, 10);
  sizer->Add(dataSizer, 0, wxALL | wxEXPAND, 10);
  sizer->Add(accountSizer, 0, wxALL | wxEXPAND, 10);
  sizer->Add(saveSettingsButton, 0, wxALL | wxALIGN_RIGHT, 10);
  
  panel->SetSizer(sizer);
  
  // Connect events
  profileButton->Bind(wxEVT_BUTTON, &MainFrame::OnProfile, this);
  logoutButton->Bind(wxEVT_BUTTON, &MainFrame::OnLogout, this);
  exportButton->Bind(wxEVT_BUTTON, &MainFrame::OnExportTasks, this);
  importButton->Bind(wxEVT_BUTTON, &MainFrame::OnImportTasks, this);
  changePasswordButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
      ProfileDialog dlg(this, userManager);
      dlg.ShowModal();
  });
}

void MainFrame::UpdateDashboardStatistics() {
  int totalTasks = tasks.size();
  int completedTasks = 0;
  int pendingTasks = 0;
  int urgentTasks = 0;
  
  wxDateTime today = wxDateTime::Today();
  wxDateTime nextWeek = today.Add(wxDateSpan::Days(7));
  
  for (const auto& task : tasks) {
      if (task.completed) {
          completedTasks++;
      } else {
          pendingTasks++;
          
          // Check if task is due within the next week
          wxDateTime dueDate;
          dueDate.ParseISODate(task.dueDate);
          
          if (dueDate.IsValid() && dueDate <= nextWeek) {
              urgentTasks++;
          }
      }
  }
  
  totalTasksText->SetLabel(wxString::Format("%d", totalTasks));
  completedTasksText->SetLabel(wxString::Format("%d", completedTasks));
  pendingTasksText->SetLabel(wxString::Format("%d", pendingTasks));
  urgentTasksText->SetLabel(wxString::Format("%d", urgentTasks));
  
  // Display recent tasks
  DisplayRecentTasks();
}

void MainFrame::LoadTasks() {
  tasks = dbManager->GetAllTasks(userManager->GetCurrentUser()->id);
  DisplayTasks();
}

void MainFrame::LoadCategories() {
  categories = categoryManager->GetAllCategories(userManager->GetCurrentUser()->id);
  
  // Update category combo in tasks panel
  categoryCombo->Clear();
  
  // Add "No Category" option
  categoryCombo->Append("No Category", (void*)-1);
  
  for (const auto& category : categories) {
      categoryCombo->Append(category.name, (void*)(wxIntPtr)category.id);
  }
  
  categoryCombo->SetSelection(0); // Default to "No Category"
}

void MainFrame::DisplayTasks() {
  // Clear grid
  if (tasksGrid->GetNumberRows() > 0) {
      tasksGrid->DeleteRows(0, tasksGrid->GetNumberRows());
  }
  
  // Add tasks to grid
  tasksGrid->AppendRows(tasks.size());
  
  for (size_t i = 0; i < tasks.size(); ++i) {
      const Task& task = tasks[i];
      
      tasksGrid->SetCellValue(i, 0, wxString::Format("%d", task.id));
      tasksGrid->SetCellValue(i, 1, task.title);
      tasksGrid->SetCellValue(i, 2, task.dueDate);
      tasksGrid->SetCellValue(i, 3, wxString::Format("%d", task.priority));
      tasksGrid->SetCellValue(i, 4, task.categoryName);
      tasksGrid->SetCellValue(i, 5, task.completed ? "Yes" : "No");
      
      // Apply background color based on category
      if (!task.categoryColor.IsEmpty()) {
          wxColour color;
          color.Set(task.categoryColor);
          tasksGrid->SetCellBackgroundColour(i, 4, color);
          
          // Set text color to ensure readability
          int brightness = (color.Red() * 299 + color.Green() * 587 + color.Blue() * 114) / 1000;
          if (brightness > 128) {
              tasksGrid->SetCellTextColour(i, 4, *wxBLACK);
          } else {
              tasksGrid->SetCellTextColour(i, 4, *wxWHITE);
          }
      }
      
      // Apply background color to completed tasks
      if (task.completed) {
          for (int col = 0; col < tasksGrid->GetNumberCols(); ++col) {
              // Skip category cell which already has a color
              if (col != 4 || task.categoryColor.IsEmpty()) {
                  tasksGrid->SetCellBackgroundColour(i, col, wxColour(240, 240, 240));
              }
          }
      }
      
      // Apply background color to high priority tasks (4-5)
      if (task.priority >= 4 && !task.completed) {
          tasksGrid->SetCellBackgroundColour(i, 3, wxColour(255, 200, 200));
      }
  }
  
  tasksGrid->AutoSize();
}

void MainFrame::DisplayRecentTasks() {
  recentTasksList->DeleteAllItems();
  
  // Sort tasks by due date (ascending)
  std::vector<Task> sortedTasks = tasks;
  std::sort(sortedTasks.begin(), sortedTasks.end(), [](const Task& a, const Task& b) {
      // Put completed tasks at the end
      if (a.completed != b.completed) {
          return !a.completed;
      }
      
      // Sort by due date
      return a.dueDate < b.dueDate;
  });
  
  // Display at most 10 most recent tasks
  int count = std::min(10, static_cast<int>(sortedTasks.size()));
  
  for (int i = 0; i < count; ++i) {
      const Task& task = sortedTasks[i];
      
      long itemIndex = recentTasksList->InsertItem(i, task.title);
      recentTasksList->SetItem(itemIndex, 1, task.dueDate);
      recentTasksList->SetItem(itemIndex, 2, wxString::Format("%d", task.priority));
      recentTasksList->SetItem(itemIndex, 3, task.categoryName);
      recentTasksList->SetItem(itemIndex, 4, task.completed ? "Completed" : "Pending");
      
      // Store task ID in the item data
      recentTasksList->SetItemData(itemIndex, task.id);
      
      // Set item color
      if (task.completed) {
          recentTasksList->SetItemTextColour(itemIndex, wxColour(0, 128, 0)); // Green for completed
      } else {
          // Determine urgency based on due date
          wxDateTime dueDate;
          wxDateTime today = wxDateTime::Today();
          dueDate.ParseISODate(task.dueDate);
          
          if (dueDate.IsValid()) {
              if (dueDate < today) {
                  recentTasksList->SetItemTextColour(itemIndex, wxColour(255, 0, 0)); // Red for overdue
              } else if (dueDate <= today.Add(wxDateSpan::Days(3))) {
                  recentTasksList->SetItemTextColour(itemIndex, wxColour(255, 128, 0)); // Orange for urgent
              }
          }
      }
  }
}

void MainFrame::ClearForm() {
  titleCtrl->Clear();
  descriptionCtrl->Clear();
  dueDateCtrl->SetValue(wxDateTime::Today());
  priorityCtrl->SetValue(1);
  completedCtrl->SetValue(false);
  categoryCombo->SetSelection(0); // Set to "No Category"
  
  selectedTaskId = -1;
  updateButton->Disable();
  deleteButton->Disable();
  addButton->Enable();
}

void MainFrame::OnTaskSelect(wxGridEvent& event) {
  int row = event.GetRow();
  
  // Add bounds checking
  if (row < 0 || row >= static_cast<int>(tasks.size())) {
      event.Skip();
      return;
  }
  
  const Task& task = tasks[row];
  
  selectedTaskId = task.id;
  titleCtrl->SetValue(task.title);
  descriptionCtrl->SetValue(task.description);
  
  wxDateTime dueDate;
  dueDate.ParseISODate(task.dueDate);
  dueDateCtrl->SetValue(dueDate);
  
  priorityCtrl->SetValue(task.priority);
  completedCtrl->SetValue(task.completed);
  
  // Select appropriate category
  int selection = 0; // Default to "No Category"
  for (size_t i = 0; i < categories.size(); ++i) {
      if (categories[i].id == task.categoryId) {
          selection = i + 1; // +1 because "No Category" is at index 0
          break;
      }
  }
  categoryCombo->SetSelection(selection);
  
  updateButton->Enable();
  deleteButton->Enable();
  addButton->Disable();
  
  event.Skip();
}

void MainFrame::OnAddTask(wxCommandEvent& event) {
  wxString title = titleCtrl->GetValue().Trim();
  wxString description = descriptionCtrl->GetValue().Trim();
  wxDateTime dueDate = dueDateCtrl->GetValue();
  int priority = priorityCtrl->GetValue();
  
  // Get selected category ID
  int categoryId = -1;
  int selection = categoryCombo->GetSelection();
  if (selection != wxNOT_FOUND) {
      categoryId = (int)(wxIntPtr)categoryCombo->GetClientData(selection);
  }
  
  if (title.IsEmpty()) {
      wxMessageBox("Title cannot be empty.", "Validation Error", wxOK | wxICON_ERROR);
      return;
  }
  
  wxString dueDateStr = dueDate.FormatISODate();
  
  if (dbManager->AddTask(title, description, dueDateStr, priority, categoryId, 
                       userManager->GetCurrentUser()->id)) {
      wxMessageBox("Task added successfully.", "Success", wxOK | wxICON_INFORMATION);
      ClearForm();
      LoadTasks();
      UpdateDashboardStatistics();
  }
}

void MainFrame::OnUpdateTask(wxCommandEvent& event) {
  if (selectedTaskId < 0) {
      return;
  }
  
  wxString title = titleCtrl->GetValue().Trim();
  wxString description = descriptionCtrl->GetValue().Trim();
  wxDateTime dueDate = dueDateCtrl->GetValue();
  int priority = priorityCtrl->GetValue();
  bool completed = completedCtrl->GetValue();
  
  // Get selected category ID
  int categoryId = -1;
  int selection = categoryCombo->GetSelection();
  if (selection != wxNOT_FOUND) {
      categoryId = (int)(wxIntPtr)categoryCombo->GetClientData(selection);
  }
  
  if (title.IsEmpty()) {
      wxMessageBox("Title cannot be empty.", "Validation Error", wxOK | wxICON_ERROR);
      return;
  }
  
  wxString dueDateStr = dueDate.FormatISODate();
  
  if (dbManager->UpdateTask(selectedTaskId, title, description, dueDateStr, priority, completed, categoryId)) {
      wxMessageBox("Task updated successfully.", "Success", wxOK | wxICON_INFORMATION);
      ClearForm();
      LoadTasks();
      UpdateDashboardStatistics();
  }
}

void MainFrame::OnDeleteTask(wxCommandEvent& event) {
  if (selectedTaskId < 0) {
      return;
  }
  
  wxMessageDialog dialog(this, "Are you sure you want to delete this task?", 
      "Confirm Deletion", wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
  
  if (dialog.ShowModal() == wxID_YES) {
      if (dbManager->DeleteTask(selectedTaskId)) {
          wxMessageBox("Task deleted successfully.", "Success", wxOK | wxICON_INFORMATION);
          ClearForm();
          LoadTasks();
          UpdateDashboardStatistics();
      }
  }
}

void MainFrame::OnTaskDetail(wxGridEvent& event) {
  int row = event.GetRow();
  
  if (row >= 0 && row < static_cast<int>(tasks.size())) {
      Task& task = tasks[row];
      
      TaskDetailDialog dlg(this, &task, dbManager, categoryManager, 
                         userManager->GetCurrentUser()->id);
      if (dlg.ShowModal() == wxID_OK) {
          LoadTasks();
          UpdateDashboardStatistics();
      }
  }
}

void MainFrame::OnExportTasks(wxCommandEvent& event) {
  wxFileDialog saveFileDialog(this, "Export Tasks", "", "",
                             "CSV files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
  
  if (saveFileDialog.ShowModal() == wxID_CANCEL) {
      return;
  }
  
  wxString filePath = saveFileDialog.GetPath();
  
  wxFileOutputStream output(filePath);
  if (!output.IsOk()) {
      wxLogError("Cannot save to file '%s'.", filePath);
      return;
  }
  
  wxTextOutputStream text(output);
  
  // Write header
  text << "ID,Title,Description,Due Date,Priority,Category,Completed\n";
  
  // Write tasks
  for (const auto& task : tasks) {
      // Format fields properly for CSV
      wxString title = task.title;
      title.Replace("\"", "\"\"");  // Escape quotes
      
      wxString description = task.description;
      description.Replace("\"", "\"\"");
      
      wxString categoryName = task.categoryName;
      categoryName.Replace("\"", "\"\"");
      
      text << task.id << ","
           << "\"" << title << "\","
           << "\"" << description << "\","
           << task.dueDate << ","
           << task.priority << ","
           << "\"" << categoryName << "\","
           << (task.completed ? "Yes" : "No") << "\n";
  }
  
  wxMessageBox(wxString::Format("Successfully exported %zu tasks to %s", 
                              tasks.size(), filePath), 
              "Export Complete", wxOK | wxICON_INFORMATION);
}

void MainFrame::OnImportTasks(wxCommandEvent& event) {
  wxFileDialog openFileDialog(this, "Import Tasks", "", "",
                             "CSV files (*.csv)|*.csv", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  
  if (openFileDialog.ShowModal() == wxID_CANCEL) {
      return;
  }
  
  wxString filePath = openFileDialog.GetPath();

  wxFileInputStream input(filePath);
  if (!input.IsOk()) {
      wxLogError("Cannot open file '%s'.", filePath);
      return;
  }

  wxTextInputStream text(input);

  // Add this warning message before processing the CSV
  wxMessageBox("Note: The CSV import has limitations with handling quoted fields containing commas. "
              "Please ensure your CSV file uses simple formatting.",
              "Import Notice", wxOK | wxICON_INFORMATION);
  
  // Skip header
  if (!input.Eof()) {
      wxString line = text.ReadLine();
  }
  
  int successCount = 0;
  int skipCount = 0;
  
  // Process each line
  while (!input.Eof()) {
      wxString line = text.ReadLine();
      if (line.IsEmpty()) continue;
      
      // Parse CSV line
      wxStringTokenizer tokenizer(line, ",", wxTOKEN_RET_EMPTY_ALL);
      
      // Skip ID field
      if (!tokenizer.HasMoreTokens()) continue;
      tokenizer.GetNextToken();
      
      // Title
      if (!tokenizer.HasMoreTokens()) continue;
      wxString title = tokenizer.GetNextToken();
      // Remove quotes if present
      if (title.StartsWith("\"") && title.EndsWith("\"")) {
          title = title.SubString(1, title.Length() - 2);
      }
      title.Replace("\"\"", "\"");  // Unescape quotes
      
      // Description
      if (!tokenizer.HasMoreTokens()) continue;
      wxString description = tokenizer.GetNextToken();
      // Remove quotes if present
      if (description.StartsWith("\"") && description.EndsWith("\"")) {
          description = description.SubString(1, description.Length() - 2);
      }
      description.Replace("\"\"", "\"");  // Unescape quotes
      
      // Due Date
      if (!tokenizer.HasMoreTokens()) continue;
      wxString dueDate = tokenizer.GetNextToken();
      
      // Priority
      if (!tokenizer.HasMoreTokens()) continue;
      wxString priorityStr = tokenizer.GetNextToken();
      long priority;
      if (!priorityStr.ToLong(&priority) || priority < 1 || priority > 5) {
          priority = 1;
      }
      
      // Category
      if (!tokenizer.HasMoreTokens()) continue;
      wxString categoryName = tokenizer.GetNextToken();
      // Remove quotes if present
      if (categoryName.StartsWith("\"") && categoryName.EndsWith("\"")) {
          categoryName = categoryName.SubString(1, categoryName.Length() - 2);
      }
      categoryName.Replace("\"\"", "\"");  // Unescape quotes
      
      // Find category ID or create new category
      int categoryId = -1;
      if (categoryName != "No Category") {
          bool found = false;
          for (const auto& category : categories) {
              if (category.name == categoryName) {
                  categoryId = category.id;
                  found = true;
                  break;
              }
          }
          
          if (!found) {
              // Create new category
              wxColour randomColor(rand() % 200 + 55, rand() % 200 + 55, rand() % 200 + 55);
              wxString colorStr = randomColor.GetAsString(wxC2S_HTML_SYNTAX);
              
              if (categoryManager->AddCategory(categoryName, colorStr, "", 
                                            userManager->GetCurrentUser()->id)) {
                  // Reload categories to get the new ID
                  LoadCategories();
                  for (const auto& category : categories) {
                      if (category.name == categoryName) {
                          categoryId = category.id;
                          break;
                      }
                  }
              }
          }
      }
      
      // Completed
      if (!tokenizer.HasMoreTokens()) continue;
      wxString completedStr = tokenizer.GetNextToken();
      bool completed = (completedStr == "Yes" || completedStr == "yes" || 
                       completedStr == "TRUE" || completedStr == "true" || 
                       completedStr == "1");
      
      // Add task to database
      if (dbManager->AddTask(title, description, dueDate, priority, categoryId, 
                           userManager->GetCurrentUser()->id)) {
          successCount++;
      } else {
          skipCount++;
      }
  }
  
  wxMessageBox(wxString::Format("Import complete. %d tasks imported, %d skipped.", 
                              successCount, skipCount), 
              "Import Results", wxOK | wxICON_INFORMATION);
  
  // Reload tasks
  LoadTasks();
  UpdateDashboardStatistics();
}

void MainFrame::OnLogout(wxCommandEvent& event) {
  wxMessageDialog confirmDlg(this, "Are you sure you want to logout?", 
                           "Confirm Logout", wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
  
  if (confirmDlg.ShowModal() == wxID_YES) {
      userManager->Logout();
      
      // Close current frame and show login dialog
      Hide();
      
      LoginDialog loginDlg(nullptr, userManager);
      if (loginDlg.ShowModal() == wxID_OK) {
          // Reload data for new user
          LoadCategories();
          LoadTasks();
          UpdateDashboardStatistics();
          
          // Update status bar
          SetStatusText(wxString::Format("Logged in as: %s", userManager->GetCurrentUser()->username));
          
          Show();
      } else {
          // User cancelled login, close application
          Close(true);
      }
  }
}

void MainFrame::OnProfile(wxCommandEvent& event) {
  ProfileDialog dlg(this, userManager);
  dlg.ShowModal();
}

void MainFrame::OnManageCategories(wxCommandEvent& event) {
  CategoryListDialog dlg(this, categoryManager, userManager);
  if (dlg.ShowModal() == wxID_OK) {
      // Reload categories and tasks (which may have updated category information)
      LoadCategories();
      LoadTasks();
  }
}

void MainFrame::OnExit(wxCommandEvent& event) {
  Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& event) {
  wxMessageBox("Task Manager Application\n\nA complete task management system with user authentication, task categories, and data import/export capabilities.\n\nVersion 1.0", 
              "About Task Manager", wxOK | wxICON_INFORMATION);
}

void MainFrame::OnSearchTasks(wxCommandEvent& event) {
  // Create a vector to store search results
  std::vector<Task> searchResults;
  
  // Show search dialog
  SearchDialog dlg(this, dbManager, categoryManager, userManager->GetCurrentUser()->id, &searchResults);
  
  if (dlg.ShowModal() == wxID_OK) {
      // Display search results
      tasks = searchResults;
      DisplayTasks();
      
      SetStatusText(wxString::Format("Search results: %zu tasks found", tasks.size()));
  }
}

