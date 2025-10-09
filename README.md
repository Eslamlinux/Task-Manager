# Task Manager

Task Manager is a modern, cross-platform desktop application for managing tasks, categories, and user profiles. Built with C++ using wxWidgets for the GUI and SQLite for persistent storage, it provides a robust and user-friendly experience for individuals and teams.

## Features

- **User Authentication & Registration**
  - Secure login and registration dialogs.
  - Password hashing for user security.
  - User profile management (edit name, email, password).

- **Task Management**
  - Create, edit, delete, and view tasks.
  - Tasks include title, description, due date, priority, completion status, and category.
  - Advanced sorting and filtering options.

- **Category Management**
  - Organize tasks into customizable categories.
  - Assign colors and descriptions to categories.
  - Category CRUD operations with user ownership.

- **Profile Management**
  - Edit user details and change password.
  - View profile information in a dedicated dialog.

- **Database Integration**
  - Uses SQLite for reliable, local data storage.
  - wxSQLite3 wrapper for seamless database operations.

- **Modern UI**
  - Built with wxWidgets for a native look and feel.
  - Dialogs for login, registration, profile, category, and task management.
  - Responsive layouts and intuitive controls.

## Architecture

- **C++17** standard for modern language features.
- **wxWidgets** for cross-platform GUI.
- **SQLite** for embedded database.
- **Modular Design**: Separate classes for user, category, and task management.
- **Custom wxSQLite3 Wrapper**: Simplifies database access and error handling.

## Getting Started

### Prerequisites

- CMake >= 3.10
- wxWidgets (core, base, adv, net components)
- SQLite3 development libraries

### Build Instructions

```bash
git clone https://github.com/Eslamlinux/Task-Manager.git
cd Task-Manager
mkdir build && cd build
cmake ..
make
```

### Run

```bash
./TaskManager
```

## File Structure

- `main.cpp` — Application entry point and main window.
- `user_manager.*` — User authentication, registration, and profile logic.
- `category_manager.*` — Category CRUD and management.
- `category_dialog.*`, `category_list_dialog.*` — Category dialogs.
- `login_dialog.*`, `register_dialog.*` — User login and registration dialogs.
- `profile_dialog.*` — User profile dialog.
- `wxsqlite3/` — Custom SQLite3 wrapper for wxWidgets.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request.

## License

This project is licensed under the MIT License.

## Author

Developed by Eslamlinux.
