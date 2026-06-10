#pragma once

namespace eld::explorer {

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
