#pragma once

#include <functional>

namespace eld::elforge {

struct CacheExplorerState;

class AnimationViewPanel {
public:
    void render(
        CacheExplorerState& state,
        const std::function<void()>&
            renderPlayerHud
    ) const;
};

}
