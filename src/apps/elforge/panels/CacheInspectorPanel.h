#pragma once

namespace eld::elforge {

struct CacheExplorerState;

class CacheInspectorPanel {
public:
    void render(
        CacheExplorerState& state,
        float width,
        float height
    );
};

}
