#include "ClientRenderContext.h"

#include "../../platform/sdl/SdlContext.h"

#include <iostream>

namespace eldoria::apps::elclient {

ClientRenderContext::ClientRenderContext(eld::platform::SdlContext& context)
    : sdlContext_(context)
    , backend_(context.renderer())
{
}

bool ClientRenderContext::initialize(int width, int height) {
    if (initialized_) {
        return true;
    }

    // Set up camera for client rendering (orthographic for 2D/UI)
    camera_.viewportX = 0;
    camera_.viewportY = 0;
    camera_.viewportWidth = width;
    camera_.viewportHeight = height;
    camera_.angleX = 0.0f;
    camera_.angleY = 0.0f;
    camera_.distance = 500.0f;
    camera_.fov = 1.04719755f; // 60 degrees
    camera_.nearPlane = 1.0f;
    camera_.farPlane = 10000.0f;

    // Clear scene
    scene_.objects.clear();
    scene_.camera = camera_;

    // Set visible baseline clear color (dark blue)
    backend_.setClearColor({ 20, 30, 60, 255 });

    initialized_ = true;
    std::cout << "ClientRenderContext: initialized (" << width << "x" << height << ")\n";
    return true;
}

void ClientRenderContext::beginFrame() {
    if (!initialized_) {
        return;
    }

    // Update scene camera reference
    scene_.camera = camera_;

    // Begin frame on backend (clears framebuffer, sets up texture)
    backend_.beginFrame(camera_);
}

void ClientRenderContext::endFrame() {
    if (!initialized_) {
        return;
    }

    // Render the scene through the pipeline
    pipeline_.render(scene_, backend_);

    // Draw baseline visible marker (white 10x10 square at top-left)
    backend_.drawRect(10, 10, 10, 10, { 255, 255, 255, 255 });

    // End frame on backend (uploads to SDL texture, renders to screen)
    backend_.endFrame();
}

} // namespace eldoria::apps::elclient