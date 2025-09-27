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

void wxSQLite3Statement::Bind(int paramIndex, const wxDateTime& value) {
    if (m_stmt == nullptr) {
        throw wxSQLite3Exception(SQLITE_ERROR, "Statement not valid");
    }
    
    wxString dateStr = value.FormatISODate();
    wxCharBuffer strBuffer = dateStr.ToUTF8();
    
    int rc = sqlite3_bind_text(m_stmt, paramIndex, strBuffer, -1, SQLITE_TRANSIENT);
    
    if (rc != SQLITE_OK) {
        const char* errmsg = sqlite3_errmsg(sqlite3_db_handle(m_stmt));
        throw wxSQLite3Exception(rc, wxString::FromUTF8(errmsg));
    }
}

void wxSQLite3Statement::BindNull(int paramIndex) {
    if (m_stmt == nullptr) {
        throw wxSQLite3Exception(SQLITE_ERROR, "Statement not valid");
    }
    
    int rc = sqlite3_bind_null(m_stmt, paramIndex);
    
    if (rc != SQLITE_OK) {
        const char* errmsg = sqlite3_errmsg(sqlite3_db_handle(m_stmt));
        throw wxSQLite3Exception(rc, wxString::FromUTF8(errmsg));
    }
}

int wxSQLite3Statement::ExecuteUpdate() {
    if (m_stmt == nullptr) {
        throw wxSQLite3Exception(SQLITE_ERROR, "Statement not valid");
    }
    
    int rc = sqlite3_step(m_stmt);
    
    if (rc != SQLITE_DONE) {
        const char* errmsg = sqlite3_errmsg(sqlite3_db_handle(m_stmt));
        throw wxSQLite3Exception(rc, wxString::FromUTF8(errmsg));
    }
    
    Reset();
    return sqlite3_changes(sqlite3_db_handle(m_stmt));
}

// Add implementation for ExecuteQuery method
wxSQLite3ResultSet wxSQLite3Statement::ExecuteQuery() {
    if (m_stmt == nullptr) {
        throw wxSQLite3Exception(SQLITE_ERROR, "Statement not valid");
    }
    
    // Don't call sqlite3_step here, just return a result set
    // The first call to NextRow() will advance to the first row
    return wxSQLite3ResultSet(m_stmt, m_db);
}

// wxSQLite3ResultSet implementation
wxSQLite3ResultSet::wxSQLite3ResultSet() : m_stmt(nullptr), m_db(nullptr), m_valid(false), m_eof(true) {
}

wxSQLite3ResultSet::wxSQLite3ResultSet(sqlite3_stmt* stmt, wxSQLite3Database* db)
    : m_stmt(stmt), m_db(db), m_valid(stmt != nullptr), m_eof(false) {
}

wxSQLite3ResultSet::wxSQLite3ResultSet(const wxSQLite3ResultSet& resultSet)
    : m_stmt(resultSet.m_stmt), m_db(resultSet.m_db), m_valid(resultSet.m_valid), m_eof(resultSet.m_eof) {
}

wxSQLite3ResultSet& wxSQLite3ResultSet::operator=(const wxSQLite3ResultSet& resultSet) {
    if (this != &resultSet) {
        Finalize();
        m_stmt = resultSet.m_stmt;
        m_db = resultSet.m_db;
        m_valid = resultSet.m_valid;
        m_eof = resultSet.m_eof;
    }
    return *this;
}

wxSQLite3ResultSet::~wxSQLite3ResultSet() {
    Finalize();
}

bool wxSQLite3ResultSet::NextRow() {
    if (m_stmt == nullptr || m_eof) {
        return false;
    }
    
    int rc = sqlite3_step(m_stmt);
    
    if (rc == SQLITE_ROW) {
        return true;
    } else if (rc == SQLITE_DONE) {
        m_eof = true;
        return false;
    } else {
        const char* errmsg = sqlite3_errmsg(sqlite3_db_handle(m_stmt));
        throw wxSQLite3Exception(rc, wxString::FromUTF8(errmsg));
    }
}

void wxSQLite3ResultSet::Finalize() {
    // Note: We don't finalize the statement here because it's owned by the statement object
    m_stmt = nullptr;
    m_valid = false;
    m_eof = true;
}

int wxSQLite3ResultSet::GetColumnCount() const {
    if (m_stmt == nullptr) {
        throw wxSQLite3Exception(SQLITE_ERROR, "Result set not valid");
    }
    
    return sqlite3_column_count(m_stmt);
}

wxString wxSQLite3ResultSet::GetColumnName(int columnIndex) const {
    if (m_stmt == nullptr) {
        throw wxSQLite3Exception(SQLITE_ERROR, "Result set not valid");
    }
    
    const char* name = sqlite3_column_name(m_stmt, columnIndex);
    
    if (name == nullptr) {
        throw wxSQLite3Exception(SQLITE_ERROR, "Invalid column index");
    }
    
    return wxString::FromUTF8(name);
}

wxString wxSQLite3ResultSet::GetAsString(int columnIndex) const {
    if (m_stmt == nullptr) {
        throw wxSQLite3Exception(SQLITE_ERROR, "Result set not valid");
    }
    
    const unsigned char* text = sqlite3_column_text(m_stmt, columnIndex);
    
    if (text == nullptr) {
        return wxEmptyString;
    }
    
    return wxString::FromUTF8(reinterpret_cast<const char*>(text));
}

int wxSQLite3ResultSet::GetAsInt(int columnIndex) const {
    if (m_stmt == nullptr) {
        throw wxSQLite3Exception(SQLITE_ERROR, "Result set not valid");
    }
    
    return sqlite3_column_int(m_stmt, columnIndex);
}

double wxSQLite3ResultSet::GetAsDouble(int columnIndex) const {
    if (m_stmt == nullptr) {
        throw wxSQLite3Exception(SQLITE_ERROR, "Result set not valid");
    }
    
    return sqlite3_column_double(m_stmt, columnIndex);
}

wxDateTime wxSQLite3ResultSet::GetAsDateTime(int columnIndex) const {
    wxString dateStr = GetAsString(columnIndex);
    wxDateTime date;
    
    if (!dateStr.IsEmpty()) {
        date.ParseISODate(dateStr);
    }
    
    return date;
}

bool wxSQLite3ResultSet::GetAsBool(int columnIndex) const {
    return GetAsInt(columnIndex) != 0;
}

bool wxSQLite3ResultSet::IsNull(int columnIndex) const {
    if (m_stmt == nullptr) {
        throw wxSQLite3Exception(SQLITE_ERROR, "Result set not valid");
    }
    
    return sqlite3_column_type(m_stmt, columnIndex) == SQLITE_NULL;
}

// wxSQLite3Database implementation
wxSQLite3Database::wxSQLite3Database() : m_db(nullptr), m_isOpen(false) {
}

wxSQLite3Database::~wxSQLite3Database() {
    Close();
}

void wxSQLite3Database::Open(const wxString& fileName) {
    if (m_isOpen) {
        Close();
    }
    
    wxCharBuffer fileNameBuffer = fileName.ToUTF8();
    int rc = sqlite3_open(fileNameBuffer, &m_db);
    
    if (rc != SQLITE_OK) {
        wxString errmsg = wxString::FromUTF8(sqlite3_errmsg(m_db));
        sqlite3_close(m_db);
        m_db = nullptr;
        throw wxSQLite3Exception(rc, errmsg);
    }
    
    m_isOpen = true;
}
