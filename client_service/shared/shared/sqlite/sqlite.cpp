#include "shared/sqlite/sqlite.h"

namespace shared::sqlite {
    
SqlStatement::SqlStatement(sqlite3* db, const std::string& statement):
    _ogStatement(statement),
    _db(db) {
    _errorCode = sqlite3_prepare(
        _db,
        statement.c_str(),
        static_cast<int>(statement.size()),
        &_stmt,
        NULL
    );
    _errMsg = sqlite3_errmsg(_db);
}

SqlStatement::~SqlStatement() {
    sqlite3_finalize(_stmt);
}

bool SqlStatement::next() {
    if (!hasNext()) {
        return false;
    }

    const int code = sqlite3_step(_stmt);
    _hasNext = (code == SQLITE_ROW);
    if (code != SQLITE_DONE && code != SQLITE_OK && code != SQLITE_ROW) {
        _errorCode = code;
        _errMsg = sqlite3_errmsg(_db);
        return false;
    }
    return (code != SQLITE_DONE);
}

SqlTransaction::SqlTransaction(sqlite3* db):
    _db(db) {
    SqlStatement begin(_db, "BEGIN IMMEDIATE TRANSACTION;");
    begin.next();
}

void SqlTransaction::exec(SqlStatement& stmt) {
    if (!_commit) {
        return;
    }
    stmt.next();
    _commit = !stmt.fail();
    if (!_commit) {
        _error = stmt.errMsg();
    }
}

SqlTransaction::~SqlTransaction() {
    std::string endStmt;
    if (!_commit) {
        endStmt = "ROLLBACK;";
    } else {
        endStmt = "COMMIT;";
    }

    SqlStatement end(_db, endStmt);
    end.next();
}

}