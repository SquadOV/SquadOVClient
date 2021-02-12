#include "aimlab/aimlab_db_interface.h"
#include "shared/sqlite/sqlite.h"
#include "shared/errors/error.h"
#include "shared/filesystem/utility.h"

#include <nlohmann/json.hpp>
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
    data.rawData = nlohmann::json::parse(stmt.getColumn<std::string>(7));
    return data;
}

}

AimlabDbInterface::AimlabDbInterface(const std::filesystem::path& path):
    _path(path) {
    LOG_INFO("Loading Aim Lab SQLite3 Database: " << _path << std::endl);
    if (sqlite3_open(shared::filesystem::pathUtf8(_path).c_str(), &_db) != SQLITE_OK) {
        THROW_ERROR("Failed to open database.");
    }
    LOG_INFO("...Success" << std::endl);
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

std::vector<shared::aimlab::TaskData> AimlabDbInterface::getAllTaskDataSince(const shared::TimePoint& tm) const {
    std::vector<shared::aimlab::TaskData> ret;

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
        WHERE createDate >= ?
        ORDER BY createDate ASC)|";

    shared::sqlite::SqlStatement stmt(_db, sql.str());
    stmt.bindParameter(1, shared::timeToStr(tm));

    if (stmt.fail()) {  
        THROW_ERROR("Failed to get all Aimlab task data: " << stmt.errMsg());
    }

    while(stmt.next()) {
        ret.emplace_back(getTaskDataFromSqlStatement(stmt));
    }

    return ret;
}

}