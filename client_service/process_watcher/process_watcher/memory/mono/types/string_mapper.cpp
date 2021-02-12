#include "process_watcher/memory/mono/types/string_mapper.h"

#include <codecvt>
#include <locale>

namespace process_watcher::memory::mono::types {
namespace {

const std::string STRING_LENGTH_FIELD_NAME("m_stringLength");
const std::string STRING_FIRSTCHAR_FIELD_NAME("m_firstChar");

}

StringMapper::StringMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {
    // There are two properties that we care about on the System.String object:
    // 1) m_firstChar
    // 2) m_stringLength
    // So the C-string representation of this object starts at the *address* of m_firstChar and
    // is m_stringLength long.
    const auto lenObj = object->get(STRING_LENGTH_FIELD_NAME);
    const auto len = lenObj.get<int32_t>();

    const auto firstCharObj = object->get(STRING_FIRSTCHAR_FIELD_NAME);
    const auto firstCharPtr = firstCharObj.ptr();

    std::u16string u16String;
    _object->memory()->readProcessMemory(u16String, firstCharPtr, static_cast<size_t>(len));

    // Convert from UTF-16 to UTF-8 for our own sanity. LOL :(
    #pragma warning(suppress: 4996)
    _value = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16String);
}

}