#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include <zlib.h>

#include <cmath>

#include <SDL3/SDL.h>

#include "../core/cache/CacheStore.h"
#include "../core/cache/CacheTypes.h"
#include "../core/io/Compression.h"
#include "../core/io/ByteBuffer.h"
#include "../core/model/ModelFooter.h"
#include "../core/model/ModelLayout.h"
#include "../core/model/VertexDecoder.h"
#include "../core/model/FaceDecoder.h"

int main() {

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

    printStep(1, "CACHE LOADING");

    // ============================================================
    // STEP 1 — CACHE LOADING
    // ============================================================

    rf::cache::CacheStore cache(
        "cache/main_file_cache.dat",
        "cache/main_file_cache.idx1"
    );

    rf::cache::CacheArchive archive =
        cache.readArchive(2635);

    uint32_t size =
        archive.entry.size;

    uint32_t sector =
        archive.entry.firstSector;

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
        << "\nidx entry:"
        << "\nsize: "
        << size
        << "\nfirst sector: "
        << sector
        << "\n";

    std::cout
        << "\nfinished:"
        << "\nexpected size: "
        << size
        << "\nactual payload size: "
        << fullPayload.size()
        << "\n";

    printStep(2, "COMPRESSION DETECTION");

    // ============================================================
    // STEP 2 — COMPRESSION DETECTION
    // ============================================================

        rf::io::CompressionType compressionType =
            rf::io::detectCompression(fullPayload);

        if (compressionType == rf::io::CompressionType::Gzip) {
            std::cout << "\ncompression: GZIP\n";
        }
        else {
            std::cout << "\ncompression: unknown\n";
        }

    printStep(3, "GZIP DECOMPRESSION");

    // ============================================================
    // STEP 3 — GZIP DECOMPRESSION
    // ============================================================

        std::vector<char> decompressedPayload =
            rf::io::decompressGzip(fullPayload);

        std::cout
            << "\ndecompressed size: "
            << decompressedPayload.size()
            << " bytes\n";

    printStep(4, "DECOMPRESSED PAYLOAD INSPECTION");

    // ============================================================
    // STEP 4 — DECOMPRESSED PAYLOAD INSPECTION
    // ============================================================

    std::cout << "\ndecompressed first bytes:\n";

    for (int i = 0; i < decompressedPayload.size(); i++) {

        if (i % 16 == 0) {
            std::cout << "\n";
        }

        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int)(unsigned char)decompressedPayload[i]
            << " ";
    }

    std::cout << std::dec << "\n";

    printStep(5, "MODEL FOOTER");

    // ============================================================
    // STEP 5 — MODEL FOOTER
    // ============================================================

    rf::model::ModelFooter footer =
        rf::model::readModelFooter(
            decompressedPayload
        );

    std::cout
        << "\nmodel footer decoded:"
        << "\nvertex count: "
        << footer.vertexCount
        << " vertices"

        << "\ntriangle count: "
        << footer.triangleCount
        << " triangles"

        << "\ntexture triangle count: "
        << footer.textureTriangleCount
        << " textured triangles"

        << "\ntexture flag: "
        << footer.textureFlag

        << "\npriority flag: "
        << footer.priorityFlag

        << "\nalpha flag: "
        << footer.alphaFlag

        << "\ntriangle skin flag: "
        << footer.triangleSkinFlag

        << "\nvertex skin flag: "
        << footer.vertexSkinFlag

        << "\nx data length: "
        << footer.xDataLength
        << " bytes"

        << "\ny data length: "
        << footer.yDataLength
        << " bytes"

        << "\nz data length: "
        << footer.zDataLength
        << " bytes"

        << "\ntriangle data length: "
        << footer.triangleDataLength
        << " bytes"

        << "\n";

    printStep(7, "MODEL CHUNK OFFSETS");

    // ============================================================
    // STEP 7 — MODEL CHUNK OFFSETS
    // ============================================================

    rf::model::ModelLayout layout =
        rf::model::calculateModelLayout(
            footer
        );

    printStep(8, "MODEL CHUNK INSPECTION");

    // ============================================================
    // STEP 8 — MODEL CHUNK INSPECTION
    // ============================================================

    auto printChunk =
        [&](const char* name, int start, int length) {

            std::cout
                << "\n\n=== "
                << name
                << " ==="
                << "\noffset: "
                << start
                << "\nlength: "
                << length
                << " bytes\n";

            for (int i = 0; i < length; i++) {

                if (i % 16 == 0) {
                    std::cout << "\n";
                }

                std::cout
                    << std::hex
                    << std::setw(2)
                    << std::setfill('0')
                    << (int)(unsigned char)
                       decompressedPayload[start + i]
                    << " ";
            }

            std::cout << std::dec << "\n";
        };

    printChunk(
        "vertex flags",
        layout.vertexFlagsOffset,
        footer.vertexCount
    );

    printChunk(
        "triangle types",
        layout.triangleTypesOffset,
        footer.triangleCount
    );

    printChunk(
        "triangle data",
        layout.triangleDataOffset,
        footer.triangleDataLength
    );

    printChunk(
        "triangle colors",
        layout.triangleColorsOffset,
        footer.triangleCount * 2
    );

    printChunk(
        "x data",
        layout.xDataOffset,
        footer.xDataLength
    );

    printChunk(
        "y data",
        layout.yDataOffset,
        footer.yDataLength
    );

    printChunk(
        "z data",
        layout.zDataOffset,
        footer.zDataLength
    );

    printStep(9, "SMART VALUE DECODING");

    // ============================================================
    // STEP 9 — SMART VALUE DECODING
    // ============================================================

    rf::io::ByteBuffer xBuffer(decompressedPayload);
    rf::io::ByteBuffer yBuffer(decompressedPayload);
    rf::io::ByteBuffer zBuffer(decompressedPayload);
    rf::io::ByteBuffer triangleDataBuffer(decompressedPayload);

    xBuffer.setPosition(layout.xDataOffset);
    yBuffer.setPosition(layout.yDataOffset);
    zBuffer.setPosition(layout.zDataOffset);
    triangleDataBuffer.setPosition(layout.triangleDataOffset);

    printStep(10, "VERTEX DECODING");

    // ============================================================
    // STEP 10 — VERTEX DECODING
    // ============================================================

    std::vector<rf::model::Vertex> vertices =
        rf::model::decodeVertices(
            decompressedPayload,
            footer,
            layout
        );

    std::cout
        << "\n\n=== decoded vertices ==="
        << "\nvertex count: "
        << vertices.size()
        << "\n";
        printStep(11, "FACE DECODING");

        // ============================================================
        // STEP 11 — FACE DECODING
        // ============================================================

        std::vector<rf::model::Face> faces =
            rf::model::decodeFaces(
                decompressedPayload,
                footer,
                layout
            );

        std::cout
            << "\n\n=== decoded faces ==="
            << "\nface count: "
            << faces.size()
            << "\n";

    printStep(12, "SDL INITIALIZATION");

    // ============================================================
    // STEP 12 — SDL INITIALIZATION
    // ============================================================

    constexpr int WINDOW_WIDTH = 960;
    constexpr int WINDOW_HEIGHT = 640;

    if (!SDL_Init(SDL_INIT_VIDEO)) {

        std::cerr
            << "SDL_Init failed: "
            << SDL_GetError()
            << "\n";

        return 1;
    }

    SDL_Window* window =
        SDL_CreateWindow(
            "RuneForge SDL3 wireframe",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            SDL_WINDOW_RESIZABLE
        );

    if (!window) {

        std::cerr
            << "SDL_CreateWindow failed: "
            << SDL_GetError()
            << "\n";

        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer =
        SDL_CreateRenderer(
            window,
            nullptr
        );

    if (!renderer) {

        std::cerr
            << "SDL_CreateRenderer failed: "
            << SDL_GetError()
            << "\n";

        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    float renderAngle = 0.0f;
    float scale = 4.0f;
    bool running = true;

    printStep(13, "EVENT LOOP");

    // ============================================================
    // STEP 13 — EVENT LOOP
    // ============================================================

    while (running) {

        SDL_Event event;

        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {

                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }

                if (event.key.key == SDLK_UP) {
                    scale += 0.25f;
                }

                if (event.key.key == SDLK_DOWN) {

                    scale -= 0.25f;

                    if (scale < 0.25f) {
                        scale = 0.25f;
                    }
                }
            }
        }

        // ============================================================
        // STEP 14 — CAMERA & MATH
        // ============================================================

        int windowWidth = 0;
        int windowHeight = 0;

        SDL_GetWindowSize(
            window,
            &windowWidth,
            &windowHeight
        );

        float centerX =
            windowWidth * 0.5f;

        float centerY =
            windowHeight * 0.5f;

        float angleY =
            renderAngle;

        float angleX =
            0.45f +
            std::sin(renderAngle * 0.7f) *
            0.15f;

        float cosY = std::cos(angleY);
        float sinY = std::sin(angleY);

        float cosX = std::cos(angleX);
        float sinX = std::sin(angleX);

        auto projectVertex =
            [&](const DecodedVertex& vertex,
                float& outX,
                float& outY,
                float& outZ) {

                float x = (float)vertex.x;
                float y = -(float)vertex.y;
                float z = (float)vertex.z;

                float x1 =
                    x * cosY +
                    z * sinY;

                float z1 =
                    -x * sinY +
                    z * cosY;

                float y2 =
                    y * cosX -
                    z1 * sinX;

                float z2 =
                    y * sinX +
                    z1 * cosX;

                outX =
                    centerX +
                    x1 * scale;

                outY =
                    centerY -
                    y2 * scale;

                outZ =
                    z2;
            };

        // ============================================================
        // STEP 15 — WIREFRAME RENDERING
        // ============================================================

        SDL_SetRenderDrawColor(
            renderer,
            12,
            12,
            16,
            255
        );

        SDL_RenderClear(renderer);

        for (int i = 0; i < faces.size(); i++) {

            DecodedFace face =
                faces[i];

            if (
                face.a < 0 ||
                face.a >= vertices.size() ||
                face.b < 0 ||
                face.b >= vertices.size() ||
                face.c < 0 ||
                face.c >= vertices.size()
            ) {
                continue;
            }

            float ax, ay, az;
            float bx, by, bz;
            float cx, cy, cz;

            projectVertex(
                vertices[face.a],
                ax, ay, az
            );

            projectVertex(
                vertices[face.b],
                bx, by, bz
            );

            projectVertex(
                vertices[face.c],
                cx, cy, cz
            );

            SDL_SetRenderDrawColor(
                renderer,
                220,
                220,
                220,
                255
            );

            SDL_RenderLine(
                renderer,
                ax, ay,
                bx, by
            );

            SDL_RenderLine(
                renderer,
                bx, by,
                cx, cy
            );

            SDL_RenderLine(
                renderer,
                cx, cy,
                ax, ay
            );
        }

        // ============================================================
        // STEP 16 — VERTEX POINT RENDERING
        // ============================================================

        SDL_SetRenderDrawColor(
            renderer,
            255,
            120,
            80,
            255
        );

        for (int i = 0; i < vertices.size(); i++) {

            float vx, vy, vz;

            projectVertex(
                vertices[i],
                vx, vy, vz
            );

            SDL_FRect point {
                vx - 2.0f,
                vy - 2.0f,
                4.0f,
                4.0f
            };

            SDL_RenderFillRect(
                renderer,
                &point
            );
        }

        // ============================================================
        // STEP 17 — FRAME PRESENTATION
        // ============================================================

        SDL_RenderPresent(renderer);

        renderAngle += 0.02f;

        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
