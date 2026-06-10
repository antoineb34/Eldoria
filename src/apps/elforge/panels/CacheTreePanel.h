#pragma once

namespace eld::explorer {

struct CacheExplorerState;
struct CacheTreeNode;

class CacheTreePanel {
public:
    void render(
        CacheExplorerState& state,
        float width,
        float height
    );

private:
    void renderNode(
        CacheExplorerState& state,
        const CacheTreeNode& node,
        int depth
    );
};

}
