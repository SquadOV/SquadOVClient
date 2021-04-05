#pragma once

#include <nlohmann/json.hpp>
#include <type_traits>

namespace shared::json {

template<typename T, typename D = void>
struct JsonConverter {};

template<typename T>
struct JsonConverter<T,
    typename std::enable_if_t<
        std::is_same_v<T, std::string> ||
        std::is_arithmetic_v<T>
    >
> {
    static T from(const nlohmann::json& v) {
        return v.get<T>();
    }

    static nlohmann::json to(const T& v) {
        return v;
    }
};

}