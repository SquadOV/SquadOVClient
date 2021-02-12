#pragma once

#include <string>
#include <sqlite3.h>
#include <sstream>

#include "shared/time.h"

namespace shared::sqlite {

class SqlStatement {
public:
    SqlStatement(sqlite3* db, const std::string& statement);

    ~SqlStatement();

    SqlStatement(const SqlStatement&) = delete;
    SqlStatement& operator=(const SqlStatement&) = delete;

    SqlStatement(SqlStatement&&) = default;
    SqlStatement& operator=(SqlStatement&&) = default;

    bool hasNext() const { return _hasNext && !fail(); }

    bool next();

    void bindNullParameter(int i) {
        sqlite3_bind_null(_stmt, i);
    }

    template<typename T>
    void bindParameter(int i, const T& param) {}

    template<>
    void bindParameter(int i, const int& param) {
        sqlite3_bind_int(_stmt, i, param);
    }

    template<>
    void bindParameter(int i, const std::string& param) {
        sqlite3_bind_text(_stmt, i, param.c_str(), -1, SQLITE_TRANSIENT);
    }

    template<typename T>
    T getColumn(int i) {}

    template<>
    int getColumn(int i) {
        return sqlite3_column_int(_stmt, i);
    }

    bool isColumnNull(int i) {
        return (sqlite3_column_type(_stmt, i) == SQLITE_NULL);
    }

    template<>
    std::string getColumn(int i) {
        const auto* txt = sqlite3_column_text(_stmt, i);
        return std::string((const char*)txt);
    }

    template<>
    long long getColumn(int i) {
        return static_cast<long long>(sqlite3_column_int64(_stmt, i));
    }

    shared::TimePoint getTimeColumnFromString(int i, const std::string& format) {
        const auto raw = getColumn<std::string>(i);
        return shared::strToTime(raw, format);
    }

    bool fail() const { return errorCode() != SQLITE_OK; }
    int errorCode() const { return _errorCode; }
    std::string errMsg() const { 
        std::ostringstream str;
        str << _errMsg << "[" << _errorCode << " - " << sqlite3_errstr(_errorCode) << "]" << std::endl
            << "\tStmt: " << _ogStatement;
        return str.str();
    }

protected:
    sqlite3* _db = nullptr;

private:
    std::string _ogStatement;
    sqlite3_stmt* _stmt = nullptr;
    int _errorCode = SQLITE_OK;
    bool _hasNext = true;
    std::string _errMsg;
};

class SqlTransaction {
public:
    SqlTransaction(sqlite3* db);

    void exec(SqlStatement& stmt);
    bool fail() const { return !_commit; }
    std::string errMsg() const { return _error; }

    ~SqlTransaction();
    SqlStatement createStatement(const std::string& str) {
        return SqlStatement(_db, str);
    }

private:
    sqlite3* _db = nullptr;
    bool _commit = true;
    std::string _error;

};

}