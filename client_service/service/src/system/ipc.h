#pragma once
#include "shared/json.h"
#include <nlohmann/json.hpp>

namespace service::system {

template<typename T>
struct GenericIpcRequest {
    std::string task;
    T data;

    static GenericIpcRequest<T> fromJson(const nlohmann::json& json) {
        GenericIpcRequest<T> req;
        req.task = json["task"].get<std::string>();
        req.data = shared::json::JsonConverter<T>::from(json["data"]);
        return req;
    }
};

template<typename T>
struct GenericIpcResponse {
    std::string task;
    bool success;
    T data;

    nlohmann::json toJson() const {
        return {
            { "task", task },
            { "success", success },
            { "data", shared::json::JsonConverter<T>::to(data) }
        };
    }
};

template<>
struct GenericIpcResponse<void> {
    std::string task;
    bool success;

    nlohmann::json toJson() const {
        return {
            { "task", task },
            { "success", success }
        };
    }
};

}