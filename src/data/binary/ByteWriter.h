#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace eld::binary {

class ByteWriter {
public:
    ByteWriter();

    explicit ByteWriter(
        std::size_t initialCapacity
    );

    explicit ByteWriter(
        std::vector<std::uint8_t> data
    );

    /*
     * Append values.
     */

    void writeU8(
        std::uint8_t value
    );

    void writeU16(
        std::uint16_t value
    );

    void writeU24(
        std::uint32_t value
    );

    void writeU32(
        std::uint32_t value
    );

    void writeI8(
        std::int8_t value
    );

    void writeI16(
        std::int16_t value
    );

    void writeI32(
        std::int32_t value
    );

    void writeSignedSmart(
        int value
    );

    void writeUnsignedSmart(
        int value
    );

    void writeBytes(
        std::span<const std::uint8_t> bytes
    );

    void writeBytes(
        const std::vector<std::uint8_t>& bytes
    );

    void writeNullTerminatedString(
        const std::string& value
    );

    /*
     * Update existing fixed-size values.
     */

    void setU8(
        std::size_t offset,
        std::uint8_t value
    );

    void setU16(
        std::size_t offset,
        std::uint16_t value
    );

    void setU24(
        std::size_t offset,
        std::uint32_t value
    );

    void setU32(
        std::size_t offset,
        std::uint32_t value
    );

    void setI8(
        std::size_t offset,
        std::int8_t value
    );

    void setI16(
        std::size_t offset,
        std::int16_t value
    );

    void setI32(
        std::size_t offset,
        std::int32_t value
    );

    void setBytes(
        std::size_t offset,
        std::span<const std::uint8_t> bytes
    );

    /*
     * Edit byte ranges.
     */

    void insertBytes(
        std::size_t offset,
        std::span<const std::uint8_t> bytes
    );

    void replaceBytes(
        std::size_t offset,
        std::size_t length,
        std::span<const std::uint8_t> bytes
    );

    void eraseBytes(
        std::size_t offset,
        std::size_t length
    );

    /*
     * Buffer management.
     */

    void reserve(
        std::size_t capacity
    );

    void resize(
        std::size_t size,
        std::uint8_t value = 0
    );

    void clear();

    /*
     * State and output.
     */

    std::size_t size() const;
    bool empty() const;

    const std::vector<std::uint8_t>& data() const;

    std::vector<std::uint8_t> takeData();

private:
    void requireRange(
        std::size_t offset,
        std::size_t length
    ) const;

    void requireInsertPosition(
        std::size_t offset
    ) const;

private:
    std::vector<std::uint8_t> data_;
};

}
