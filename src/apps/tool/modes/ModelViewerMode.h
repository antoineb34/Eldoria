#pragma once

#include "ToolMode.h"

#include <cstdint>
#include <vector>

#include "../../../core/cache/CacheStore.h"

#include "../../../core/model/FaceDecoder.h"
#include "../../../core/model/VertexDecoder.h"

namespace rf::tool {

class ModelViewerMode : public ToolMode {
public:
    ModelViewerMode();

    bool initialize() override;

    void onEnter() override;

    void handleEvent(
        const SDL_Event& event
    ) override;

    void update() override;

    void render(
        SDL_Renderer* renderer,
        rf::render::DepthBuffer& depthBuffer,
        int windowWidth,
        int windowHeight
    ) override;

    void renderUi() override;

private:
    bool loadModel(
        uint32_t id
    );

    bool hasAlpha(
        uint32_t id
    );

    void findNextAlphaModel();

private:
    rf::cache::CacheStore modelCache_;

    uint32_t modelId_ = 0;

    std::vector<rf::model::Vertex> vertices_;
    std::vector<rf::model::Face> faces_;

    float renderAngle_ = 0.0f;
    float scale_ = 4.0f;

    float cameraOffsetX_ = 0.0f;
    float cameraOffsetY_ = 0.0f;

    bool showWireframe_ = true;
    bool showVertices_ = true;
    bool fillTriangles_ = true;
    bool useAlpha_ = true;
    bool highlightTexturedFaces_ = false;

    bool loaded_ = false;
};

}
