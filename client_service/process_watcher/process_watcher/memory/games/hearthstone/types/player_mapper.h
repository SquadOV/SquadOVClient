#pragma once

#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/medal_info_translator_mapper.h"
#include "process_watcher/memory/games/hearthstone/types/bnet_account_id_mapper.h"
#include <memory>
#include <nlohmann/json.hpp>

namespace process_watcher::memory::games::hearthstone::types {

enum class PlayerSide {
    Neutral = 0,
    Friendly = 1,
    Opposing = 2
};

std::string playerSideToString(PlayerSide side);

using PlayerMapperSPtr = std::shared_ptr<class PlayerMapper>;
class PlayerMapper {
public:
    explicit PlayerMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

    std::string name() const;
    bool local() const;
    PlayerSide side() const;
    int32_t cardBackId() const;
    MedalInfoTranslatorMapperSPtr medalInfo() const;
    uint32_t arenaWins() const;
    uint32_t arenaLoss() const;
    uint32_t tavernBrawlWins() const;
    uint32_t tavernBrawlLoss() const;
    BNetAccountIdMapperSPtr gameAccountId() const;

    nlohmann::json toJson() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

std::ostream& operator<<(std::ostream& os, const PlayerMapper& map);

}