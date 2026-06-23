#pragma once

#include "TextureFile.h"
#include "TextureImage.h"
#include "TextureSourceMap.h"

namespace eld::texture {

class TextureDecoder {
public:
    TextureImage decode(
        const TextureFile& file
    ) const;

    TextureImage decode(
        const TextureFile& file,
        TextureSourceMap& sourceMap
    ) const;

private:
    TextureImage decodeImage(
        const TextureFile& file,
        TextureSourceMap* sourceMap
    ) const;
};

}
