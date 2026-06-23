#include "CacheViewportPanel.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
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

    float scale =
        transform.scale.x;

    if (keys[SDL_SCANCODE_EQUALS]) {
        scale += zoomSpeed;
    }

    if (keys[SDL_SCANCODE_MINUS]) {
        scale -= zoomSpeed;
    }

    scale =
        std::max(
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

void renderCheckerboard(
    SDL_Renderer* renderer,
    const CacheExplorerState& state
) {
    constexpr int CellSize = 16;

    for (
        int y = 0;
        y < state.viewportHeight;
        y += CellSize
    ) {
        for (
            int x = 0;
            x < state.viewportWidth;
            x += CellSize
        ) {
            const bool light =
                (
                    x / CellSize +
                    y / CellSize
                ) %
                2 ==
                0;

            const std::uint8_t color =
                light
                    ? 180
                    : 130;

            SDL_SetRenderDrawColor(
                renderer,
                color,
                color,
                color,
                255
            );

            const SDL_FRect cell{
                static_cast<float>(
                    state.viewportX + x
                ),
                static_cast<float>(
                    state.viewportY + y
                ),
                static_cast<float>(
                    std::min(
                        CellSize,
                        state.viewportWidth - x
                    )
                ),
                static_cast<float>(
                    std::min(
                        CellSize,
                        state.viewportHeight - y
                    )
                )
            };

            SDL_RenderFillRect(
                renderer,
                &cell
            );
        }
    }
}

void renderImage(
    SDL_Renderer* renderer,
    const CacheExplorerState& state,
    const eld::image::Image& image
) {
    if (
        image.width == 0 ||
        image.height == 0 ||
        image.pixels.empty()
    ) {
        return;
    }

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

    renderCheckerboard(
        renderer,
        state
    );

    SDL_Texture* texture =
        SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STATIC,
            image.width,
            image.height
        );

    if (texture == nullptr) {
        SDL_SetRenderClipRect(
            renderer,
            nullptr
        );

        return;
    }

    SDL_UpdateTexture(
        texture,
        nullptr,
        image.pixels.data(),
        static_cast<int>(
            image.width *
            sizeof(eld::image::RgbaPixel)
        )
    );

    SDL_SetTextureBlendMode(
        texture,
        SDL_BLENDMODE_BLEND
    );

    SDL_SetTextureScaleMode(
        texture,
        SDL_SCALEMODE_NEAREST
    );

    float scale =
        std::min(
            static_cast<float>(
                state.viewportWidth
            ) /
                image.width,
            static_cast<float>(
                state.viewportHeight
            ) /
                image.height
        );

    if (scale >= 1.0f) {
        scale =
            std::floor(scale);
    }

    scale =
        std::max(
            scale,
            0.01f
        );

    const float width =
        image.width *
        scale;

    const float height =
        image.height *
        scale;

    const SDL_FRect destination{
        state.viewportX +
            (
                state.viewportWidth -
                width
            ) /
            2.0f,
        state.viewportY +
            (
                state.viewportHeight -
                height
            ) /
            2.0f,
        width,
        height
    };

    SDL_RenderTexture(
        renderer,
        texture,
        nullptr,
        &destination
    );

    SDL_DestroyTexture(
        texture
    );

    SDL_SetRenderClipRect(
        renderer,
        nullptr
    );
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
    if (state.activeSprite.has_value()) {
        renderImage(
            renderer,
            state,
            state.activeSprite->image
        );

        return;
    }

    if (state.activeTexture.has_value()) {
        renderImage(
            renderer,
            state,
            state.activeTexture->image
        );

        return;
    }

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

    scene.camera =
        state.camera;

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
