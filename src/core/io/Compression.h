#pragma once

#include <vector>
#include <bzlib.h>

namespace rf::io {

enum class CompressionType {
    Unknown,
    Gzip
};

CompressionType detectCompression(
    const std::vector<char>& payload
);

std::vector<char> decompressGzip(
    const std::vector<char>& payload
);

std::vector<char> decompressBzip2(
    const std::vector<char>& payload,
    size_t expectedSize
);

}
