#include "process_watcher/memory/games/hearthstone/types/player_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string NAME_FIELD = "m_name";
const std::string LOCAL_FIELD = "m_local";
const std::string SIDE_FIELD = "m_side";
const std::string CARD_BACK_FIELD = "m_cardBackId";
const std::string MEDAL_INFO_FIELD = "m_medalInfo";
const std::string ARENA_WINS_FIELD = "m_arenaWins";
const std::string ARENA_LOSS_FIELD = "m_arenaLoss";
const std::string BRAWL_WIN_FIELD = "m_tavernBrawlWins";
const std::string BRAWL_LOSS_FIELD = "m_tavernBrawlLoss";
const std::string BNET_ACCOUNT_FIELD = "m_gameAccountId";

}

std::string playerSideToString(PlayerSide side) {
    switch (side) {
        case PlayerSide::Neutral:
            return "Neutral";
        case PlayerSide::Friendly:
            return "Friendly";
        case PlayerSide::Opposing:
            return "Opposing";
    }
    return "";
}

PlayerMapper::PlayerMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

std::string PlayerMapper::name() const {
    const auto value = _object->get(NAME_FIELD);
    return value.getDefault<std::string>("");
}

bool PlayerMapper::local() const {
    const auto value = _object->get(LOCAL_FIELD);
    return value.getDefault<uint8_t>(0);
}

PlayerSide PlayerMapper::side() const {
    const auto value = _object->get(SIDE_FIELD);
    return static_cast<PlayerSide>(value.getDefault<int32_t>(0));
}

int32_t PlayerMapper::cardBackId() const {
    const auto value = _object->get(CARD_BACK_FIELD);
    return value.getDefault<int32_t>(0);
}

MedalInfoTranslatorMapperSPtr PlayerMapper::medalInfo() const {
    const auto value = _object->get(MEDAL_INFO_FIELD);
    return std::make_shared<MedalInfoTranslatorMapper>(value.getDefault<process_watcher::memory::mono::MonoObjectMapperSPtr>(nullptr));
}

uint32_t PlayerMapper::arenaWins() const {
    const auto value = _object->get(ARENA_WINS_FIELD);
    return value.getDefault<uint32_t>(0);
}

uint32_t PlayerMapper::arenaLoss() const {
    const auto value = _object->get(ARENA_LOSS_FIELD);
    return value.getDefault<uint32_t>(0);
}

uint32_t PlayerMapper::tavernBrawlWins() const {
    const auto value = _object->get(BRAWL_WIN_FIELD);
    return value.getDefault<uint32_t>(0);
}

uint32_t PlayerMapper::tavernBrawlLoss() const {
    const auto value = _object->get(BRAWL_LOSS_FIELD);
    return value.getDefault<uint32_t>(0);
}

BNetAccountIdMapperSPtr PlayerMapper::gameAccountId() const {
    const auto value = _object->get(BNET_ACCOUNT_FIELD);
    return std::make_shared<BNetAccountIdMapper>(value.getDefault<process_watcher::memory::mono::MonoObjectMapperSPtr>(nullptr));
}

nlohmann::json PlayerMapper::toJson() const {
    return {
        { "name", name() },
        { "local", local() },
        { "side", static_cast<int>(side()) },
        { "cardBackId", static_cast<int>(cardBackId()) },
        { "medalInfo", medalInfo()->toJson() },
        { "arenaWins", arenaWins() },
        { "arenaLoss", arenaLoss() },
        { "tavernBrawlWins", tavernBrawlWins() },
        { "tavernBrawlLoss", tavernBrawlLoss() }
    };
}

std::ostream& operator<<(std::ostream& os, const PlayerMapper& map) {
    os << "Player: " << map.name()
        << std::endl << "\tLocal: " << map.local()
        << std::endl << "\tSide: " << playerSideToString(map.side())
        << std::endl << "\tCard Back ID: " << map.cardBackId()
        << std::endl << "\tMedal Info: " << *map.medalInfo()
        << std::endl << "\tArena WL: " << map.arenaWins() << "/" << map.arenaLoss()
        << std::endl << "\tTavern WL: " << map.tavernBrawlWins() << "/" << map.tavernBrawlLoss();
    return os;
}

}