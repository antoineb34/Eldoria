#pragma once

#include <cstdint>
#include <functional>

#include "InterfaceViewPanel.h"
#include "ModelViewPanel.h"
#include "TextureViewPanel.h"

namespace eld::elforge {

struct CacheExplorerState;

enum class ViewportViewKind {
    None,
    Map,
    Midi,
    Animation,
    Interface,
    Npc,
    Location,
    SpotAnimation,
    Model,
    Texture
};

struct ViewportDrawerLayout {
    float drawerHeight = 30.0f;
    float resizeHandleHeight = 0.0f;
    float minimumHeight = 30.0f;
    float maximumHeight = 30.0f;
};

class ViewportViewDrawer {
public:
    static constexpr float CollapsedHeight = 30.0f;

    ViewportViewKind kindFor(
        const CacheExplorerState& state
    ) const;

    void update(
        CacheExplorerState& state,
        ViewportViewKind kind
    );

    ViewportDrawerLayout updateLayout(float availableHeight);

    void renderResizeHandle(
        const ViewportDrawerLayout& layout
    );

    void render(
        CacheExplorerState& state,
        ViewportViewKind kind,
        float drawerHeight,
        const std::function<void()>& renderAnimationControls,
        const std::function<void()>& renderMidiControls
    );

    const InterfaceViewOptions& interfaceOptions() const;
    const ModelViewOptions& modelOptions() const;
    const TextureViewOptions& textureOptions() const;

private:
    static const char* titleFor(ViewportViewKind kind);

    void renderActivePanel(
        CacheExplorerState& state,
        ViewportViewKind kind,
        const std::function<void()>& renderAnimationControls,
        const std::function<void()>& renderMidiControls
    );

    static const char* transformTypeName(std::uint8_t type);

    static void renderAnimationArchiveView(
        CacheExplorerState& state
    );

    InterfaceViewPanel interfaceViewPanel_;
    ModelViewPanel modelViewPanel_;
    TextureViewPanel textureViewPanel_;

    bool open_ = false;
    ViewportViewKind lastKind_ = ViewportViewKind::None;
    float preferredHeight_ = 300.0f;
    float animatedHeight_ = CollapsedHeight;
};

}
