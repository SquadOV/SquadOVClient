#pragma once

#include "process_watcher/memory/mono/mono_forward.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

enum class DeckType {
    ClientOnly = -1,
    Unknown = 0,
    Normal = 1,
    AI = 2,
    Draft = 4,
    Precon = 5,
    TavernBrawl = 6,
    FsgBrawl = 7,
    PvpDr = 8,
    PvpDrDisplay = 9,
    Hidden = 1000
};

std::string deckTypeToString(DeckType typ);

enum class TagPremium {
    Normal = 0,
    Golden = 1
};

std::string tagPremiumToString(TagPremium typ);

using CollectionDeckMapperSPtr = std::shared_ptr<class CollectionDeckMapper>;
class CollectionDeckMapper {
public:
    explicit CollectionDeckMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    std::string name() const;
    int64_t deckId() const;
    std::string heroCard() const;
    TagPremium heroPremium() const;
    DeckType deckType() const;
    uint64_t createDate() const;
    bool isWild() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

std::ostream& operator<<(std::ostream& os, const CollectionDeckMapper& map);

}