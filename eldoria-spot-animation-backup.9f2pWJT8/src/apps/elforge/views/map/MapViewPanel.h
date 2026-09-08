#pragma once

namespace eld::elforge {

struct CacheExplorerState;

class MapViewPanel {
public:
    void render(
        CacheExplorerState& state
    ) const;
};

}
