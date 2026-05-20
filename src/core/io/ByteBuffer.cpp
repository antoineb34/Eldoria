#include "ByteBuffer.h"

namespace rf::io {

ByteBuffer::ByteBuffer(
    const std::vector<char>& data
)
    : data_(data)
{
}

uint8_t ByteBuffer::readU8() {

    return static_cast<uint8_t>(
        data_[position_++]
    );
}

uint16_t ByteBuffer::readU16() {

    uint16_t value =
        (readU8() << 8) |
        readU8();

    return value;
}

uint32_t ByteBuffer::readU24() {

    uint32_t value =
        (readU8() << 16) |
        (readU8() << 8) |
        readU8();

    return value;
}

int ByteBuffer::readSmart() {

    uint8_t first =
        static_cast<uint8_t>(
            data_[position_]
        );

    if (first < 128) {
        return readU8() - 64;
    }

    return readU16() - 49152;
}

void ByteBuffer::skip(int bytes) {

    position_ += bytes;
}

int ByteBuffer::position() const {

    return position_;
}

void ByteBuffer::setPosition(int position) {

    position_ = position;
}

}
