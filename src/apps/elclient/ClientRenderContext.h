#pragma once

#include <memory>
#include <optional>
#include <SDL3/SDL.h>

#include "RenderPipeline.h"
#include "backend/software/SoftwareRenderBackend.h"
#include "scene/RenderCamera.h"
#include "scene/RenderScene.h"
#include "model/ModelAsset.h"
#include "model/ModelLoader.h"

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class ClientRenderContext {
public:
    explicit ClientRenderContext(
        eld::platform::SdlContext& context,
        eld::cache::Cache& cache,
        eld::model::ModelLoader& modelLoader
    );
    ~ClientRenderContext() = default;

    // Deleted copy/move
    ClientRenderContext(const ClientRenderContext&) = delete;
    ClientRenderContext& operator=(const ClientRenderContext&) = delete;
    ClientRenderContext(ClientRenderContext&&) = delete;
    ClientRenderContext& operator=(ClientRenderContext&&) = delete;

    // Initialize render resources (called once at startup)
    bool initialize(int width, int height);

    // Begin a new frame (clears (clears framebuffer, sets up camera)
    void beginFrame();

    // End frame (renders scene through pipeline, presents to SDL)
    void endFrame();

    // Get the render scene for client to populate
    eld::render::RenderScene& scene() { return scene_; }

    // Get the render camera
    eld::render::RenderCamera& camera() { return camera_; }

    // Get the software render backend
    eld::render::SoftwareRenderBackend& backend() { return backend_; }

    // Get the render pipeline
    eld::render::RenderPipeline& pipeline() { return pipeline_; }

    // Check if initialized
    bool isInitialized() const { return initialized_; }

private:
    // Camera configuration
    void configureCamera(int width, int height);

    // Load model 147 from cache
    void loadModel147();

    // Add model 147 to the scene
    void addModel147ToScene();

    // Draw fallback placeholder panel
    void drawFallbackPlaceholder();

private:
    eld::platform::SdlContext& sdlContext_;
    eld::cache::Cache& cache_;
    eld::model::ModelLoader& modelLoader_;
    eld::render::SoftwareRenderBackend backend_;
    eld::render::RenderPipeline pipeline_;
    eld::render::RenderScene scene_;
    eld::render::RenderCamera camera_;
    std::optional<eld::model::ModelAsset> modelAsset_;
    bool modelLoadedInScene_ = false;
    bool initialized_ = false;
    bool debugLoggedFirstFrame_ = false;
};

} // namespace eldoria::apps::elclient