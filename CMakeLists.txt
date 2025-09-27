cmake_minimum_required(VERSION 3.10)
project(TaskManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base adv net)
include(${wxWidgets_USE_FILE})

# Find SQLite3
find_package(SQLite3 REQUIRED)

# Add wxSQLite3 library
add_library(wxsqlite3 STATIC
  wxsqlite3/wxsqlite3.cpp
)

target_include_directories(wxsqlite3 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/wxsqlite3
  ${SQLite3_INCLUDE_DIRS}
)

target_link_libraries(wxsqlite3
  ${SQLite3_LIBRARIES}
  ${wxWidgets_LIBRARIES}
)

# Source files
set(SOURCES
  main.cpp
  user_manager.cpp
  login_dialog.cpp
  register_dialog.cpp
  category_manager.cpp
  category_dialog.cpp
  category_list_dialog.cpp
  profile_dialog.cpp
)

# Add executable
add_executable(TaskManager ${SOURCES})

target_link_libraries(TaskManager
  ${wxWidgets_LIBRARIES}
  wxsqlite3
)
