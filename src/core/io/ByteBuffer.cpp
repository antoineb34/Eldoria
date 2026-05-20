#include "ByteBuffer.h"

namespace rf::io {

ByteBuffer::ByteBuffer(
    const std::vector<uint8_t>& data
)
    : data_(data)
{
}

uint8_t ByteBuffer::readU8() {
    return data_[position_++];
}

uint16_t ByteBuffer::readU16() {

    uint16_t value =
        (data_[position_] << 8) |
        (data_[position_ + 1]);

    position_ += 2;

    return value;
}

uint32_t ByteBuffer::readU24() {

    uint32_t value =
        (data_[position_] << 16) |
        (data_[position_ + 1] << 8) |
        (data_[position_ + 2]);

    position_ += 3;

    return value;
}

int ByteBuffer::readSmart() {

    uint8_t peek =
        data_[position_];

    if (peek < 128) {
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
