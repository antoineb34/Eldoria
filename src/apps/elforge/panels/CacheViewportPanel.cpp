#include "CacheViewportPanel.h"

#include <algorithm>
#include <cstdint>

#include <imgui.h>

#include "../CacheExplorerState.h"

#include "graphics/GraphicsResources.h"
#include "../../../render/RenderPipeline.h"
#include "../../../render/backend/software/SoftwareRenderBackend.h"

namespace eld::elforge {

namespace {

void updateViewportControls(
    CacheExplorerState& state
) {
    const bool* keys =
        SDL_GetKeyboardState(nullptr);

    eld::render::Transform& transform =
        state.modelTransform;

    constexpr float rotationSpeed = 0.03f;
    constexpr float moveSpeed = 8.0f;
    constexpr float zoomSpeed = 0.02f;

    if (keys[SDL_SCANCODE_LEFT]) {
        transform.rotation.y -= rotationSpeed;
    }

    if (keys[SDL_SCANCODE_RIGHT]) {
        transform.rotation.y += rotationSpeed;
    }

    if (keys[SDL_SCANCODE_UP]) {
        transform.rotation.x -= rotationSpeed;
    }

    if (keys[SDL_SCANCODE_DOWN]) {
        transform.rotation.x += rotationSpeed;
    }

    if (keys[SDL_SCANCODE_Q]) {
        transform.rotation.z -= rotationSpeed;
    }

    if (keys[SDL_SCANCODE_E]) {
        transform.rotation.z += rotationSpeed;
    }

    float scale = transform.scale.x;

    if (keys[SDL_SCANCODE_EQUALS]) {
        scale += zoomSpeed;
    }

    if (keys[SDL_SCANCODE_MINUS]) {
        scale -= zoomSpeed;
    }

    scale = std::max(
        scale,
        0.1f
    );

    transform.scale = {
        scale,
        scale,
        scale
    };

    if (keys[SDL_SCANCODE_W]) {
        transform.position.y -= moveSpeed;
    }

    if (keys[SDL_SCANCODE_S]) {
        transform.position.y += moveSpeed;
    }

    if (keys[SDL_SCANCODE_A]) {
        transform.position.x -= moveSpeed;
    }

    if (keys[SDL_SCANCODE_D]) {
        transform.position.x += moveSpeed;
    }

    if (keys[SDL_SCANCODE_R]) {
        transform = {};
    }
}

}

void CacheViewportPanel::render(
    CacheExplorerState& state,
    float width,
    float height
) {
    ImGui::BeginChild(
        "CacheViewportPanel",
        ImVec2(width, height),
        true
    );

    ImGui::TextUnformatted("VIEWPORT");
    ImGui::Separator();

    const ImVec2 viewportPosition =
        ImGui::GetCursorScreenPos();

    const ImVec2 viewportSize =
        ImGui::GetContentRegionAvail();

    state.viewportX =
        static_cast<int>(
            viewportPosition.x
        );

    state.viewportY =
        static_cast<int>(
            viewportPosition.y
        );

    state.viewportWidth =
        std::max(
            static_cast<int>(
                viewportSize.x
            ),
            1
        );

    state.viewportHeight =
        std::max(
            static_cast<int>(
                viewportSize.y
            ),
            1
        );

    ImGui::Dummy(
        viewportSize
    );

    ImGui::EndChild();
}

void CacheViewportPanel::renderViewport(
    SDL_Renderer* renderer,
    CacheExplorerState& state,
    const eld::graphics::GraphicsResources& resources
) {
    if (!state.activeModelHandle.has_value()) {
        return;
    }

    state.camera.viewportWidth =
        static_cast<std::uint32_t>(
            state.viewportWidth
        );

    state.camera.viewportHeight =
        static_cast<std::uint32_t>(
            state.viewportHeight
        );

    const SDL_Rect clip{
        state.viewportX,
        state.viewportY,
        state.viewportWidth,
        state.viewportHeight
    };

    SDL_SetRenderClipRect(
        renderer,
        &clip
    );

    updateViewportControls(
        state
    );

    eld::render::RenderObject object;

    object.model =
        *state.activeModelHandle;

    object.transform =
        state.modelTransform;

    eld::render::RenderScene scene;
    scene.camera = state.camera;

    scene.objects.push_back(
        object
    );

    eld::render::SoftwareRenderBackend backend(
        renderer
    );

    backend.setOutputPosition(
        state.viewportX,
        state.viewportY
    );

    backend.setClearColor({
        68,
        88,
        68,
        255
    });

    eld::render::RenderPipeline pipeline;

    pipeline.render(
        scene,
        resources,
        backend
    );

    SDL_SetRenderClipRect(
        renderer,
        nullptr
    );
}

}
