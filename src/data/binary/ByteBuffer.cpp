#include "ByteBuffer.h"

namespace eld::binary {

ByteBuffer::ByteBuffer(
    const std::vector<std::uint8_t>& data
)
    : data_(data)
{
}

std::uint8_t ByteBuffer::readU8() {
    return data_[position_++];
}

std::uint16_t ByteBuffer::readU16() {
    std::uint16_t value =
        static_cast<std::uint16_t>(
            readU8()
        ) << 8;

    value |=
        static_cast<std::uint16_t>(
            readU8()
        );

    return value;
}

std::uint32_t ByteBuffer::readU24() {
    std::uint32_t value =
        static_cast<std::uint32_t>(
            readU8()
        ) << 16;

    value |=
        static_cast<std::uint32_t>(
            readU8()
        ) << 8;

    value |=
        static_cast<std::uint32_t>(
            readU8()
        );

    return value;
}

std::uint32_t ByteBuffer::readU32() {
    std::uint32_t value =
        static_cast<std::uint32_t>(
            readU8()
        ) << 24;

    value |=
        static_cast<std::uint32_t>(
            readU8()
        ) << 16;

    value |=
        static_cast<std::uint32_t>(
            readU8()
        ) << 8;

    value |=
        static_cast<std::uint32_t>(
            readU8()
        );

    return value;
}

int ByteBuffer::readSignedSmart() {
    std::uint8_t peek =
        peekU8();

    if (peek < 128) {
        return
            static_cast<int>(
                readU8()
            ) - 64;
    }

    return
        static_cast<int>(
            readU16()
        ) - 49152;
}

int ByteBuffer::readUnsignedSmart() {
    std::uint8_t peek =
        peekU8();

    if (peek < 128) {
        return
            static_cast<int>(
                readU8()
            );
    }

    return
        static_cast<int>(
            readU16()
        ) - 32768;
}

void ByteBuffer::skip(
    int bytes
) {
    position_ += bytes;
}

int ByteBuffer::position() const {
    return position_;
}

void ByteBuffer::setPosition(
    int position
) {
    position_ = position;
}

std::uint8_t ByteBuffer::peekU8() const {
    return data_[position_];
}

}
