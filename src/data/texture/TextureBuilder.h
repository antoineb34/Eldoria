#pragma once

#include "TextureAsset.h"
#include "TextureFile.h"

namespace eld::texture {

class TextureBuilder {
public:
    TextureAsset build(
        const TextureFile& file
    ) const;
};

}
