#pragma once

#include <memory>

#include <SDL3/SDL.h>

#include "RenderPipeline.h"
#include "backend/software/SoftwareRenderBackend.h"
#include "scene/RenderCamera.h"
#include "scene/RenderScene.h"

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class ClientRenderContext {
public:
    explicit ClientRenderContext(eld::platform::SdlContext& context);
    ~ClientRenderContext() = default;

    // Deleted copy/move
    ClientRenderContext(const ClientRenderContext&) = delete;
    ClientRenderContext& operator=(const ClientRenderContext&) = delete;
    ClientRenderContext(ClientRenderContext&&) = delete;
    ClientRenderContext& operator=(ClientRenderContext&&) = delete;

    // Initialize render resources (called once at startup)
    bool initialize(int width, int height);

    // Begin a new frame (clears framebuffer, sets up camera)
    void beginFrame();

    // End frame (presents to SDL)
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
    eld::platform::SdlContext& sdlContext_;
    eld::render::SoftwareRenderBackend backend_;
    eld::render::RenderPipeline pipeline_;
    eld::render::RenderScene scene_;
    eld::render::RenderCamera camera_;
    bool initialized_ = false;
};

} // namespace eldoria::apps::elclient