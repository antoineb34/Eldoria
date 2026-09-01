#pragma once

#include <cstdint>
#include <functional>

#include "views/interface/InterfaceViewPanel.h"
#include "views/model/ModelViewPanel.h"
#include "views/texture/TextureViewPanel.h"

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

struct ViewportControlsLayout {
    float controlsHeight = 30.0f;
    float resizeHandleHeight = 0.0f;
    float minimumHeight = 30.0f;
    float maximumHeight = 30.0f;
};

class ViewportControlsPanel {
public:
    static constexpr float CollapsedHeight = 32.0f;

    ViewportViewKind kindFor(
        const CacheExplorerState& state
    ) const;

    void update(
        CacheExplorerState& state,
        ViewportViewKind kind
    );

    ViewportControlsLayout updateLayout(float availableHeight);

    void renderResizeHandle(
        const ViewportControlsLayout& layout
    );

    void render(
        CacheExplorerState& state,
        ViewportViewKind kind,
        float controlsHeight,
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

    InterfaceViewPanel interfaceViewPanel_;
    ModelViewPanel modelViewPanel_;
    TextureViewPanel textureViewPanel_;

    bool open_ = false;
    ViewportViewKind lastKind_ = ViewportViewKind::None;
    float preferredHeight_ = 175.0f;
    float animatedHeight_ = CollapsedHeight;
};

}
