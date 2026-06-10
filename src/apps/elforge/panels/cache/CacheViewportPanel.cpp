#include "CacheViewportPanel.h"

#include <imgui.h>

#include "CacheState.h"

#include "../../../../render_next/RenderPipeline.h"
#include "../../../../render_next/backend/software/SoftwareRenderBackend.h"

namespace eldoria::apps::elforge {

    namespace {

        void updateViewportControls(CacheState& state) {
            const bool* keys = SDL_GetKeyboardState(nullptr);

            auto& transform = state.modelViewportTransform;

            constexpr float rotationSpeed = 0.03f;
            constexpr float moveSpeed = 8.0f;
            constexpr float zoomSpeed = 0.02f;

            if (keys[SDL_SCANCODE_LEFT]) {
                transform.rotationY -= rotationSpeed;
            }

            if (keys[SDL_SCANCODE_RIGHT]) {
                transform.rotationY += rotationSpeed;
            }

            if (keys[SDL_SCANCODE_UP]) {
                transform.rotationX -= rotationSpeed;
            }

            if (keys[SDL_SCANCODE_DOWN]) {
                transform.rotationX += rotationSpeed;
            }

            if (keys[SDL_SCANCODE_Q]) {
                transform.rotationZ -= rotationSpeed;
            }

            if (keys[SDL_SCANCODE_E]) {
                transform.rotationZ += rotationSpeed;
            }

            if (keys[SDL_SCANCODE_EQUALS]) {
                transform.scale += zoomSpeed;
            }

            if (keys[SDL_SCANCODE_MINUS]) {
                transform.scale -= zoomSpeed;
            }

            if (transform.scale < 0.1f) {
                transform.scale = 0.1f;
            }

            if (keys[SDL_SCANCODE_W]) {
                transform.offsetY -= moveSpeed;
            }

            if (keys[SDL_SCANCODE_S]) {
                transform.offsetY += moveSpeed;
            }

            if (keys[SDL_SCANCODE_A]) {
                transform.offsetX -= moveSpeed;
            }

            if (keys[SDL_SCANCODE_D]) {
                transform.offsetX += moveSpeed;
            }

            if (keys[SDL_SCANCODE_R]) {
                transform.offsetX = 0.0f;
                transform.offsetY = 0.0f;
                transform.offsetZ = 0.0f;
                transform.scale = 1.0f;
                transform.rotationX = 0.0f;
                transform.rotationY = 0.0f;
                transform.rotationZ = 0.0f;
            }

            static bool previousT = false;
            bool currentT = keys[SDL_SCANCODE_T];

            if (currentT && !previousT) {
                state.modelViewportHighlightTexturedFaces =
                    !state.modelViewportHighlightTexturedFaces;
            }

            previousT = currentT;
        }
    }

void CacheViewportPanel::render(
    CacheState& state,
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

    ImVec2 viewportPos =
        ImGui::GetCursorScreenPos();

    ImVec2 viewportSize =
        ImGui::GetContentRegionAvail();

    state.modelViewportX =
        static_cast<int>(viewportPos.x);

    state.modelViewportY =
        static_cast<int>(viewportPos.y);

    state.modelViewportWidth =
        static_cast<int>(viewportSize.x);

    state.modelViewportHeight =
        static_cast<int>(viewportSize.y);

    ImGui::Dummy(
        viewportSize
    );

    ImGui::EndChild();
}

void CacheViewportPanel::renderViewport(
    SDL_Renderer* renderer,
    const CacheState& state
) {

    if (!state.selectedModel) {
        if (state.selectedModelLoadError) {
            SDL_Rect clip {
                state.modelViewportX,
                state.modelViewportY,
                state.modelViewportWidth,
                state.modelViewportHeight
            };

            SDL_SetRenderClipRect(
                renderer,
                &clip
            );

            SDL_SetRenderDrawColor(
                renderer,
                68,
                36,
                36,
                255
            );

            SDL_FRect rect {
                static_cast<float>(state.modelViewportX),
                static_cast<float>(state.modelViewportY),
                static_cast<float>(state.modelViewportWidth),
                static_cast<float>(state.modelViewportHeight)
            };

            SDL_RenderFillRect(
                renderer,
                &rect
            );

            SDL_SetRenderClipRect(
                renderer,
                nullptr
            );
        }

        return;
    }

    CacheState& mutableState = const_cast<CacheState&>(state);

    mutableState.modelViewportCamera.viewportX = state.modelViewportX;
    mutableState.modelViewportCamera.viewportY = state.modelViewportY;
    mutableState.modelViewportCamera.viewportWidth = state.modelViewportWidth;
    mutableState.modelViewportCamera.viewportHeight = state.modelViewportHeight;

    SDL_Rect clip {
        state.modelViewportX,
        state.modelViewportY,
        state.modelViewportWidth,
        state.modelViewportHeight
    };

    SDL_SetRenderClipRect(
        renderer,
        &clip
    );

    SDL_SetRenderDrawColor(
        renderer,
        68,
        88,
        68,
        255
    );

    SDL_FRect rect {
        static_cast<float>(state.modelViewportX),
        static_cast<float>(state.modelViewportY),
        static_cast<float>(state.modelViewportWidth),
        static_cast<float>(state.modelViewportHeight)
    };

    SDL_RenderFillRect(
        renderer,
        &rect
    );

    updateViewportControls(mutableState);

    rf::render_next::RenderObject object;
    object.model = &state.selectedModel.value();
    object.transform.position = {
        state.modelViewportTransform.offsetX,
        state.modelViewportTransform.offsetY,
        state.modelViewportTransform.offsetZ
    };
    object.transform.rotation = {
        state.modelViewportTransform.rotationX,
        state.modelViewportTransform.rotationY,
        state.modelViewportTransform.rotationZ
    };
    object.transform.scale = {
        state.modelViewportTransform.scale,
        state.modelViewportTransform.scale,
        state.modelViewportTransform.scale
    };

    rf::render_next::RenderScene scene;
    scene.camera = state.modelViewportCamera;
    scene.objects.push_back(object);

    rf::render_next::SoftwareRenderBackend backend(renderer);

    backend.setHighlightTexturedFaces(
        state.modelViewportHighlightTexturedFaces
    );

    rf::render_next::RenderPipeline pipeline;

    pipeline.render(
        scene,
        backend
    );

    SDL_SetRenderClipRect(
        renderer,
        nullptr
    );
}

}
