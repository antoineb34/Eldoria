#pragma once

namespace eld::elforge {

struct CacheExplorerState;

class AnimationViewOverlay {
public:
    void renderHeader(
        CacheExplorerState& state
    ) const;
};

}
