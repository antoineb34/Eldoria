#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace eld::binary {

class ByteReader {
public:
    explicit ByteReader(
        std::span<const std::uint8_t> data
    );

    explicit ByteReader(
        const std::vector<std::uint8_t>& data
    );

    std::uint8_t readU8();
    std::uint16_t readU16();
    std::uint32_t readU24();
    std::uint32_t readU32();

    std::int8_t readI8();
    std::int16_t readI16();
    std::int32_t readI32();

    int readSignedSmart();
    int readUnsignedSmart();

    std::vector<std::uint8_t> readBytes(
        std::size_t amount
    );

    std::vector<std::uint8_t> readRemainingBytes();

    std::span<const std::uint8_t> readSpan(
        std::size_t amount
    );

    ByteReader readSubReader(
        std::size_t amount
    );

    std::string readTerminatedString(
        std::uint8_t terminator
    );

    std::string readNullTerminatedString();

    std::uint8_t peekU8() const;

    void skip(
        std::size_t amount
    );

    void setPosition(
        std::size_t position
    );

    void reset();

    std::size_t position() const;
    std::size_t size() const;
    std::size_t remaining() const;

    bool atEnd() const;

    bool canRead(
        std::size_t amount
    ) const;

private:
    void requireReadable(
        std::size_t amount
    ) const;

    void requirePosition(
        std::size_t position
    ) const;

private:
    std::span<const std::uint8_t> data_;
    std::size_t position_ = 0;
};

}
