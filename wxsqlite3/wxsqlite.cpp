#include "wxsqlite3.h"

// wxSQLite3Statement implementation
wxSQLite3Statement::wxSQLite3Statement() : m_stmt(nullptr), m_db(nullptr), m_valid(false) {
}

wxSQLite3Statement::wxSQLite3Statement(sqlite3_stmt* stmt, wxSQLite3Database* db)
    : m_stmt(stmt), m_db(db), m_valid(stmt != nullptr) {
}

wxSQLite3Statement::wxSQLite3Statement(const wxSQLite3Statement& statement)
    : m_stmt(statement.m_stmt), m_db(statement.m_db), m_valid(statement.m_valid) {
}

wxSQLite3Statement& wxSQLite3Statement::operator=(const wxSQLite3Statement& statement) {
    if (this != &statement) {
        Finalize();
        m_stmt = statement.m_stmt;
        m_db = statement.m_db;
        m_valid = statement.m_valid;
    }
    return *this;
}

wxSQLite3Statement& wxSQLite3Statement::operator=(sqlite3_stmt* stmt) {
    Finalize();
    m_stmt = stmt;
    m_valid = (m_stmt != nullptr);
    return *this;
}

wxSQLite3Statement::~wxSQLite3Statement() {
    Finalize();
}

void wxSQLite3Statement::Reset() {
    if (m_stmt != nullptr) {
        int rc = sqlite3_reset(m_stmt);
        if (rc != SQLITE_OK) {
            const char* errmsg = sqlite3_errmsg(sqlite3_db_handle(m_stmt));
            throw wxSQLite3Exception(rc, wxString::FromUTF8(errmsg));
        }
    }
}

void wxSQLite3Statement::Finalize() {
    if (m_stmt != nullptr) {
        sqlite3_finalize(m_stmt);
        m_stmt = nullptr;
    }
    m_valid = false;
}

void wxSQLite3Statement::Bind(int paramIndex, const wxString& value) {
    if (m_stmt == nullptr) {
        throw wxSQLite3Exception(SQLITE_ERROR, "Statement not valid");
    }
    
    wxCharBuffer strBuffer = value.ToUTF8();
    int rc = sqlite3_bind_text(m_stmt, paramIndex, strBuffer, -1, SQLITE_TRANSIENT);
    
    if (rc != SQLITE_OK) {
        const char* errmsg = sqlite3_errmsg(sqlite3_db_handle(m_stmt));
        throw wxSQLite3Exception(rc, wxString::FromUTF8(errmsg));
    }
}

void wxSQLite3Statement::Bind(int paramIndex, int value) {
    if (m_stmt == nullptr) {
        throw wxSQLite3Exception(SQLITE_ERROR, "Statement not valid");
    }
    
    int rc = sqlite3_bind_int(m_stmt, paramIndex, value);
    
    if (rc != SQLITE_OK) {
        const char* errmsg = sqlite3_errmsg(sqlite3_db_handle(m_stmt));
        throw wxSQLite3Exception(rc, wxString::FromUTF8(errmsg));
    }
}

void wxSQLite3Statement::Bind(int paramIndex, double value) {
    if (m_stmt == nullptr) {
        throw wxSQLite3Exception(SQLITE_ERROR, "Statement not valid");
    }
    
    int rc = sqlite3_bind_double(m_stmt, paramIndex, value);
    
    if (rc != SQLITE_OK) {
        const char* errmsg = sqlite3_errmsg(sqlite3_db_handle(m_stmt));
        throw wxSQLite3Exception(rc, wxString::FromUTF8(errmsg));
    }
}

