#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/mono/types/list_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {

enum class TagPremium {
    Normal = 0,
    Golden = 1
};

std::string tagPremiumToString(TagPremium typ);

using CollectionDeckSlotMapperSPtr = std::shared_ptr<class CollectionDeckSlotMapper>;
class CollectionDeckSlotMapper {
public:
    explicit CollectionDeckSlotMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    int32_t index() const;
    std::string cardId() const;
    bool owned() const;
    int32_t count() const;
    int32_t count(TagPremium premium) const;

    nlohmann::json toJson() const;
private:
    process_watcher::memory::mono::types::ListMapper<int32_t> allCounts() const;

    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

std::ostream& operator<<(std::ostream& os, const CollectionDeckSlotMapper& map);

}