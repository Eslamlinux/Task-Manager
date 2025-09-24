#ifndef WXSQLITE3_H
#define WXSQLITE3_H

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/datetime.h>
#include <wx/buffer.h>
#include <wx/regex.h>
#include <wx/hashmap.h>

#include <sqlite3.h>

#include <map>
#include <vector>

// Forward declarations
class wxSQLite3Database;
class wxSQLite3Statement;
class wxSQLite3ResultSet;

// Exception class
class wxSQLite3Exception {
private:
    int m_errorCode;
    wxString m_errorMessage;

public:
    wxSQLite3Exception(int errorCode, const wxString& errorMessage)
        : m_errorCode(errorCode), m_errorMessage(errorMessage) {}

    int GetErrorCode() const { return m_errorCode; }
    wxString GetMessage() const { return m_errorMessage; }
};

// Result set class
class wxSQLite3ResultSet {
private:
    sqlite3_stmt* m_stmt;
    wxSQLite3Database* m_db;
    bool m_valid;
    bool m_eof;

    friend class wxSQLite3Database;
    friend class wxSQLite3Statement;

    wxSQLite3ResultSet(sqlite3_stmt* stmt, wxSQLite3Database* db);

public:
    wxSQLite3ResultSet();
    wxSQLite3ResultSet(const wxSQLite3ResultSet& resultSet);
    wxSQLite3ResultSet& operator=(const wxSQLite3ResultSet& resultSet);
    virtual ~wxSQLite3ResultSet();

    bool IsValid() const { return m_valid; }
    bool Eof() const { return m_eof; }
    bool NextRow();
    void Finalize();

    int GetColumnCount() const;
    wxString GetColumnName(int columnIndex) const;
    wxString GetAsString(int columnIndex) const;
    int GetAsInt(int columnIndex) const;
    double GetAsDouble(int columnIndex) const;
    wxDateTime GetAsDateTime(int columnIndex) const;
    bool GetAsBool(int columnIndex) const;
    bool IsNull(int columnIndex) const;
};

// Statement class
class wxSQLite3Statement {
private:
    sqlite3_stmt* m_stmt;
    wxSQLite3Database* m_db;
    bool m_valid;

    friend class wxSQLite3Database;

    wxSQLite3Statement(sqlite3_stmt* stmt, wxSQLite3Database* db);

public:
    wxSQLite3Statement();
    wxSQLite3Statement(const wxSQLite3Statement& statement);
    wxSQLite3Statement& operator=(const wxSQLite3Statement& statement);
    virtual ~wxSQLite3Statement();

    bool IsValid() const { return m_valid; }
    void Reset();
    void Finalize();

    void Bind(int paramIndex, const wxString& value);
    void Bind(int paramIndex, int value);
    void Bind(int paramIndex, double value);
    void Bind(int paramIndex, const wxDateTime& value);
    void BindNull(int paramIndex);

    int ExecuteUpdate();
    wxSQLite3ResultSet ExecuteQuery();
    wxSQLite3Statement& operator=(sqlite3_stmt* stmt);
};

// Database class
class wxSQLite3Database {
private:
    sqlite3* m_db;
    bool m_isOpen;

public:
    wxSQLite3Database();
    virtual ~wxSQLite3Database();

    void Open(const wxString& fileName);
    void Close();
    bool IsOpen() const { return m_isOpen; }

    wxSQLite3Statement PrepareStatement(const wxString& sql);
    wxSQLite3ResultSet ExecuteQuery(const wxString& sql);
    int ExecuteUpdate(const wxString& sql);
    
    sqlite3* GetDatabaseHandle() const { return m_db; }
};

#endif // WXSQLITE3_H
