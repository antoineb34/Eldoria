#include "viewport/ViewportControlsPanel.h"

#include "explorer/CacheExplorerState.h"

namespace eld::elforge {

ViewportViewKind ViewportControlsPanel::kindFor(
    const CacheExplorerState& state
) const {
    if (state.activeMap.has_value()) {
        return ViewportViewKind::Map;
    }

    if (state.activeMidi.has_value()) {
        return ViewportViewKind::Midi;
    }

    if (state.activeAnimation.has_value()) {
        return ViewportViewKind::Animation;
    }

    if (state.activeInterface.has_value()) {
        return ViewportViewKind::Interface;
    }

    if (state.activeTexture.has_value()) {
        return ViewportViewKind::Texture;
    }

    if (
        state.activeLocation.has_value() &&
        state.activeModel.has_value() &&
        state.activeModelHandle.has_value()
    ) {
        return ViewportViewKind::Location;
    }

    if (
        state.activeSpotAnimation.has_value() &&
        state.activeModel.has_value() &&
        state.activeModelHandle.has_value()
    ) {
        return ViewportViewKind::SpotAnimation;
    }

    if (
        state.activeNpc.has_value() &&
        state.activeModel.has_value() &&
        state.activeModelHandle.has_value()
    ) {
        return ViewportViewKind::Npc;
    }

    if (
        state.activeModel.has_value() &&
        state.activeModelHandle.has_value()
    ) {
        return ViewportViewKind::Model;
    }

    if (state.activeSprite.has_value()) {
        return ViewportViewKind::Sprite;
    }

    if (state.activeImage.has_value()) {
        return ViewportViewKind::Image;
    }

    return ViewportViewKind::None;
}


void ViewportControlsPanel::update(
    CacheExplorerState& state,
    ViewportViewKind kind
) {
    modelViewPanel_.update(
        state,
        kind ==
            ViewportViewKind::Model ||
        kind ==
            ViewportViewKind::Npc ||
        kind ==
            ViewportViewKind::Location ||
        kind ==
            ViewportViewKind::SpotAnimation
    );
}


void ViewportControlsPanel::
renderInterfaceWorkspace(
    CacheExplorerState& state,
    const ImVec2& controlsPosition,
    const ImVec2& controlsSize
) {
    interfaceViewPanel_.renderWorkspace(
        state,
        state.activeInterface.has_value(),
        controlsPosition,
        controlsSize
    );
}


void ViewportControlsPanel::
renderModelWorkspace(
    CacheExplorerState& state
) {
    modelViewPanel_.renderWorkspace(
        state,
        state.activeModel.has_value()
    );
}


void ViewportControlsPanel::
renderTextureWorkspace(
    CacheExplorerState& state,
    const ImVec2& controlsPosition,
    const ImVec2& controlsSize
) {
    textureViewPanel_.renderWorkspace(
        state,
        state.activeTexture.has_value(),
        controlsPosition,
        controlsSize
    );
}


const InterfaceViewOptions&
ViewportControlsPanel::interfaceOptions() const {
    return
        interfaceViewPanel_.options();
}


const ModelViewOptions&
ViewportControlsPanel::modelOptions() const {
    return
        modelViewPanel_.options();
}


const TextureViewOptions&
ViewportControlsPanel::textureOptions() const {
    return
        textureViewPanel_.options();
}

}
