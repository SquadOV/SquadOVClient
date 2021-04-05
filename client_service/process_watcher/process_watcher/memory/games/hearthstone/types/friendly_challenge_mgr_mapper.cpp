#include "process_watcher/memory/games/hearthstone/types/friendly_challenge_mgr_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string FULL_CLASS_NAME("FriendChallengeMgr");
const std::string SINGLETON_FIELD_NAME("s_instance");
const std::string DATA_FIELD_NAME("m_data");

}

FriendlyChallengeMgrMapper::FriendlyChallengeMgrMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

FriendlyChallengeMgrMapperSPtr FriendlyChallengeMgrMapper::singleton(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId) {
    const auto* cls = image.loadClassFromFullName(FULL_CLASS_NAME);
    const auto* field = cls->field(SINGLETON_FIELD_NAME);
    if (!field) {
        return nullptr;
    }
    const auto value = field->getStatic(domainId);
    if (value.isNull()) {
        return nullptr;
    }

    const auto object = value.get<mono::MonoObjectMapperSPtr>();
    return std::make_shared<FriendlyChallengeMgrMapper>(object);
}

FriendlyChallengeDataMapperSPtr FriendlyChallengeMgrMapper::data() const {
    const auto value = _object->get(DATA_FIELD_NAME);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<FriendlyChallengeDataMapper>(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
}

}