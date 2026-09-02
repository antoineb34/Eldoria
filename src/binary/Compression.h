#pragma once

#include <cstdint>
#include <vector>

namespace eld::binary {

enum class CompressionType {
    None,
    Gzip,
    Bzip2
};

CompressionType getCompressionType(
    const std::vector<std::uint8_t>& bytes
);

std::vector<std::uint8_t> compress(
    const std::vector<std::uint8_t>& bytes,
    CompressionType type
);

std::vector<std::uint8_t> decompress(
    const std::vector<std::uint8_t>& bytes,
    CompressionType type
);

}
