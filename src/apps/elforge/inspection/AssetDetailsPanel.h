#pragma once

namespace eld::elforge {

struct CacheExplorerState;

class AssetDetailsPanel {
public:
    void render(
        CacheExplorerState& state,
        float width,
        float height
    );
};

}
