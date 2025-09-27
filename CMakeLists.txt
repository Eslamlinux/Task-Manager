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

