#pragma once
#include <vector>
namespace shared::memory {

class ByteWriter {
public:
    explicit ByteWriter(std::vector<char>& buffer);

    template<typename T>
    void writeData(const T& obj) {
        writeGenericData(&obj, sizeof(T));
    }

    template<>
    void writeData(const std::vector<char>& data) {
        writeGenericData((const void*)data.data(), data.size());
    }

private:
    std::vector<char>& _buffer;
    void writeGenericData(const void* data, size_t bytes);

    size_t _index = 0;
};

}