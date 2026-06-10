#pragma once

#include <vector>

#include "../ModelAsset.h"
#include "../ModelFile.h"
#include "binary/ByteBuffer.h"

namespace eld::model {

class TextureUVMappingDecoder {
public:
    explicit TextureUVMappingDecoder(
        const ModelFile& file
    );

    std::vector<TextureUVMapping> decode();

private:
    TextureUVMapping decodeMapping();

    const ModelFile& file_;

    binary::ByteBuffer buffer_;
};

}
