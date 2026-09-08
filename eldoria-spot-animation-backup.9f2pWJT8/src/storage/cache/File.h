#pragma once

#include <cstdint>
#include <vector>

#include "Index.h"
#include "Payload.h"
#include "binary/Compression.h"

namespace eld::cache {

enum class CompressionState : std::uint8_t {
    Decompressed,
    Compressed
};

struct FileEntry {
    std::uint16_t fileId;
    IndexEntry indexEntry;
};

struct FileData {
    std::vector<std::uint8_t> bytes;
    eld::binary::CompressionType compressionType;
};

class File {
public:
    File(
        std::uint16_t id,
        IndexEntry entry,
        Payload payload,
        eld::binary::CompressionType compressionType
    );

    std::uint16_t getId() const;

    const IndexEntry& getEntry() const;

    const Payload& getPayload() const;

    eld::binary::CompressionType
    getCompressionType() const;

    std::vector<std::uint8_t> getBytes(
        CompressionState state =
            CompressionState::Decompressed
    ) const;

private:
    std::uint16_t id_;

    IndexEntry entry_;

    Payload payload_;

    eld::binary::CompressionType
        compressionType_;
};

}
