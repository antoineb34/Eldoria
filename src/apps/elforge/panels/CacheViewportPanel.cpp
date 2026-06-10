#include "CacheViewportPanel.h"

#include <imgui.h>

#include "../CacheExplorerState.h"
#include "../../../render/model/ModelRenderer.h"

#include "../../../render_next/RenderPipeline.h"
#include "../../../render_next/backend/software/SoftwareRenderBackend.h"

namespace eld::explorer {

    namespace {

        void updateViewportControls(CacheExplorerState& state) {
            const bool* keys = SDL_GetKeyboardState(nullptr);

            auto& transform = state.modelTransform;

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
                state.debugHighlightTexturedFaces =
                    !state.debugHighlightTexturedFaces;
            }

            previousT = currentT;
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

    ImVec2 viewportPos =
        ImGui::GetCursorScreenPos();

    ImVec2 viewportSize =
        ImGui::GetContentRegionAvail();

    state.viewportX =
        static_cast<int>(viewportPos.x);

    state.viewportY =
        static_cast<int>(viewportPos.y);

    state.viewportWidth =
        static_cast<int>(viewportSize.x);

    state.viewportHeight =
        static_cast<int>(viewportSize.y);

    ImGui::Dummy(
        viewportSize
    );

    ImGui::EndChild();
}

void CacheViewportPanel::renderViewport(
    SDL_Renderer* renderer,
    CacheExplorerState& state
) {

    if (!state.activeModel) {
        return;
    }

    state.camera.viewportX = state.viewportX;
    state.camera.viewportY = state.viewportY;
    state.camera.viewportWidth = state.viewportWidth;
    state.camera.viewportHeight = state.viewportHeight;

    SDL_Rect clip {
        state.viewportX,
        state.viewportY,
        state.viewportWidth,
        state.viewportHeight
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
        static_cast<float>(state.viewportX),
        static_cast<float>(state.viewportY),
        static_cast<float>(state.viewportWidth),
        static_cast<float>(state.viewportHeight)
    };

    SDL_RenderFillRect(
        renderer,
        &rect
    );

    updateViewportControls(state);

    eld::render_next::RenderObject object;
    object.model = &state.activeModel.value();    object.transform.position = {
        state.modelTransform.offsetX,
        state.modelTransform.offsetY,
        state.modelTransform.offsetZ
    };
    object.transform.rotation = {
        state.modelTransform.rotationX,
        state.modelTransform.rotationY,
        state.modelTransform.rotationZ
    };
    object.transform.scale = {
        state.modelTransform.scale,
        state.modelTransform.scale,
        state.modelTransform.scale
    };

    eld::render_next::RenderScene scene;
    scene.camera = state.camera;
    scene.objects.push_back(object);

    eld::render_next::SoftwareRenderBackend backend(renderer);

    backend.setHighlightTexturedFaces(
        state.debugHighlightTexturedFaces
    );

    eld::render_next::RenderPipeline pipeline;

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
