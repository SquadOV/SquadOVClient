#pragma once

#include "shared/json.h"
#include <civetweb.h>
#include <nlohmann/json.hpp>
#include <string>

namespace shared::http {

enum class HttpStatusCode {
    NoContent = 204    
};

void writeCivetStatusResponse(HttpStatusCode code, struct mg_connection* conn);
std::string statusCodeToString(HttpStatusCode code);

nlohmann::json readCivetJsonBody(struct mg_connection* conn);

template<typename T>
T readTypedCivetJsonBody(struct mg_connection* conn) {
    return shared::json::JsonConverter<T>::from(readCivetJsonBody(conn));
}

}