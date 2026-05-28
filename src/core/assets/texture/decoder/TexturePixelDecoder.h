#pragma once

#include <cstdint>
#include <vector>

#include "../TextureFile.h"

namespace rf::texture {

class TexturePixelDecoder {
public:
    explicit TexturePixelDecoder(
        const TextureFile& file
    );

    std::vector<uint8_t> decode() const;

private:
    std::vector<uint8_t> decodeType0() const;
    std::vector<uint8_t> decodeType1() const;

private:
    const TextureFile& file_;
};

}
