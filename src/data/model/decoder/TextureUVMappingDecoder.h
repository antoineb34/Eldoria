#pragma once

#include <vector>

#include "../ModelAsset.h"
#include "../ModelFile.h"
#include "../../../io/ByteBuffer.h"

namespace rf::model {

class TextureUVMappingDecoder {
public:
    explicit TextureUVMappingDecoder(
        const ModelFile& file
    );

    std::vector<TextureUVMapping> decode();

private:
    TextureUVMapping decodeMapping();

    const ModelFile& file_;

    rf::io::ByteBuffer buffer_;
};

}
