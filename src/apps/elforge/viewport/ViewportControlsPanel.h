#pragma once

#include <imgui.h>

#include <cstdint>

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
    Texture,
    Image,
    Sprite
};


class ViewportControlsPanel {
public:
    ViewportViewKind kindFor(
        const CacheExplorerState& state
    ) const;

    void update(
        CacheExplorerState& state,
        ViewportViewKind kind
    );

    void renderInterfaceWorkspace(
        CacheExplorerState& state,
        const ImVec2& controlsPosition,
        const ImVec2& controlsSize
    );

    void renderModelWorkspace(
        CacheExplorerState& state
    );

    void renderTextureWorkspace(
        CacheExplorerState& state,
        const ImVec2& controlsPosition,
        const ImVec2& controlsSize
    );

    const InterfaceViewOptions&
    interfaceOptions() const;

    const ModelViewOptions&
    modelOptions() const;

    const TextureViewOptions&
    textureOptions() const;


private:
    InterfaceViewPanel interfaceViewPanel_;
    ModelViewPanel modelViewPanel_;
    TextureViewPanel textureViewPanel_;
};

}
