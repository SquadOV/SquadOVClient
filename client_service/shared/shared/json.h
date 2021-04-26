#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
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

template<typename T>
struct JsonConverter<std::optional<T>> {
    static std::optional<T> from(const nlohmann::json& v) {
        std::optional<T> ret;
        if (!v.is_null()) {
            ret = JsonConverter<T>::from(v);
        }
        return ret;
    }

    static std::optional<T> fromKey(const nlohmann::json& v, const std::string& key) {
        std::optional<T> ret;
        if (v.contains(key)) {
            ret = JsonConverter<T>::from(v[key]);
        }
        return ret;
    }

    static nlohmann::json to(const std::optional<T>& v) {
        if (v) {
            return v.value();
        } else {
            return nlohmann::json{};
        }
    }
};
}