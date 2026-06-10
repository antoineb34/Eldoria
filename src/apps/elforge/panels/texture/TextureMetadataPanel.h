#pragma once

#include "panels/cache/CacheState.h"

namespace eldoria::apps::elforge {

class TextureMetadataPanel {
public:
    void render(
        CacheState& state,
        float width,
        float height
    );
};

}
