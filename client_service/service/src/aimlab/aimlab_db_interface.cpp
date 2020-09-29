#include "aimlab/aimlab_db_interface.h"
#include "shared/sqlite/sqlite.h"
#include "shared/errors/error.h"

#include <sstream>
#include <iostream>

using namespace shared::sqlite;
namespace service::aimlab {
namespace {

shared::aimlab::TaskData getTaskDataFromSqlStatement(SqlStatement& stmt) {
    shared::aimlab::TaskData data;
    data.taskId = stmt.getColumn<int>(0);
    data.klutchId = stmt.getColumn<std::string>(1);
    data.taskName = stmt.getColumn<std::string>(2);
    data.createDate = stmt.getTimeColumnFromString(3, "%Y-%m-%d %H:%M:%S");
    data.mode = stmt.getColumn<int>(4);
    data.score = stmt.getColumn<int>(5);
    data.version = stmt.getColumn<std::string>(6);
    data.rawData = stmt.getColumn<std::string>(7);
    return data;
}

}

AimlabDbInterface::AimlabDbInterface(const std::filesystem::path& path):
    _path(path) {
    if (sqlite3_open(_path.string().c_str(), &_db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database.");
    }
}

AimlabDbInterface::~AimlabDbInterface() {
    sqlite3_close(_db);
}

shared::aimlab::TaskData AimlabDbInterface::getLatestTaskData() const {
    std::ostringstream sql;
    sql << R"|(
        SELECT
            taskId,
            klutchId,
            taskName,
            createDate,
            mode,
            score,
            aimlab_version,
            performance
        FROM TaskData
        ORDER BY taskId DESC
        LIMIT 1)|";

    shared::sqlite::SqlStatement stmt(_db, sql.str());
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to get latest task data for Aim Lab: " << stmt.errMsg());
    }

    return getTaskDataFromSqlStatement(stmt);
}

shared::aimlab::TaskData AimlabDbInterface::getTaskDataFromId(int id) const {
    std::ostringstream sql;
    sql << R"|(
        SELECT
            taskId,
            klutchId,
            taskName,
            createDate,
            mode,
            score,
            aimlab_version,
            performance
        FROM TaskData
        WHERE taskId = ?)|";

    shared::sqlite::SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, id);
    stmt.next();

    if (stmt.fail()) {
        THROW_ERROR("Failed to get latest task data for Aim Lab: " << stmt.errMsg());
    }

    return getTaskDataFromSqlStatement(stmt);
}

std::vector<int> AimlabDbInterface::getAllTaskDataId() const {
    std::ostringstream sql;
    sql << R"|(
        SELECT
            taskId,
            klutchId,
            taskName,
            createDate,
            mode,
            score,
            aimlab_version,
            performance
        FROM TaskData
        ORDER BY taskId DESC)|";

    std::vector<int> ret;
    SqlStatement stmt(_db, sql.str());

    while (stmt.hasNext()) {
        if (!stmt.next()) {
            break;
        }

        if (stmt.fail()) {
            THROW_ERROR("Failed to get task ids: " << stmt.errMsg());
        }

        ret.push_back(stmt.getColumn<int>(0));
    }
    return ret;
}

}