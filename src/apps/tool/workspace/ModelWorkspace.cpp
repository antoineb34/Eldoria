#include "ModelWorkspace.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <imgui.h>

#include "../../../core/debug/ModelDebug.h"
#include "../../../core/io/Compression.h"
#include "../../../core/codecs/model/ModelDecoder.h"
#include "../../../render/software/WireframeRenderer.h"

#include "../../../core/codecs/texture/TextureDecoder.h"
#include "../../../core/codecs/texture/TextureIndex.h"

namespace rf::tool {

ModelWorkspace::ModelWorkspace()
    : modelCache_(
          "cache/main_file_cache.dat",
          "cache/main_file_cache.idx1"
      ),
      textureArchive_(
          "cache/main_file_cache.dat",
          "cache/main_file_cache.idx0"
      )
{
}

bool ModelWorkspace::initialize() {
    return true;
}

void ModelWorkspace::onEnter() {
    if (loaded_) {
        return;
    }

    loaded_ =
        loadModel(
            modelId_
        );
}

void ModelWorkspace::handleEvent(
    const SDL_Event& event
) {
    if (
        event.type !=
        SDL_EVENT_KEY_DOWN
    ) {
        return;
    }

    if (event.key.key == SDLK_W) {
        cameraOffsetY_ -= 20.0f;
    }

    if (event.key.key == SDLK_S) {
        cameraOffsetY_ += 20.0f;
    }

    if (event.key.key == SDLK_A) {
        cameraOffsetX_ -= 20.0f;
    }

    if (event.key.key == SDLK_D) {
        cameraOffsetX_ += 20.0f;
    }

    if (event.key.key == SDLK_UP) {
        scale_ += 0.25f;
    }

    if (event.key.key == SDLK_DOWN) {
        scale_ -= 0.25f;

        if (scale_ < 0.25f) {
            scale_ = 0.25f;
        }
    }

    if (event.key.key == SDLK_RIGHT) {
        modelId_++;

        std::cout
            << "\nloading model "
            << modelId_
            << "\n";

        loadModel(
            modelId_
        );
    }

    if (event.key.key == SDLK_LEFT) {
        if (modelId_ > 0) {
            modelId_--;
        }

        std::cout
            << "\nloading model "
            << modelId_
            << "\n";

        loadModel(
            modelId_
        );
    }

    if (event.key.key == SDLK_1) {
        showWireframe_ =
            !showWireframe_;
    }

    if (event.key.key == SDLK_2) {
        showVertices_ =
            !showVertices_;
    }

    if (event.key.key == SDLK_3) {
        fillTriangles_ =
            !fillTriangles_;
    }

    if (event.key.key == SDLK_4) {
        useAlpha_ =
            !useAlpha_;
    }

    if (event.key.key == SDLK_5) {
        highlightTexturedFaces_ =
            !highlightTexturedFaces_;
    }

    if (event.key.key == SDLK_F) {
        findNextAlphaModel();
    }

    if (event.key.key == SDLK_T) {
        findNextTexturedModel();
    }

    if (event.key.key == SDLK_R) {
        findNextModelWithRenderType(
            static_cast<uint8_t>(
                targetRenderType_
            )
        );
    }
}

void ModelWorkspace::update() {
    renderAngle_ += 0.02f;
}

void ModelWorkspace::render(
    SDL_Renderer* renderer,
    rf::render::DepthBuffer& depthBuffer,
    int viewportX,
    int viewportY,
    int viewportWidth,
    int viewportHeight
) {
    rf::render::Camera camera {};

    SDL_Rect clip {
        viewportX,
        viewportY,
        viewportWidth,
        viewportHeight
    };

    SDL_SetRenderClipRect(
        renderer,
        &clip
    );

    SDL_SetRenderDrawColor(
        renderer,
        185,
        212,
        177,
        255
    );

    SDL_FRect background {
        static_cast<float>(viewportX),
        static_cast<float>(viewportY),
        static_cast<float>(viewportWidth),
        static_cast<float>(viewportHeight)
    };

    SDL_RenderFillRect(
        renderer,
        &background
    );

    camera.centerX =
        viewportX +
        viewportWidth * 0.5f +
        cameraOffsetX_;

    camera.centerY =
        viewportY +
        viewportHeight * 0.5f +
        cameraOffsetY_;

    camera.angleY =
        renderAngle_;

    camera.angleX =
        0.45f +
        std::sin(renderAngle_ * 0.7f) *
        0.15f;

    camera.scale =
        scale_;

    rf::render::drawWireframeModel(
        renderer,
        depthBuffer,
        vertices_,
        faces_,
        textures_,
        camera,
        showWireframe_,
        showVertices_,
        fillTriangles_,
        useAlpha_,
        highlightTexturedFaces_
    );

    SDL_SetRenderClipRect(
        renderer,
        nullptr
    );
}

void ModelWorkspace::renderUi() {
    ImGui::Text("Model Viewer");
    ImGui::Separator();

    ImGui::Text("Model ID: %u", modelId_);

    if (ImGui::Button("Previous")) {
        if (modelId_ > 0) {
            modelId_--;
        }

        loadModel(modelId_);
    }

    ImGui::SameLine();

    if (ImGui::Button("Next")) {
        modelId_++;
        loadModel(modelId_);
    }

    ImGui::Separator();

    if (ImGui::Button("Find textured")) {
        findNextTexturedModel();
    }

    ImGui::SameLine();

    if (ImGui::Button("Find alpha")) {
        findNextAlphaModel();
    }

    ImGui::Separator();

    ImGui::Text("Render type search");

    ImGui::SliderInt(
        "Type",
        &targetRenderType_,
        0,
        3
    );

    if (ImGui::Button("Find render type")) {
        findNextModelWithRenderType(
            static_cast<uint8_t>(
                targetRenderType_
            )
        );
    }

    ImGui::Separator();

    ImGui::Text("Display");

    ImGui::Checkbox(
        "Face debug",
        &showFaceDebug_
    );

    if (showFaceDebug_) {
        ImGui::Separator();
        ImGui::Text("Textured faces");

        ImGui::BeginChild(
            "FaceDebugPanel",
            ImVec2(0.0f, 220.0f),
            true
        );

        for (std::size_t i = 0; i < faces_.size(); i++) {
            const rf::model::Face& face = faces_[i];

            if (face.textureInfo < 0) {
                continue;
            }

            ImGui::Text(
                "#%zu color=%u info=%d type=%d tri=%d",
                i,
                face.color,
                face.textureInfo,
                static_cast<int>(face.renderType),
                face.textureTriangleIndex
            );
        }

        ImGui::EndChild();
    }

    ImGui::Checkbox(
        "Filled",
        &fillTriangles_
    );

    ImGui::Checkbox(
        "Wireframe",
        &showWireframe_
    );

    ImGui::Checkbox(
        "Vertices",
        &showVertices_
    );

    ImGui::Checkbox(
        "Alpha",
        &useAlpha_
    );

    ImGui::Checkbox(
        "Highlight textured",
        &highlightTexturedFaces_
    );

    ImGui::Separator();

    ImGui::TextWrapped(
        "Controls: arrows change model/scale, WASD moves camera."
    );
}

bool ModelWorkspace::loadTexture(
    int textureId
) {
    if (textures_.contains(textureId)) {
        return true;
    }

    const int fileIndex =
        textureId;

    try {
        std::vector<char> indexData =
            textureArchive_.loadTextureFile(
                "index.dat"
            );

        std::vector<std::uint8_t> indexBytes(
            indexData.begin(),
            indexData.end()
        );

        rf::texture::TextureIndex textureIndex =
            rf::texture::TextureIndexParser::parse(
                indexBytes
            );

        std::vector<char> textureData =
            textureArchive_.loadFileByIndexFromArchive(
                6,
                fileIndex
            );

        std::vector<std::uint8_t> textureBytes(
            textureData.begin(),
            textureData.end()
        );

        rf::texture::DecodedTexture decoded =
            rf::texture::TextureDecoder::decode(
                textureIndex,
                textureBytes,
                fileIndex
            );

        std::cout
            << "\nloaded texture id "
            << textureId
            << " from file index "
            << fileIndex
            << ": "
            << decoded.width
            << "x"
            << decoded.height
            << " pixels="
            << decoded.pixels.size()
            << "\n";

        textures_[textureId] =
            std::move(decoded);

        return true;
    }
    catch (const std::exception& error) {
        std::cout
            << "\nfailed to load texture "
            << textureId
            << ": "
            << error.what()
            << "\n";

        return false;
    }
}

bool ModelWorkspace::loadModel(
    uint32_t id
) {
    std::cout
        << "\n\n====================================================\n"
        << "MODEL LOAD\n"
        << "====================================================\n";

    rf::cache::CacheArchive archive =
        modelCache_.readArchive(
            id
        );

    std::vector<char> fullPayload;

    fullPayload.reserve(
        archive.payload.size()
    );

    for (uint8_t byte : archive.payload) {
        fullPayload.push_back(
            static_cast<char>(
                byte
            )
        );
    }

    std::cout
        << "\nmodel id: "
        << id
        << "\narchive size: "
        << archive.entry.size
        << "\n";

    rf::io::CompressionType compressionType =
        rf::io::detectCompression(
            fullPayload
        );

    if (
        compressionType !=
        rf::io::CompressionType::Gzip
    ) {
        std::cout
            << "\ncompression: unknown\n";

        return false;
    }

    std::cout
        << "\ncompression: GZIP\n";

    std::vector<char> decompressedPayload =
        rf::io::decompressGzip(
            fullPayload
        );

    debug::dumpChunk(
        decompressedPayload,
        "model payload",
        0,
        decompressedPayload.size()
    );

    std::cout
        << "\ndecompressed size: "
        << decompressedPayload.size()
        << " bytes\n";

    rf::model::ModelDef model =
        rf::model::decodeModel(decompressedPayload);

    rf::debug::dumpModelFooter(model.footer);

    vertices_ = std::move(model.vertices);
    faces_ = std::move(model.faces);
    textureTriangles_ = std::move(model.textureTriangles);

    textures_.clear();

    int texturedFaces = 0;
    int imageTexturedFaces = 0;

    for (const rf::model::Face& face : faces_) {
        if (face.textureInfo < 0) {
            continue;
        }

        texturedFaces++;

        if (
            face.renderType != 2 &&
            face.renderType != 3
        ) {
            continue;
        }

        imageTexturedFaces++;

        loadTexture(
            face.color
        );
    }

    std::cout
        << "\nmodel decoded:"
        << "\nvertices: "
        << vertices_.size()
        << "\nfaces: "
        << faces_.size()
        << "\ntexture triangles: "
        << textureTriangles_.size()
        << "\ntextured face-info faces: "
        << texturedFaces
        << "\nimage textured faces: "
        << imageTexturedFaces
        << "\nloaded textures: "
        << textures_.size()
        << "\n";

    return true;
}

bool ModelWorkspace::hasRenderType(
    uint32_t id,
    uint8_t renderType
) {
    rf::cache::CacheArchive archive =
        modelCache_.readArchive(
            id
        );

    if (archive.payload.empty()) {
        return false;
    }

    std::vector<char> fullPayload;

    fullPayload.reserve(
        archive.payload.size()
    );

    for (uint8_t byte : archive.payload) {
        fullPayload.push_back(
            static_cast<char>(
                byte
            )
        );
    }

    if (
        rf::io::detectCompression(
            fullPayload
        ) !=
        rf::io::CompressionType::Gzip
    ) {
        return false;
    }

    std::vector<char> decompressedPayload =
        rf::io::decompressGzip(
            fullPayload
        );

    rf::model::ModelFooter footer =
        rf::model::readModelFooter(
            decompressedPayload
        );

    if (footer.textureFlag != 1) {
        return false;
    }

    rf::model::ModelLayout layout =
        rf::model::calculateModelLayout(
            footer
        );

    std::vector<rf::model::Face> faces =
        rf::model::decodeFaces(
            decompressedPayload,
            footer,
            layout
        );

    for (const rf::model::Face& face : faces) {
        if (
            face.textureInfo >= 0 &&
            face.renderType == renderType
        ) {
            return true;
        }
    }

    return false;
}

void ModelWorkspace::findNextModelWithRenderType(
    uint8_t renderType
) {
    uint32_t searchId =
        modelId_ + 1;

    while (searchId < 100000) {
        if (
            hasRenderType(
                searchId,
                renderType
            )
        ) {
            modelId_ =
                searchId;

            std::cout
                << "\nfound model "
                << modelId_
                << " with renderType "
                << static_cast<int>(
                    renderType
                )
                << "\n";

            loadModel(
                modelId_
            );

            return;
        }

        searchId++;
    }

    std::cout
        << "\nno model found with renderType "
        << static_cast<int>(
            renderType
        )
        << "\n";
}

bool ModelWorkspace::hasAlpha(
    uint32_t id
) {
    rf::cache::CacheArchive archive =
        modelCache_.readArchive(
            id
        );

    if (archive.payload.empty()) {
        return false;
    }

    std::vector<char> fullPayload;

    fullPayload.reserve(
        archive.payload.size()
    );

    for (uint8_t byte : archive.payload) {
        fullPayload.push_back(
            static_cast<char>(
                byte
            )
        );
    }

    if (
        rf::io::detectCompression(
            fullPayload
        ) !=
        rf::io::CompressionType::Gzip
    ) {
        return false;
    }

    std::vector<char> decompressedPayload =
        rf::io::decompressGzip(
            fullPayload
        );

    rf::model::ModelFooter footer =
        rf::model::readModelFooter(
            decompressedPayload
        );

    return footer.alphaFlag == 1;
}

bool ModelWorkspace::hasTexture(
    uint32_t id
) {
    rf::cache::CacheArchive archive =
        modelCache_.readArchive(
            id
        );

    if (archive.payload.empty()) {
        return false;
    }

    std::vector<char> fullPayload;

    fullPayload.reserve(
        archive.payload.size()
    );

    for (uint8_t byte : archive.payload) {
        fullPayload.push_back(
            static_cast<char>(
                byte
            )
        );
    }

    if (
        rf::io::detectCompression(
            fullPayload
        ) !=
        rf::io::CompressionType::Gzip
    ) {
        return false;
    }

    std::vector<char> decompressedPayload =
        rf::io::decompressGzip(
            fullPayload
        );

    rf::model::ModelFooter footer =
        rf::model::readModelFooter(
            decompressedPayload
        );

    return footer.textureFlag == 1 &&
        footer.textureTriangleCount > 0;
}

void ModelWorkspace::findNextTexturedModel() {
    uint32_t searchId =
        modelId_ + 1;

    while (searchId < 100000) {
        if (hasTexture(searchId)) {
            modelId_ =
                searchId;

            std::cout
                << "\nfound textured model "
                << modelId_
                << "\n";

            loadModel(
                modelId_
            );

            return;
        }

        searchId++;
    }

    std::cout
        << "\nno textured model found\n";
}

void ModelWorkspace::findNextAlphaModel() {
    uint32_t searchId =
        modelId_ + 1;

    while (searchId < 100000) {
        if (hasAlpha(searchId)) {
            modelId_ =
                searchId;

            std::cout
                << "\nfound alpha model "
                << modelId_
                << "\n";

            loadModel(
                modelId_
            );

            return;
        }

        searchId++;
    }

    std::cout
        << "\nno alpha model found\n";
}

}
