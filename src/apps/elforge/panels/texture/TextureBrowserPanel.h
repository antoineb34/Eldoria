#pragma once

#include "panels/cache/CacheState.h"

namespace eldoria::apps::elforge {

class TextureBrowserPanel {
public:
    void render(
        CacheState& state,
        float width,
        float height
    );
};

}
