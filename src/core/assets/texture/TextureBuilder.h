#pragma once

#include "TextureAsset.h"
#include "TextureFile.h"

namespace rf::texture {

class TextureBuilder {
public:
    TextureAsset build(
        const TextureFile& file
    ) const;
};

}
