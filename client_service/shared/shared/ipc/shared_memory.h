#pragma once

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <memory>
#include <string>

namespace shared::ipc {

enum class ShmemOp {
    Create,
    Open,
    Unknown
};

class SharedMemory {
public:
    explicit SharedMemory(const std::string& id);
    ~SharedMemory();

    void open();
    void create(size_t size);

    template<typename T>
    T* find(const std::string& id) {
        if (!_memory) {
            return nullptr;
        }
        return _memory->find<T>(id.c_str()).first;
    }

    template<typename T>
    void destroy(const std::string& id) {
        if (!_memory) {
            return;
        }
        _memory->destroy<T>(id.c_str());
    }

    // Helper typedefs
    using CharAllocator = boost::interprocess::allocator<char, boost::interprocess::managed_shared_memory::segment_manager>;
    using String = boost::interprocess::basic_string<char, std::char_traits<char>, CharAllocator>;

    // Helper allocation functions.
    String* createString(const std::string& id);

    template<typename T>
    T createAllocator() {
        return T(_memory->get_segment_manager());
    }
private:
    std::string _id;
    ShmemOp _op = ShmemOp::Unknown;

    std::unique_ptr<boost::interprocess::managed_shared_memory> _memory;
};

}