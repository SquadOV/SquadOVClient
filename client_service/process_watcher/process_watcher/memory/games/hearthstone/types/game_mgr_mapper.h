#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/mono/types/nullable_mapper.h"
#include <memory>

namespace process_watcher::memory::games::hearthstone::types {

enum class GameType {
    GT_UNKNOWN = 0,
	GT_VS_AI = 1,
	GT_VS_FRIEND = 2,
	GT_TUTORIAL = 4,
	GT_ARENA = 5,
	GT_TEST_AI_VS_AI = 6,
	GT_RANKED = 7,
	GT_CASUAL = 8,
	GT_TAVERNBRAWL = 0x10,
	GT_TB_1P_VS_AI = 17,
	GT_TB_2P_COOP = 18,
	GT_FSG_BRAWL_VS_FRIEND = 19,
	GT_FSG_BRAWL = 20,
	GT_FSG_BRAWL_1P_VS_AI = 21,
	GT_FSG_BRAWL_2P_COOP = 22,
	GT_BATTLEGROUNDS = 23,
	GT_BATTLEGROUNDS_FRIENDLY = 24,
	GT_RESERVED_18_22 = 26,
	GT_RESERVED_18_23 = 27,
	GT_PVPDR_PAID = 28,
	GT_PVPDR = 29
};

using GameMgrMapperSPtr = std::shared_ptr<class GameMgrMapper>;
class GameMgrMapper {
public:
    explicit GameMgrMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);
    static const process_watcher::memory::mono::MonoClassMapper* klass(const process_watcher::memory::mono::MonoImageMapper& image);

    process_watcher::memory::mono::types::NullableMapper<int64_t> lastDeckId() const;
    GameType gameType() const;
    bool isSpectator() const;
private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
};

}