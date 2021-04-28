#include "shared/ipc/shared_memory.h"

namespace bi = boost::interprocess;
namespace shared::ipc {

SharedMemory::SharedMemory(const std::string& id):
    _id(id) {
}

SharedMemory::~SharedMemory() {
    if (_op == ShmemOp::Create) {
        bi::shared_memory_object::remove(_id.c_str());
    }
}

void SharedMemory::open() {
    _op = ShmemOp::Open;
    _memory = std::make_unique<bi::managed_shared_memory>(bi::open_only, _id.c_str());
}

void SharedMemory::create(size_t size) {
    bi::shared_memory_object::remove(_id.c_str());
    _op = ShmemOp::Create;
    _memory = std::make_unique<bi::managed_shared_memory>(bi::create_only, _id.c_str(), size);
}

SharedMemory::String* SharedMemory::createString(const std::string& id) {
    if (!_memory) {
        return nullptr;
    }
    return _memory->construct<String>(id.c_str())("", _memory->get_segment_manager());
}

}