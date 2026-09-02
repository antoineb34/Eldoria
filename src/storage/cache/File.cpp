#include "File.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace eld::cache {

File::File(
    std::uint16_t id,
    IndexEntry entry,
    Payload payload,
    eld::binary::CompressionType compressionType
)
    : id_(id),
      entry_(entry),
      payload_(std::move(payload)),
      compressionType_(compressionType) {
}

std::uint16_t File::getId() const {
    return id_;
}

const IndexEntry& File::getEntry() const {
    return entry_;
}

const Payload& File::getPayload() const {
    return payload_;
}

eld::binary::CompressionType
File::getCompressionType() const {
    return compressionType_;
}

std::vector<std::uint8_t> File::getBytes(
    CompressionState state
) const {
    std::vector<std::uint8_t> storedBytes;

    storedBytes.reserve(
        entry_.size
    );

    for (
        const Sector& sector :
        payload_.getSectors()
    ) {
        const std::size_t remaining =
            entry_.size -
            storedBytes.size();

        const std::size_t amount =
            std::min(
                remaining,
                sector.data.size()
            );

        storedBytes.insert(
            storedBytes.end(),
            sector.data.begin(),
            sector.data.begin() +
                static_cast<std::ptrdiff_t>(
                    amount
                )
        );

        if (
            storedBytes.size() ==
            entry_.size
        ) {
            break;
        }
    }

    if (
        storedBytes.size() !=
        entry_.size
    ) {
        throw std::runtime_error(
            "Cache payload does not contain the complete file"
        );
    }

    if (
        state ==
        CompressionState::Compressed
    ) {
        return storedBytes;
    }

    return eld::binary::decompress(
        storedBytes,
        compressionType_
    );
}

}
