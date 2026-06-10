#include "TextureUVMappingDecoder.h"

namespace rf::model {

TextureUVMappingDecoder::TextureUVMappingDecoder(
    const ModelFile& file
)
    : file_(file),
      buffer_(file.payload)
{
    buffer_.setPosition(
        file_.layout.textureDataOffset
    );
}

std::vector<TextureUVMapping> TextureUVMappingDecoder::decode() {
    std::vector<TextureUVMapping> mappings;

    mappings.reserve(
        file_.footer.textureTriangleCount
    );

    for (
        uint32_t i = 0;
        i < file_.footer.textureTriangleCount;
        i++
    ) {
        mappings.push_back(
            decodeMapping()
        );
    }

    return mappings;
}

TextureUVMapping TextureUVMappingDecoder::decodeMapping() {
    return {
        buffer_.readU16(),
        buffer_.readU16(),
        buffer_.readU16()
    };
}

}
