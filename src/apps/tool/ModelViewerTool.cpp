#include "ModelViewerTool.h"

#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

#include <SDL3/SDL.h>

#include "../../core/cache/CacheStore.h"
#include "../../core/cache/CacheTypes.h"

#include "../../core/debug/ModelDebug.h"

#include "../../core/io/Compression.h"

#include "../../core/model/FaceDecoder.h"
#include "../../core/model/ModelFooter.h"
#include "../../core/model/ModelLayout.h"
#include "../../core/model/VertexDecoder.h"

#include "../../core/platform/SdlContext.h"

#include "../../core/render/Projection.h"
#include "../../core/render/WireframeRenderer.h"

#include "../../core/render/DepthBuffer.h"


namespace rf::tool {

int ModelViewerTool::run() {

    auto printStep =
        [](int step, const std::string& name) {

            std::cout
                << "\n\n"
                << "====================================================\n"
                << "STEP "
                << step
                << " — "
                << name
                << "\n"
                << "====================================================\n";
        };

    rf::cache::CacheStore cache(
        "cache/main_file_cache.dat",
        "cache/main_file_cache.idx1"
    );

    uint32_t modelId = 1000;

    std::vector<rf::model::Vertex> vertices;
    std::vector<rf::model::Face> faces;

    auto loadModel =
        [&](uint32_t id) -> bool {

            printStep(1, "CACHE LOADING");

            rf::cache::CacheArchive archive =
                cache.readArchive(id);

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

            printStep(2, "COMPRESSION DETECTION");

            rf::io::CompressionType compressionType =
                rf::io::detectCompression(
                    fullPayload
                );

            if (
                compressionType ==
                rf::io::CompressionType::Gzip
            ) {
                std::cout
                    << "\ncompression: GZIP\n";
            }
            else {
                std::cout
                    << "\ncompression: unknown\n";

                return false;
            }

            printStep(3, "GZIP DECOMPRESSION");

            std::vector<char> decompressedPayload =
                rf::io::decompressGzip(
                    fullPayload
                );

            std::cout
                << "\ndecompressed size: "
                << decompressedPayload.size()
                << " bytes\n";

            rf::debug::dumpBytes(
                decompressedPayload,
                "decompressed payload"
            );

            printStep(4, "MODEL FOOTER");

            rf::model::ModelFooter footer =
                rf::model::readModelFooter(
                    decompressedPayload
                );

            rf::debug::dumpModelFooter(
                footer
            );

            printStep(5, "MODEL LAYOUT");

            rf::model::ModelLayout layout =
                rf::model::calculateModelLayout(
                    footer
                );

            rf::debug::dumpModelChunks(
                decompressedPayload,
                footer,
                layout
            );

            printStep(6, "VERTEX DECODING");

            vertices =
                rf::model::decodeVertices(
                    decompressedPayload,
                    footer,
                    layout
                );

            rf::debug::dumpDecodedVertices(
                vertices
            );

            printStep(7, "FACE DECODING");

            faces =
                rf::model::decodeFaces(
                    decompressedPayload,
                    footer,
                    layout
                );

            rf::debug::dumpDecodedFaces(
                faces
            );

            return true;
        };

    auto hasAlpha =
        [&](uint32_t id) -> bool {

            rf::cache::CacheArchive archive =
                cache.readArchive(id);

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
                rf::io::detectCompression(fullPayload) !=
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
        };

    if (!loadModel(modelId)) {
        return 1;
    }

    printStep(8, "SDL INITIALIZATION");

    constexpr int WINDOW_WIDTH = 960;
    constexpr int WINDOW_HEIGHT = 640;

    rf::platform::SdlContext sdl(
        "RuneForge Model Viewer",
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    SDL_Window* window =
        sdl.window();

    SDL_Renderer* renderer =
        sdl.renderer();

    if (!window || !renderer) {
        return 1;
    }

    float renderAngle = 0.0f;
    float scale = 4.0f;

    bool showWireframe = true;
    bool showVertices = true;
    bool fillTriangles = true;
    bool useAlpha = true;

    bool running = true;

    printStep(9, "EVENT LOOP");

    while (running) {

        SDL_Event event;

        while (SDL_PollEvent(&event)) {

            if (
                event.type ==
                SDL_EVENT_QUIT
            ) {
                running = false;
            }

            if (
                event.type ==
                SDL_EVENT_KEY_DOWN
            ) {

                if (
                    event.key.key ==
                    SDLK_ESCAPE
                ) {
                    running = false;
                }

                if (
                    event.key.key ==
                    SDLK_UP
                ) {
                    scale += 0.25f;
                }

                if (
                    event.key.key ==
                    SDLK_DOWN
                ) {

                    scale -= 0.25f;

                    if (scale < 0.25f) {
                        scale = 0.25f;
                    }
                }

                if (
                    event.key.key ==
                    SDLK_RIGHT
                ) {
                    modelId++;

                    std::cout
                        << "\nloading model "
                        << modelId
                        << "\n";

                    loadModel(modelId);
                }

                if (
                    event.key.key ==
                    SDLK_LEFT
                ) {

                    if (modelId > 0) {
                        modelId--;
                    }

                    std::cout
                        << "\nloading model "
                        << modelId
                        << "\n";

                    loadModel(modelId);
                }

                if (
                    event.key.key ==
                    SDLK_1
                ) {
                    showWireframe =
                        !showWireframe;
                }

                if (
                    event.key.key ==
                    SDLK_2
                ) {
                    showVertices =
                        !showVertices;
                }

                if (
                    event.key.key ==
                    SDLK_3
                ) {
                    fillTriangles =
                        !fillTriangles;
                }
                if (
                    event.key.key ==
                    SDLK_4
                ) {
                    useAlpha =
                        !useAlpha;
                }

                if (
                    event.key.key ==
                    SDLK_A
                ) {
                    uint32_t searchId =
                        modelId + 1;

                    while (searchId < 100000) {

                        if (hasAlpha(searchId)) {

                            modelId =
                                searchId;

                            std::cout
                                << "\nfound alpha model "
                                << modelId
                                << "\n";

                            loadModel(modelId);

                            break;
                        }

                        searchId++;
                    }
                }
            }
        }

        int windowWidth = 0;
        int windowHeight = 0;

        SDL_GetWindowSize(
            window,
            &windowWidth,
            &windowHeight
        );

        static rf::render::DepthBuffer depthBuffer(
            WINDOW_WIDTH,
            WINDOW_HEIGHT
        );

        depthBuffer.resize(
            windowWidth,
            windowHeight
        );

        depthBuffer.clear();

        rf::render::Camera camera {};

        camera.centerX =
            windowWidth * 0.5f;

        camera.centerY =
            windowHeight * 0.5f;

        camera.angleY =
            renderAngle;

        camera.angleX =
            0.45f +
            std::sin(renderAngle * 0.7f) *
            0.15f;

        camera.scale =
            scale;

        rf::render::drawWireframeModel(
            renderer,
            depthBuffer,
            vertices,
            faces,
            camera,
            showWireframe,
            showVertices,
            fillTriangles,
            useAlpha
        );

        SDL_RenderPresent(
            renderer
        );

        renderAngle += 0.02f;

        SDL_Delay(16);
    }

    return 0;
}

}
