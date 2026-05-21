#include "ModelViewerMode.h"

#include <cmath>
#include <iostream>

#include <imgui.h>

#include "../../../core/debug/ModelDebug.h"

#include "../../../core/io/Compression.h"

#include "../../../core/model/ModelFooter.h"
#include "../../../core/model/ModelLayout.h"

#include "../../../core/render/WireframeRenderer.h"

namespace rf::tool {

ModelViewerMode::ModelViewerMode()
    : modelCache_(
          "cache/main_file_cache.dat",
          "cache/main_file_cache.idx1"
      )
{
}

bool ModelViewerMode::initialize() {

    return true;
}

void ModelViewerMode::onEnter() {

    if (loaded_) {
        return;
    }

    loaded_ =
        loadModel(
            modelId_
        );
}

void ModelViewerMode::handleEvent(
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
}

void ModelViewerMode::update() {

    renderAngle_ += 0.02f;
}

void ModelViewerMode::render(
    SDL_Renderer* renderer,
    rf::render::DepthBuffer& depthBuffer,
    int windowWidth,
    int windowHeight
) {
    rf::render::Camera camera {};

    camera.centerX =
        windowWidth * 0.5f +
        cameraOffsetX_;

    camera.centerY =
        windowHeight * 0.5f +
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
        camera,
        showWireframe_,
        showVertices_,
        fillTriangles_,
        useAlpha_,
        highlightTexturedFaces_
    );
}

void ModelViewerMode::renderUi() {

    ImGui::Text(
        "Model Viewer"
    );

    ImGui::Separator();

    ImGui::Text(
        "Model ID: %u",
        modelId_
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
        "Fill triangles",
        &fillTriangles_
    );

    ImGui::Checkbox(
        "Use alpha",
        &useAlpha_
    );

    ImGui::Checkbox(
        "Highlight textured faces",
        &highlightTexturedFaces_
    );

    ImGui::Separator();

    ImGui::Text(
        "Controls"
    );

    ImGui::Text(
        "Left / Right: change model"
    );

    ImGui::Text(
        "Up / Down: scale"
    );

    ImGui::Text(
        "WASD: move camera"
    );

    ImGui::Text(
        "F: find next alpha model"
    );
}

bool ModelViewerMode::loadModel(
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

    std::cout
        << "\ndecompressed size: "
        << decompressedPayload.size()
        << " bytes\n";

    rf::model::ModelFooter footer =
        rf::model::readModelFooter(
            decompressedPayload
        );

    rf::debug::dumpModelFooter(
        footer
    );

    rf::model::ModelLayout layout =
        rf::model::calculateModelLayout(
            footer
        );

    rf::debug::dumpModelChunks(
        decompressedPayload,
        footer,
        layout
    );

    vertices_ =
        rf::model::decodeVertices(
            decompressedPayload,
            footer,
            layout
        );

    faces_ =
        rf::model::decodeFaces(
            decompressedPayload,
            footer,
            layout
        );

    rf::debug::dumpDecodedVertices(
        vertices_
    );

    rf::debug::dumpDecodedFaces(
        faces_
    );

    return true;
}

bool ModelViewerMode::hasAlpha(
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

void ModelViewerMode::findNextAlphaModel() {

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
