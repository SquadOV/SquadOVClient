#pragma once

#include "shared/json.h"
#include <iostream>

namespace shared::wow {

enum class InstanceType {
    NotInstanced,
    PartyDungeon,
    RaidDungeon,
    PVPBattlefield,
    ArenaBattlefield,
    Scenario,
    Unknown
};

inline
std::string instanceTypeToName(InstanceType t) {
    switch (t) {
        case InstanceType::NotInstanced:
            return "Not Instanced";
        case InstanceType::PartyDungeon:
            return "Party Dungeon";
        case InstanceType::RaidDungeon:
            return "Raid Dungeon";
        case InstanceType::PVPBattlefield:
            return "PVP Battlefield";
        case InstanceType::ArenaBattlefield:
            return "Arena Battlefield";
        case InstanceType::Scenario:
            return "Scenario";
        default:
            return "Unknown";
    }
}

struct TypedInstanceData {
    int64_t id;
    std::string name;
    int64_t expansion;
    int64_t loadingScreenId;
    InstanceType instanceType;
};

inline
std::ostream& operator<<(std::ostream& os, const TypedInstanceData& e) {
    os << "{"
       << "Id: " << e.id 
       << ", Name:" << e.name
       << ", Expansion: " << e.expansion
       << ", Loading Screen Id: " << e.loadingScreenId
       << ", Instance Type: " << instanceTypeToName(e.instanceType)
       << "}";
    return os;
}

struct InstanceData {
    std::string id;
    std::string name;
    std::string expansion;
    std::string loadingScreenId;
    std::string instanceType;

    TypedInstanceData toTyped() const;
};

inline
TypedInstanceData InstanceData::toTyped() const {
    TypedInstanceData data;
    data.id = std::stoll(id);
    data.name = name;
    data.expansion = std::stoll(expansion);
    data.loadingScreenId = std::stoll(loadingScreenId);
    data.instanceType = static_cast<InstanceType>(std::stoi(instanceType));
    return data;
}

}

namespace shared::json {

template<>
struct JsonConverter<shared::wow::InstanceData> {
    static shared::wow::InstanceData from(const nlohmann::json& v) {
        shared::wow::InstanceData data;
        data.id = JsonConverter<decltype(data.id)>::from(v["id"]);
        data.name = JsonConverter<decltype(data.name)>::from(v["name"]);
        data.expansion = JsonConverter<decltype(data.expansion)>::from(v["expansion"]);
        data.loadingScreenId = JsonConverter<decltype(data.loadingScreenId)>::from(v["loadingScreenId"]);
        data.instanceType = JsonConverter<decltype(data.instanceType)>::from(v["instanceType"]);
        return data;
    }
};

template<>
struct JsonConverter<shared::wow::TypedInstanceData> {
    static nlohmann::json to(const shared::wow::TypedInstanceData& v) {
        const nlohmann::json data = {
            { "id", JsonConverter<decltype(v.id)>::to(v.id) },
            { "name", JsonConverter<decltype(v.name)>::to(v.name) },
            { "expansion", JsonConverter<decltype(v.expansion)>::to(v.expansion) },
            { "loadingScreenId", JsonConverter<decltype(v.loadingScreenId)>::to(v.loadingScreenId) },
            { "instanceType", JsonConverter<int>::to(static_cast<int>(v.instanceType)) }
        };

        return data;
    }
};

}