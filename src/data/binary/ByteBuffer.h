#pragma once

#include <cstdint>
#include <vector>

namespace eld::binary {

class ByteBuffer {
public:
    explicit ByteBuffer(
        const std::vector<std::uint8_t>& data
    );

    std::uint8_t readU8();
    std::uint16_t readU16();
    std::uint32_t readU24();
    std::uint32_t readU32();

    int readSignedSmart();
    int readUnsignedSmart();

    void skip(
        int bytes
    );

    int position() const;

    void setPosition(
        int position
    );

private:
    std::uint8_t peekU8() const;

private:
    std::vector<std::uint8_t> data_;
    int position_ = 0;
};

}
