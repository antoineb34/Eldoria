#pragma once

#include <cstdint>
#include <vector>

namespace rf::io {

class ByteBuffer {
public:
    explicit ByteBuffer(const std::vector<uint8_t>& data);

    uint8_t readU8();
    uint16_t readU16();
    uint32_t readU24();
    uint32_t readU32();

    int readSignedSmart();
    int readUnsignedSmart();

    void skip(int bytes);

    int position() const;
    void setPosition(int position);

private:
    uint8_t peekU8() const;

    std::vector<uint8_t> data_;
    int position_ = 0;
};

}