#pragma once

#include "panels/cache/CacheState.h"

namespace eldoria::apps::elforge {

class TexturePreviewPanel {
public:
    void render(
        CacheState& state,
        float width,
        float height
    );
};

}
