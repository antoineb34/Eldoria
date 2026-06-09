#pragma once

#include "CacheState.h"

namespace eldoria::apps::elforge {

class CacheTreePanel {
public:
    void render(
        CacheState& state,
        float width,
        float height
    );

private:
    void renderNode(
        CacheState& state,
        const CacheTreeNode& node,
        int depth
    );
};

}
