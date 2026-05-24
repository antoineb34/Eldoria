#pragma once

#include "ToolMode.h"

#include <cstdint>
#include <vector>

#include "../../../core/cache/CacheStore.h"
#include "../../../core/cache/ConfigArchiveLoader.h"

#include "../../../core/model/FaceDecoder.h"
#include "../../../core/model/VertexDecoder.h"

#include "../../../core/texture/Texture.h"
#include <unordered_map>

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
        int viewportX,
        int viewportY,
        int viewportWidth,
        int viewportHeight
    ) override;

    void renderUi() override;

private:
    bool loadModel(
        uint32_t id
    );

    bool loadTexture(
        int textureId
    );

    bool hasAlpha(
        uint32_t id
    );

    void findNextAlphaModel();

    bool hasTexture(
        uint32_t id
    );

    void findNextTexturedModel();

    bool hasRenderType(
        uint32_t id,
        uint8_t renderType
    );

    void findNextModelWithRenderType(
        uint8_t renderType
    );

    int targetRenderType_ = 0;
    bool showFaceDebug_ = false;


private:
    rf::cache::CacheStore modelCache_;
    rf::cache::ConfigArchiveLoader textureArchive_;

    uint32_t modelId_ = 147;

    std::vector<rf::model::Vertex> vertices_;
    std::vector<rf::model::Face> faces_;
    std::vector<rf::model::TextureTriangle> textureTriangles_;

    std::unordered_map<int, rf::texture::DecodedTexture> textures_;

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
