#pragma once

#include <imgui.h>

namespace eld::elforge {

struct CacheExplorerState;

struct InterfaceViewOptions {
    bool showHiddenWidgets = false;

    bool showRectangles = true;
    bool showText = true;
    bool showSprites = true;
    bool showModels = true;
    bool showInventories = true;
    bool showItemLists = true;

    bool showCanvasBounds = true;
    bool showGrid = false;
    int gridSpacing = 16;

    bool showWidgetBounds = false;
    bool showContainerBounds = false;
    bool showClipRegions = false;
    bool showWidgetIds = false;
    bool showWidgetTypes = false;
    bool showWidgetOrigins = false;
    bool showParentLinks = false;
    bool showScrollExtents = false;
};

class InterfaceViewPanel {
public:
    void render(bool hasInterface);

    void renderWorkspace(
        CacheExplorerState& state,
        bool hasInterface,
        const ImVec2& controlsPosition,
        const ImVec2& controlsSize
    );

    const InterfaceViewOptions& options() const;

private:
    void renderPresets();

    InterfaceViewOptions options_;
};

}
