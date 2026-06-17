#include "ByteWriter.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace eld::binary {

ByteWriter::ByteWriter() = default;

ByteWriter::ByteWriter(
    std::size_t initialCapacity
) {
    data_.reserve(initialCapacity);
}

ByteWriter::ByteWriter(
    std::vector<std::uint8_t> data
)
    : data_(std::move(data)) {
}

void ByteWriter::writeU8(
    std::uint8_t value
) {
    data_.push_back(value);
}

void ByteWriter::writeU16(
    std::uint16_t value
) {
    data_.push_back(
        static_cast<std::uint8_t>(
            value >> 8
        )
    );

    data_.push_back(
        static_cast<std::uint8_t>(
            value
        )
    );
}

void ByteWriter::writeU24(
    std::uint32_t value
) {
    if (value > 0xFFFFFFU) {
        throw std::out_of_range(
            "U24 value exceeds 24 bits"
        );
    }

    data_.push_back(
        static_cast<std::uint8_t>(
            value >> 16
        )
    );

    data_.push_back(
        static_cast<std::uint8_t>(
            value >> 8
        )
    );

    data_.push_back(
        static_cast<std::uint8_t>(
            value
        )
    );
}

void ByteWriter::writeU32(
    std::uint32_t value
) {
    data_.push_back(
        static_cast<std::uint8_t>(
            value >> 24
        )
    );

    data_.push_back(
        static_cast<std::uint8_t>(
            value >> 16
        )
    );

    data_.push_back(
        static_cast<std::uint8_t>(
            value >> 8
        )
    );

    data_.push_back(
        static_cast<std::uint8_t>(
            value
        )
    );
}

void ByteWriter::writeI8(
    std::int8_t value
) {
    writeU8(
        static_cast<std::uint8_t>(value)
    );
}

void ByteWriter::writeI16(
    std::int16_t value
) {
    writeU16(
        static_cast<std::uint16_t>(value)
    );
}

void ByteWriter::writeI32(
    std::int32_t value
) {
    writeU32(
        static_cast<std::uint32_t>(value)
    );
}

void ByteWriter::writeSignedSmart(
    int value
) {
    if (
        value >= -64 &&
        value <= 63
    ) {
        writeU8(
            static_cast<std::uint8_t>(
                value + 64
            )
        );

        return;
    }

    if (
        value >= -16384 &&
        value <= 16383
    ) {
        writeU16(
            static_cast<std::uint16_t>(
                value + 49152
            )
        );

        return;
    }

    throw std::out_of_range(
        "Signed smart value is outside the supported range"
    );
}

void ByteWriter::writeUnsignedSmart(
    int value
) {
    if (
        value >= 0 &&
        value < 128
    ) {
        writeU8(
            static_cast<std::uint8_t>(
                value
            )
        );

        return;
    }

    if (
        value >= 128 &&
        value < 32768
    ) {
        writeU16(
            static_cast<std::uint16_t>(
                value + 32768
            )
        );

        return;
    }

    throw std::out_of_range(
        "Unsigned smart value is outside the supported range"
    );
}

void ByteWriter::writeBytes(
    std::span<const std::uint8_t> bytes
) {
    data_.insert(
        data_.end(),
        bytes.begin(),
        bytes.end()
    );
}

void ByteWriter::writeBytes(
    const std::vector<std::uint8_t>& bytes
) {
    writeBytes(
        std::span<const std::uint8_t>(
            bytes.data(),
            bytes.size()
        )
    );
}

void ByteWriter::writeNullTerminatedString(
    const std::string& value
) {
    const auto terminator =
        std::find(
            value.begin(),
            value.end(),
            '\0'
        );

    if (terminator != value.end()) {
        throw std::invalid_argument(
            "Null-terminated string contains an embedded null character"
        );
    }

    data_.insert(
        data_.end(),
        value.begin(),
        value.end()
    );

    data_.push_back(0);
}

void ByteWriter::setU8(
    std::size_t offset,
    std::uint8_t value
) {
    requireRange(offset, 1);

    data_[offset] = value;
}

void ByteWriter::setU16(
    std::size_t offset,
    std::uint16_t value
) {
    requireRange(offset, 2);

    data_[offset] =
        static_cast<std::uint8_t>(
            value >> 8
        );

    data_[offset + 1] =
        static_cast<std::uint8_t>(
            value
        );
}

void ByteWriter::setU24(
    std::size_t offset,
    std::uint32_t value
) {
    if (value > 0xFFFFFFU) {
        throw std::out_of_range(
            "U24 value exceeds 24 bits"
        );
    }

    requireRange(offset, 3);

    data_[offset] =
        static_cast<std::uint8_t>(
            value >> 16
        );

    data_[offset + 1] =
        static_cast<std::uint8_t>(
            value >> 8
        );

    data_[offset + 2] =
        static_cast<std::uint8_t>(
            value
        );
}

void ByteWriter::setU32(
    std::size_t offset,
    std::uint32_t value
) {
    requireRange(offset, 4);

    data_[offset] =
        static_cast<std::uint8_t>(
            value >> 24
        );

    data_[offset + 1] =
        static_cast<std::uint8_t>(
            value >> 16
        );

    data_[offset + 2] =
        static_cast<std::uint8_t>(
            value >> 8
        );

    data_[offset + 3] =
        static_cast<std::uint8_t>(
            value
        );
}

void ByteWriter::setI8(
    std::size_t offset,
    std::int8_t value
) {
    setU8(
        offset,
        static_cast<std::uint8_t>(value)
    );
}

void ByteWriter::setI16(
    std::size_t offset,
    std::int16_t value
) {
    setU16(
        offset,
        static_cast<std::uint16_t>(value)
    );
}

void ByteWriter::setI32(
    std::size_t offset,
    std::int32_t value
) {
    setU32(
        offset,
        static_cast<std::uint32_t>(value)
    );
}

void ByteWriter::setBytes(
    std::size_t offset,
    std::span<const std::uint8_t> bytes
) {
    requireRange(
        offset,
        bytes.size()
    );

    std::copy(
        bytes.begin(),
        bytes.end(),
        data_.begin() +
            static_cast<std::ptrdiff_t>(
                offset
            )
    );
}

void ByteWriter::insertBytes(
    std::size_t offset,
    std::span<const std::uint8_t> bytes
) {
    requireInsertPosition(offset);

    data_.insert(
        data_.begin() +
            static_cast<std::ptrdiff_t>(
                offset
            ),
        bytes.begin(),
        bytes.end()
    );
}

void ByteWriter::replaceBytes(
    std::size_t offset,
    std::size_t length,
    std::span<const std::uint8_t> bytes
) {
    requireRange(
        offset,
        length
    );

    const auto first =
        data_.begin() +
        static_cast<std::ptrdiff_t>(
            offset
        );

    const auto last =
        first +
        static_cast<std::ptrdiff_t>(
            length
        );

    data_.erase(
        first,
        last
    );

    data_.insert(
        data_.begin() +
            static_cast<std::ptrdiff_t>(
                offset
            ),
        bytes.begin(),
        bytes.end()
    );
}

void ByteWriter::eraseBytes(
    std::size_t offset,
    std::size_t length
) {
    requireRange(
        offset,
        length
    );

    const auto first =
        data_.begin() +
        static_cast<std::ptrdiff_t>(
            offset
        );

    const auto last =
        first +
        static_cast<std::ptrdiff_t>(
            length
        );

    data_.erase(
        first,
        last
    );
}

void ByteWriter::reserve(
    std::size_t capacity
) {
    data_.reserve(capacity);
}

void ByteWriter::resize(
    std::size_t size,
    std::uint8_t value
) {
    data_.resize(
        size,
        value
    );
}

void ByteWriter::clear() {
    data_.clear();
}

std::size_t ByteWriter::size() const {
    return data_.size();
}

bool ByteWriter::empty() const {
    return data_.empty();
}

const std::vector<std::uint8_t>&
ByteWriter::data() const {
    return data_;
}

std::vector<std::uint8_t>
ByteWriter::takeData() {
    return std::move(data_);
}

void ByteWriter::requireRange(
    std::size_t offset,
    std::size_t length
) const {
    if (
        offset > data_.size() ||
        length > data_.size() - offset
    ) {
        throw std::out_of_range(
            "ByteWriter range lies outside the buffer"
        );
    }
}

void ByteWriter::requireInsertPosition(
    std::size_t offset
) const {
    if (offset > data_.size()) {
        throw std::out_of_range(
            "ByteWriter insert position lies outside the buffer"
        );
    }
}

}
