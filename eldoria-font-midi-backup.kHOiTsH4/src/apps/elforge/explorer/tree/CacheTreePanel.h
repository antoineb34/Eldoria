#pragma once

namespace eld::elforge {

struct CacheExplorerState;

class CacheTreePanel {
public:
    void render(
        CacheExplorerState& state,
        float width,
        float height
    );
};

}
