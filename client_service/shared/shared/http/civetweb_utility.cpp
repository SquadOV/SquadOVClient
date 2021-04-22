#include "shared/http/civetweb_utility.h"
#include <sstream>

namespace shared::http {

void writeCivetStatusResponse(HttpStatusCode code, struct mg_connection* conn) {
    mg_printf(conn, "HTTP/1.1 %d %s\r\n", static_cast<int>(code), statusCodeToString(code).c_str());
}

std::string statusCodeToString(HttpStatusCode code) {
    switch (code) {
        case HttpStatusCode::NoContent:
            return "No Content";
    }
    return "Unknown";
}

nlohmann::json readCivetJsonBody(struct mg_connection* conn) {
    std::ostringstream rawJson;

    char buffer[2048];
    auto read = mg_read(conn, buffer, sizeof(buffer));
    while (read > 0) {
        std::string tmp(buffer, read);
        rawJson << tmp;
        read = mg_read(conn, buffer, sizeof(buffer));
    }

    return nlohmann::json::parse(rawJson.str());
}

}