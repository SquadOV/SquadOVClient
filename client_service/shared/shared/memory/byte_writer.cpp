#include "shared/memory/byte_writer.h"

namespace shared::memory {

ByteWriter::ByteWriter(std::vector<char>& buffer):
    _buffer(buffer)
{}

void ByteWriter::writeGenericData(const void* data, size_t bytes) {
    std::memcpy(&_buffer[_index], data, bytes);
    _index += bytes;
}

}