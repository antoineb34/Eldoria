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

    configureCamera(width, height);

    scene_.objects.clear();
    scene_.camera = camera_;

    backend_.setClearColor({ 20, 30, 60, 255 });

    loadModel147();

    initialized_ = true;

    std::cout << "ClientRenderContext: initialized (" << width << "x" << height << ")\n";
    std::cout << "ClientRenderContext: scene objects=" << scene_.objects.size() << "\n";

    return true;
}

void ClientRenderContext::configureCamera(int width, int height) {
    camera_.viewportX = 0;
    camera_.viewportY = 0;
    camera_.viewportWidth = width;
    camera_.viewportHeight = height;

    // Match the known working ElForge model viewport defaults.
    camera_.angleX = 0.45f;
    camera_.angleY = 0.6f;
    camera_.distance = 1200.0f;
    camera_.fov = 0.35f;
    camera_.nearPlane = 1.0f;
    camera_.farPlane = 10000.0f;
}

void ClientRenderContext::loadModel147() {
    modelLoadedInScene_ = false;
    modelAsset_.reset();

    if (!cache_.isValid()) {
        std::cerr << "ElClient: failed to load model 147: cache invalid\n";
        return;
    }

    std::cout << "ElClient: loading model 147\n";

    auto loadedModel = modelLoader_.load(147);
    if (!loadedModel.has_value()) {
        std::cerr << "ElClient: failed to load model 147: ModelLoader returned empty optional\n";
        return;
    }

    std::cout << "ElClient: model 147 loaded vertices="
              << loadedModel->vertices.size()
              << " faces="
              << loadedModel->faces.size()
              << "\n";

    if (loadedModel->vertices.empty() || loadedModel->faces.empty()) {
        std::cerr << "ElClient: failed to load model 147: model has no geometry\n";
        return;
    }

    // Compute and log model bounds
    const auto& first = loadedModel->vertices.front();
    float minX = first.x;
    float maxX = first.x;
    float minY = first.y;
    float maxY = first.y;
    float minZ = first.z;
    float maxZ = first.z;

    for (const auto& vertex : loadedModel->vertices) {
        if (vertex.x < minX) minX = vertex.x;
        if (vertex.x > maxX) maxX = vertex.x;
        if (vertex.y < minY) minY = vertex.y;
        if (vertex.y > maxY) maxY = vertex.y;
        if (vertex.z < minZ) minZ = vertex.z;
        if (vertex.z > maxZ) maxZ = vertex.z;
    }

    std::cout << "ElClient: model 147 bounds x=["
              << minX << ", " << maxX << "] y=["
              << minY << ", " << maxY << "] z=["
              << minZ << ", " << maxZ << "]\n";

    modelAsset_ = std::move(*loadedModel);
    addModel147ToScene();
}

void ClientRenderContext::addModel147ToScene() {
    if (!modelAsset_.has_value()) {
        return;
    }

    eld::render::RenderObject object;
    object.model = &modelAsset_.value();
    object.transform.position = { 0.0f, 0.0f, 0.0f };
    object.transform.rotation = { 0.0f, 0.0f, 0.0f };
    object.transform.scale = { 1.0f, 1.0f, 1.0f };

    scene_.objects.clear();
    scene_.objects.push_back(object);
    scene_.camera = camera_;

    modelLoadedInScene_ = true;

    std::cout << "ClientRenderContext: added model 147 to scene (objects="
              << scene_.objects.size()
              << ")\n";

    std::cout << "ClientRenderContext: model transform position=("
              << object.transform.position.x << ", "
              << object.transform.position.y << ", "
              << object.transform.position.z << ") scale=("
              << object.transform.scale.x << ", "
              << object.transform.scale.y << ", "
              << object.transform.scale.z << ")\n";
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

    if (modelLoadedInScene_) {
        std::cout << "ClientRenderContext: rendering model scene objects="
                  << scene_.objects.size()
                  << "\n";

        pipeline_.render(scene_, backend_);
    }

    drawFallbackPlaceholder();

    backend_.endFrame();

    if (!debugLoggedFirstFrame_) {
        std::cout << "ClientRenderContext debug:\n";
        std::cout << "  initialized=" << initialized_ << "\n";
        std::cout << "  modelLoadedInScene=" << modelLoadedInScene_ << "\n";
        std::cout << "  scene objects=" << scene_.objects.size() << "\n";
        std::cout << "  camera viewport="
                  << camera_.viewportWidth << "x" << camera_.viewportHeight << "\n";
        std::cout << "  camera angleX=" << camera_.angleX
                  << " angleY=" << camera_.angleY
                  << " distance=" << camera_.distance
                  << " fov=" << camera_.fov << "\n";

        if (modelAsset_.has_value()) {
            std::cout << "  model vertices=" << modelAsset_->vertices.size()
                      << " faces=" << modelAsset_->faces.size() << "\n";

            if (!modelAsset_->vertices.empty()) {
                const auto& v = modelAsset_->vertices.front();
                std::cout << "  first vertex=("
                          << v.x << ", "
                          << v.y << ", "
                          << v.z << ")\n";
            }
        } else {
            std::cout << "  modelAsset=null\n";
        }

        debugLoggedFirstFrame_ = true;
    }
}

void ClientRenderContext::drawFallbackPlaceholder() {
    const int panelW = 200;
    const int panelH = 150;
    const int panelX = (camera_.viewportWidth - panelW) / 2;
    const int panelY = (camera_.viewportHeight - panelH) / 2;

    backend_.drawRect(
        panelX,
        panelY,
        panelW,
        panelH,
        { 0, 255, 255, 255 }
    );
}

} // namespace eldoria::apps::elclient