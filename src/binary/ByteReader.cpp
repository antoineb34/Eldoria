#include "ByteReader.h"

#include <stdexcept>

namespace eld::binary {

ByteReader::ByteReader(
    std::span<const std::uint8_t> data
)
    : data_(data) {
}

ByteReader::ByteReader(
    const std::vector<std::uint8_t>& data
)
    : ByteReader(
          std::span<const std::uint8_t>(
              data.data(),
              data.size()
          )
      ) {
}

std::uint8_t ByteReader::readU8() {
    requireReadable(1);

    return data_[position_++];
}

std::uint16_t ByteReader::readU16() {
    requireReadable(2);

    const std::uint16_t value =
        (
            static_cast<std::uint16_t>(
                data_[position_]
            ) << 8
        ) |
        static_cast<std::uint16_t>(
            data_[position_ + 1]
        );

    position_ += 2;

    return value;
}

std::uint32_t ByteReader::readU24() {
    requireReadable(3);

    const std::uint32_t value =
        (
            static_cast<std::uint32_t>(
                data_[position_]
            ) << 16
        ) |
        (
            static_cast<std::uint32_t>(
                data_[position_ + 1]
            ) << 8
        ) |
        static_cast<std::uint32_t>(
            data_[position_ + 2]
        );

    position_ += 3;

    return value;
}

std::uint32_t ByteReader::readU32() {
    requireReadable(4);

    const std::uint32_t value =
        (
            static_cast<std::uint32_t>(
                data_[position_]
            ) << 24
        ) |
        (
            static_cast<std::uint32_t>(
                data_[position_ + 1]
            ) << 16
        ) |
        (
            static_cast<std::uint32_t>(
                data_[position_ + 2]
            ) << 8
        ) |
        static_cast<std::uint32_t>(
            data_[position_ + 3]
        );

    position_ += 4;

    return value;
}

std::int8_t ByteReader::readI8() {
    return static_cast<std::int8_t>(
        readU8()
    );
}

std::int16_t ByteReader::readI16() {
    return static_cast<std::int16_t>(
        readU16()
    );
}

std::int32_t ByteReader::readI32() {
    return static_cast<std::int32_t>(
        readU32()
    );
}

int ByteReader::readSignedSmart() {
    const std::uint8_t value =
        peekU8();

    if (value < 128) {
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

int ByteReader::readUnsignedSmart() {
    const std::uint8_t value =
        peekU8();

    if (value < 128) {
        return static_cast<int>(
            readU8()
        );
    }

    return
        static_cast<int>(
            readU16()
        ) - 32768;
}

std::vector<std::uint8_t> ByteReader::readBytes(
    std::size_t amount
) {
    const std::span<const std::uint8_t> bytes =
        readSpan(amount);

    return std::vector<std::uint8_t>(
        bytes.begin(),
        bytes.end()
    );
}

std::span<const std::uint8_t> ByteReader::readSpan(
    std::size_t amount
) {
    requireReadable(amount);

    const std::span<const std::uint8_t> result =
        data_.subspan(
            position_,
            amount
        );

    position_ += amount;

    return result;
}

std::string ByteReader::readTerminatedString(
    std::uint8_t terminator
) {
    const std::size_t start =
        position_;

    while (!atEnd()) {
        if (readU8() == terminator) {
            const std::size_t length =
                position_ - start - 1;

            return std::string(
                reinterpret_cast<const char*>(
                    data_.data() + start
                ),
                length
            );
        }
    }

    throw std::runtime_error(
        "String terminator was not found"
    );
}

std::uint8_t ByteReader::peekU8() const {
    requireReadable(1);

    return data_[position_];
}

void ByteReader::skip(
    std::size_t amount
) {
    requireReadable(amount);

    position_ += amount;
}

void ByteReader::setPosition(
    std::size_t position
) {
    requirePosition(position);

    position_ = position;
}

std::size_t ByteReader::position() const {
    return position_;
}

std::size_t ByteReader::remaining() const {
    return data_.size() - position_;
}

bool ByteReader::atEnd() const {
    return position_ == data_.size();
}

bool ByteReader::canRead(
    std::size_t amount
) const {
    return amount <= remaining();
}

void ByteReader::requireReadable(
    std::size_t amount
) const {
    if (!canRead(amount)) {
        throw std::out_of_range(
            "ByteReader does not contain enough remaining bytes"
        );
    }
}

void ByteReader::requirePosition(
    std::size_t position
) const {
    if (position > data_.size()) {
        throw std::out_of_range(
            "ByteReader position lies outside the data"
        );
    }
}

}
