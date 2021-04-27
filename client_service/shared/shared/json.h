#pragma once

#include "shared/time.h"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

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

template<>
struct JsonConverter<shared::TimePoint> {
    static shared::TimePoint from(const nlohmann::json& v) {
        return shared::isoStrToTime(v.get<std::string>());
    }

    static nlohmann::json to(const shared::TimePoint& v) {
        return shared::timeToIso(v);
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
            return JsonConverter<T>::to(v.value());
        } else {
            return nlohmann::json{};
        }
    }
};

template<typename T>
struct JsonConverter<std::vector<T>> {
    static std::vector<T> from(const nlohmann::json& v) {
        std::vector<T> d(v.size());
        for (auto i = 0; i < v.size(); ++i) {
            d[i] = JsonConverter<T>::from(v[i]);
        }
        return d;
    }

    static nlohmann::json to(const std::vector<T>& v) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& k : v) {
            arr.push_back(JsonConverter<T>::to(k));
        }
        return arr;
    }
};


template<typename K, typename V>
struct JsonConverter<std::unordered_map<K, V>> {
    static std::unordered_map<K, V> from(const nlohmann::json& v) {
        std::unordered_map<K, V> ret;
        for (const auto& [key, value] : v.items()) {
            ret[key] = JsonConverter<V>::from(value);
        }
        return ret;
    }

    static nlohmann::json to(const std::unordered_map<K, V>& v) {
        nlohmann::json obj;
        for (const auto& kvp : v) {
            obj[kvp.first] = JsonConverter<V>::to(kvp.second);
        }
        return obj;
    }
};

}