#include "process_watcher/memory/games/hearthstone/types/bnet_presence_mgr_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string FULL_CLASS_NAME("BnetPresenceMgr");
const std::string SINGLETON_FIELD_NAME("s_instance");
const std::string MYPLAYER_FIELD_NAME("m_myPlayer");

}

BNetPresenceMgrMapper::BNetPresenceMgrMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {

}

BNetPresenceMgrMapperSPtr BNetPresenceMgrMapper::singleton(const process_watcher::memory::mono::MonoImageMapper& image, int32_t domainId) {
    const auto* cls = image.loadClassFromFullName(FULL_CLASS_NAME);
    const auto& field = cls->field(SINGLETON_FIELD_NAME);
    const auto value = field.getStatic(domainId);
    if (value.isNull()) {
        return nullptr;
    }

    const auto object = value.get<mono::MonoObjectMapperSPtr>();
    return std::make_shared<BNetPresenceMgrMapper>(object);
}

BNetPlayerMapperSPtr BNetPresenceMgrMapper::myPlayer() const {
    const auto value = _object->get(MYPLAYER_FIELD_NAME);
    if (value.isNull()) {
        return nullptr;
    }
    return std::make_shared<BNetPlayerMapper>(value.get<process_watcher::memory::mono::MonoObjectMapperSPtr>());
}

}