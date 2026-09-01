#pragma once

namespace eld::elforge {

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

    const InterfaceViewOptions& options() const;

private:
    void renderPresets();

    InterfaceViewOptions options_;
};

}
