#pragma once

#include <vector>

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

}
