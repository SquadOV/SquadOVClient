#include "process_watcher/memory/games/hearthstone/types/uber_text_mapper.h"
#include <string>

using namespace process_watcher::memory;

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string UBERTEXT_TEXT_FIELD_NAME("m_Text");

}

UberText::UberText(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {
}

std::string UberText::text() const {
    const auto value = _object->get(UBERTEXT_TEXT_FIELD_NAME);
    return value.get<std::string>();
}

}