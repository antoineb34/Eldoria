#include "ClientRenderContext.h"

#include "../../platform/sdl/SdlContext.h"
#include "model/ModelAsset.h"

#include <iostream>

namespace eldoria::apps::elclient {

ClientRenderContext::ClientRenderContext(
    eld::platform::SdlContext& context,
    eld::cache::Cache& cache,
    eld::model::ModelLoader& modelLoader
)
    : sdlContext_(context)
    , backend_(context.renderer())
    , cache_(cache)
    , modelLoader_(modelLoader)
{
}

bool ClientRenderContext::initialize(int width, int height) {
    if (initialized_) {
        return true;
    }

    // Set up camera matching ElForge working configuration
    camera_.viewportX = 0;
    camera_.viewportY = 0;
    camera_.viewportWidth = width;
    camera_.viewportHeight = height;
    camera_.angleX = 0.45f;     // ElForge uses 0.45f
    camera_.angleY = 0.6f;      // ElForge uses 0.6f
    camera_.distance = 1200.0f; // ElForge uses 1200.0f
    camera_.fov = 0.35f;        // ElForge uses 0.35f (not 60 degrees!)
    camera_.nearPlane = 1.0f;
    camera_.farPlane = 10000.0f;

    // Clear scene
    scene_.objects.clear();
    scene_.camera = camera_;

    // Load model 147 from cache and add to scene
    loadAndAddModel();

    // Set visible baseline clear color (dark blue)
    backend_.setClearColor({ 20, 30, 60, 255 });

    initialized_ = true;
    std::cout << "ClientRenderContext: initialized (" << width << "x" << height << ")\n";
    std::cout << "ClientRenderContext: scene objects=" << scene_.objects.size() << "\n";
    return true;
}

void ClientRenderContext::loadAndAddModel() {
    if (!cache_.isValid()) {
        std::cerr << "ElClient: failed to load model 147: cache invalid\n";
        return;
    }

    std::cout << "ElClient: loading model 147\n";

    auto modelAsset = modelLoader_.load(147);
    if (!modelAsset.has_value()) {
        std::cerr << "ElClient: failed to load model 147: not found in cache\n";
        return;
    }

    // Log model details to prove it loaded
    const auto& model = modelAsset.value();
    std::cout << "ElClient: model 147 loaded vertices=" << model.vertices.size()
              << " faces=" << model.faces.size() << "\n";

    // Verify model has actual geometry
    if (model.vertices.empty() || model.faces.empty()) {
        std::cerr << "ElClient: model 147 has no geometry (vertices=" << model.vertices.size()
                  << " faces=" << model.faces.size() << ")\n";
        return;
    }

    // Store the model asset
    modelAsset_ = std::move(modelAsset);

    // Create RenderObject with ElForge-style transform (scale=1, offset=0, rotation=0)
    eld::render::RenderObject obj;
    obj.model = &*modelAsset_;
    obj.transform.position = { 0.0f, 0.0f, 0.0f };
    obj.transform.rotation = { 0.0f, 0.0f, 0.0f };
    obj.transform.scale = { 1.0f, 1.0f, 1.0f }; // ElForge uses scale=1.0

    // Add to scene
    scene_.objects.push_back(obj);
    modelLoadedInScene_ = true;

    std::cout << "ClientRenderContext: added model 147 to scene (objects=" << scene_.objects.size() << ")\n";
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
    if (modelLoadedInScene_) {
        pipeline_.render(scene_, backend_);
    } else {
        // Still need to call backend endFrame to present the cleared frame
        backend_.beginFrame(camera_);
    }

    // Draw simple placeholder panel (centered, 200x150, bright cyan) ONLY if no model in scene
    if (!modelLoadedInScene_) {
        int panelW = 200;
        int panelH = 150;
        int panelX = (camera_.viewportWidth - panelW) / 2;
        int panelY = (camera_.viewportHeight - panelH) / 2;
        backend_.drawRect(panelX, panelY, panelW, panelH, { 0, 255, 255, 255 });
    }

    // End frame on backend (uploads to SDL texture, renders to screen)
    backend_.endFrame();
}

} // namespace eldoria::apps::elclient