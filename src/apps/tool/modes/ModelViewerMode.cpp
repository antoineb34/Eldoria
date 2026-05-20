#include "ModelViewerMode.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

#include "../../../core/cache/ArchiveDecoder.h"
#include "../../../core/cache/ArchiveFileTable.h"
#include "../../../core/cache/KnownArchives.h"
#include "../../../core/cache/NameHash.h"

#include "../../../core/debug/ModelDebug.h"

#include "../../../core/io/Compression.h"

#include "../../../core/model/ModelFooter.h"
#include "../../../core/model/ModelLayout.h"

#include "../../../core/render/Projection.h"
#include "../../../core/render/WireframeRenderer.h"

namespace rf::tool {

ModelViewerMode::ModelViewerMode()
    : modelCache_(
          "cache/main_file_cache.dat",
          "cache/main_file_cache.idx1"
      ),
      configCache_(
          "cache/main_file_cache.dat",
          "cache/main_file_cache.idx0"
      ),
      configLoader_(
          "cache/main_file_cache.dat",
          "cache/main_file_cache.idx0"
      )
{
}

bool ModelViewerMode::initialize() {

    if (!loadModel(modelId_)) {
        return false;
    }

    inspectIndex0();

    return true;
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

        loadModel(modelId_);
    }

    if (event.key.key == SDLK_LEFT) {

        if (modelId_ > 0) {
            modelId_--;
        }

        std::cout
            << "\nloading model "
            << modelId_
            << "\n";

        loadModel(modelId_);
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

                loadModel(modelId_);

                break;
            }

            searchId++;
        }
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

    ImGui::Begin("Model Viewer");

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

    ImGui::End();
}

bool ModelViewerMode::loadModel(
    uint32_t id
) {
    std::cout
        << "\n\n====================================================\n"
        << "MODEL LOAD\n"
        << "====================================================\n";

    rf::cache::CacheArchive archive =
        modelCache_.readArchive(id);

    std::vector<char> fullPayload;

    fullPayload.reserve(
        archive.payload.size()
    );

    for (uint8_t byte : archive.payload) {

        fullPayload.push_back(
            static_cast<char>(byte)
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
        modelCache_.readArchive(id);

    if (archive.payload.empty()) {
        return false;
    }

    std::vector<char> fullPayload;

    fullPayload.reserve(
        archive.payload.size()
    );

    for (uint8_t byte : archive.payload) {

        fullPayload.push_back(
            static_cast<char>(byte)
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

void ModelViewerMode::inspectIndex0() {

    const auto& knownNames =
        rf::cache::KNOWN_ARCHIVES;

    auto findKnownName =
        [&](uint32_t hash) -> std::string {

            for (const std::string& name : knownNames) {

                if (rf::cache::hashName(name) == hash) {
                    return name;
                }
            }

            return "";
        };

    std::cout
        << "\n\n====================================================\n"
        << "IDX0 ARCHIVE INSPECTION\n"
        << "====================================================\n";

    for (uint32_t archiveId = 0; archiveId < 20; archiveId++) {

        rf::cache::CacheArchive archive =
            configCache_.readArchive(archiveId);

        if (
            archive.entry.size == 0 ||
            archive.payload.empty()
        ) {
            continue;
        }

        rf::cache::DecodedArchive decoded =
            rf::cache::decodeArchiveContainer(
                archive.payload
            );

        rf::cache::ArchiveFileTable table =
            rf::cache::readArchiveFileTable(
                decoded.payload
            );

        std::cout
            << "\n----------------------------------------------------\n"
            << "archive "
            << archiveId
            << "\n"
            << "----------------------------------------------------\n"
            << "size: "
            << archive.entry.size
            << "\n"
            << "first sector: "
            << archive.entry.firstSector
            << "\n"
            << "compressed: "
            << (decoded.compressed ? "yes" : "no")
            << "\n"
            << "compressed size: "
            << decoded.compressedSize
            << "\n"
            << "uncompressed size: "
            << decoded.uncompressedSize
            << "\n"
            << "decoded payload: "
            << decoded.payload.size()
            << "\n"
            << "file count: "
            << table.fileCount
            << "\n";

        for (int i = 0; i < table.files.size(); i++) {

            const auto& file =
                table.files[i];

            std::string knownName =
                findKnownName(file.hash);

            std::cout
                << "  file "
                << std::setw(2)
                << i
                << " | hash="
                << file.hash;

            if (!knownName.empty()) {
                std::cout
                    << " | name="
                    << knownName;
            }

            std::cout
                << " | uncompressed="
                << file.uncompressedSize
                << " | compressed="
                << file.compressedSize
                << " | offset="
                << file.offset
                << "\n";
        }
    }

    std::vector<char> npcDat =
        configLoader_.loadFile(
            "npc.dat"
        );

    std::cout
        << "\nEXTRACTED npc.dat: "
        << npcDat.size()
        << " bytes\n";
}

}
