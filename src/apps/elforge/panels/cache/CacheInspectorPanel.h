#pragma once

#include "CacheState.h"

namespace eldoria::apps::elforge {

class CacheInspectorPanel {
public:
    void render(
        CacheState& state,
        float width,
        float height
    );
};

}
