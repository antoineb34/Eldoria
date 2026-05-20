
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include <zlib.h>

#include <cmath>
#include <chrono>
#include <thread>

#include <SDL3/SDL.h>

#include "../core/cache/CacheStore.h"
#include "../core/cache/CacheTypes.h"


int main() {

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

    std::cout << "\nfinished:"
              << "\nexpected size: " << size
              << "\nactual payload size: " << fullPayload.size()
              << "\n";

    std::cout << "\nfull payload bytes:\n";

    for (int i = 0; i < fullPayload.size(); i++) {

        if (i % 16 == 0) {
            std::cout << "\n";
            }

        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int)(unsigned char)fullPayload[i]
            << " ";
    }

    std::cout << std::dec << "\n";

    std::cout << "\nfull payload bytes:\n";

    for (int i = 0; i < fullPayload.size(); i++) {

        if (i % 16 == 0) {
            std::cout << "\n";
        }

        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int)(unsigned char)fullPayload[i]
            << " ";
    }

    std::cout << std::dec << "\n";

    if (fullPayload.size() >= 3) {

        unsigned char b0 = fullPayload[0];
        unsigned char b1 = fullPayload[1];
        unsigned char b2 = fullPayload[2];

        // gzip magic:
        // 1F 8B 08

        if (b0 == 0x1F &&
            b1 == 0x8B &&
            b2 == 0x08) {

            std::cout << "\ncompression: GZIP\n";
        }
        else {

            std::cout << "\ncompression: unknown\n";
        }
    }

    // =========================
    // DECOMPRESS GZIP INLINE
    // =========================

    z_stream stream{};
    stream.next_in = reinterpret_cast<Bytef*>(fullPayload.data());
    stream.avail_in = fullPayload.size();

    // 16 + MAX_WBITS means: expect gzip header
    int inflateResult = inflateInit2(&stream, 16 + MAX_WBITS);

    if (inflateResult != Z_OK) {
        std::cerr << "inflateInit2 failed: "
                  << inflateResult
                  << "\n";
        return 1;
    }

    std::vector<char> decompressedPayload;

    char decompressBuffer[4096];

    do {
        stream.next_out =
            reinterpret_cast<Bytef*>(decompressBuffer);

        stream.avail_out =
            sizeof(decompressBuffer);

        inflateResult =
            inflate(&stream, Z_NO_FLUSH);

        if (inflateResult != Z_OK &&
            inflateResult != Z_STREAM_END) {

            std::cerr << "inflate failed: "
                      << inflateResult
                      << "\n";

            inflateEnd(&stream);
            return 1;
        }

        int bytesProduced =
            sizeof(decompressBuffer) - stream.avail_out;

        decompressedPayload.insert(
            decompressedPayload.end(),
            decompressBuffer,
            decompressBuffer + bytesProduced
        );

    } while (inflateResult != Z_STREAM_END);

    inflateEnd(&stream);

    std::cout << "\ndecompressed size: "
              << decompressedPayload.size()
              << " bytes\n";

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

    // =========================
    // READ MODEL FOOTER
    // =========================

    constexpr int MODEL_FOOTER_SIZE = 18;

    if (decompressedPayload.size() < MODEL_FOOTER_SIZE) {
        std::cerr << "Decompressed payload too small for model footer\n";
        return 1;
    }

    int footerStart =
        decompressedPayload.size() - MODEL_FOOTER_SIZE;

    std::cout << "\nmodel footer bytes:\n";

    for (int i = footerStart; i < decompressedPayload.size(); i++) {

        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int)(unsigned char)decompressedPayload[i]
            << " ";
    }

    std::cout << std::dec << "\n";

    // =========================
    // DECODE MODEL FOOTER
    // =========================

    int modelFooterStart =
        decompressedPayload.size() - MODEL_FOOTER_SIZE;

    // footer format:
    //
    // 0-1   vertex count
    // 2-3   triangle count
    // 4     texture triangle count
    // 5     texture flag
    // 6     priority flag
    // 7     alpha flag
    // 8     triangle skin flag
    // 9     vertex skin flag
    // 10-11 x data length
    // 12-13 y data length
    // 14-15 z data length
    // 16-17 triangle data length

    uint32_t vertexCount =
        ((unsigned char)decompressedPayload[modelFooterStart + 0] << 8) |
        ((unsigned char)decompressedPayload[modelFooterStart + 1]);

    uint32_t triangleCount =
        ((unsigned char)decompressedPayload[modelFooterStart + 2] << 8) |
        ((unsigned char)decompressedPayload[modelFooterStart + 3]);

    uint32_t textureTriangleCount =
        (unsigned char)decompressedPayload[modelFooterStart + 4];

    uint32_t textureFlag =
        (unsigned char)decompressedPayload[modelFooterStart + 5];

    uint32_t priorityFlag =
        (unsigned char)decompressedPayload[modelFooterStart + 6];

    uint32_t alphaFlag =
        (unsigned char)decompressedPayload[modelFooterStart + 7];

    uint32_t triangleSkinFlag =
        (unsigned char)decompressedPayload[modelFooterStart + 8];

    uint32_t vertexSkinFlag =
        (unsigned char)decompressedPayload[modelFooterStart + 9];

    uint32_t xDataLength =
        ((unsigned char)decompressedPayload[modelFooterStart + 10] << 8) |
        ((unsigned char)decompressedPayload[modelFooterStart + 11]);

    uint32_t yDataLength =
        ((unsigned char)decompressedPayload[modelFooterStart + 12] << 8) |
        ((unsigned char)decompressedPayload[modelFooterStart + 13]);

    uint32_t zDataLength =
        ((unsigned char)decompressedPayload[modelFooterStart + 14] << 8) |
        ((unsigned char)decompressedPayload[modelFooterStart + 15]);

    uint32_t triangleDataLength =
        ((unsigned char)decompressedPayload[modelFooterStart + 16] << 8) |
        ((unsigned char)decompressedPayload[modelFooterStart + 17]);

    std::cout
        << "\nmodel footer decoded:"
        << "\nvertex count: "
        << vertexCount
        << " vertices"

        << "\ntriangle count: "
        << triangleCount
        << " triangles"

        << "\ntexture triangle count: "
        << textureTriangleCount
        << " textured triangles"

        << "\ntexture flag: "
        << textureFlag
        << " (0 = no texture info, 1 = texture data present)"

        << "\npriority flag: "
        << priorityFlag
        << " (255 = per-face priorities, otherwise shared priority)"

        << "\nalpha flag: "
        << alphaFlag
        << " (0 = no alpha/transparency, 1 = alpha data present)"

        << "\ntriangle skin flag: "
        << triangleSkinFlag
        << " (0 = no triangle skinning, 1 = triangle skin groups present)"

        << "\nvertex skin flag: "
        << vertexSkinFlag
        << " (0 = no vertex skinning, 1 = vertex skin groups present)"

        << "\nx data length: "
        << xDataLength
        << " bytes"

        << "\ny data length: "
        << yDataLength
        << " bytes"

        << "\nz data length: "
        << zDataLength
        << " bytes"

        << "\ntriangle data length: "
        << triangleDataLength
        << " bytes"

        << "\n";

        // =========================
        // CALCULATE MODEL CHUNK OFFSETS
        // =========================

        int offset = 0;

        int vertexFlagsOffset = offset;
        offset += vertexCount;

        int triangleTypesOffset = offset;
        offset += triangleCount;

        int trianglePrioritiesOffset = offset;

        if (priorityFlag == 255) {
            offset += triangleCount;
        }

        int triangleSkinsOffset = offset;

        if (triangleSkinFlag == 1) {
            offset += triangleCount;
        }

        int texturePointersOffset = offset;

        if (textureFlag == 1) {
            offset += triangleCount;
        }

        int vertexSkinsOffset = offset;

        if (vertexSkinFlag == 1) {
            offset += vertexCount;
        }

        int triangleAlphasOffset = offset;

        if (alphaFlag == 1) {
            offset += triangleCount;
        }

        int triangleDataOffset = offset;
        offset += triangleDataLength;

        int triangleColorsOffset = offset;
        offset += triangleCount * 2;

        int textureDataOffset = offset;
        offset += textureTriangleCount * 6;

        int xDataOffset = offset;
        offset += xDataLength;

        int yDataOffset = offset;
        offset += yDataLength;

        int zDataOffset = offset;
        offset += zDataLength;

        // =========================
        // PRINT MODEL CHUNKS
        // =========================

        auto printChunk = [&](const char* name, int start, int length) {

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
            vertexFlagsOffset,
            vertexCount
        );

        printChunk(
            "triangle types",
            triangleTypesOffset,
            triangleCount
        );

        if (priorityFlag == 255) {

            printChunk(
                "triangle priorities",
                trianglePrioritiesOffset,
                triangleCount
            );
        }

        if (triangleSkinFlag == 1) {

            printChunk(
                "triangle skins",
                triangleSkinsOffset,
                triangleCount
            );
        }

        if (textureFlag == 1) {

            printChunk(
                "texture pointers",
                texturePointersOffset,
                triangleCount
            );
        }

        if (vertexSkinFlag == 1) {

            printChunk(
                "vertex skins",
                vertexSkinsOffset,
                vertexCount
            );
        }

        if (alphaFlag == 1) {

            printChunk(
                "triangle alphas",
                triangleAlphasOffset,
                triangleCount
            );
        }

        printChunk(
            "triangle data",
            triangleDataOffset,
            triangleDataLength
        );

        printChunk(
            "triangle colors",
            triangleColorsOffset,
            triangleCount * 2
        );

        if (textureTriangleCount > 0) {

            printChunk(
                "texture data",
                textureDataOffset,
                textureTriangleCount * 6
            );
        }

        printChunk(
            "x data",
            xDataOffset,
            xDataLength
        );

        printChunk(
            "y data",
            yDataOffset,
            yDataLength
        );

        printChunk(
            "z data",
            zDataOffset,
            zDataLength
        );

        printChunk(
            "footer",
            modelFooterStart,
            MODEL_FOOTER_SIZE
        );



        // =========================
        // DECODE VERTICES
        // =========================

        struct DecodedVertex {
            int x;
            int y;
            int z;
        };

        auto readSmart = [&](int& cursor) -> int {
            unsigned char first =
                (unsigned char)decompressedPayload[cursor];

            if (first < 128) {
                cursor += 1;
                return (int)first - 64;
            }

            int value =
                (((unsigned char)decompressedPayload[cursor] << 8) |
                 ((unsigned char)decompressedPayload[cursor + 1]))
                - 49152;

            cursor += 2;

            return value;
        };

        std::vector<DecodedVertex> vertices;
        vertices.reserve(vertexCount);

        int currentX = 0;
        int currentY = 0;
        int currentZ = 0;

        int xCursor = xDataOffset;
        int yCursor = yDataOffset;
        int zCursor = zDataOffset;

        for (int i = 0; i < vertexCount; i++) {

            unsigned char flag =
                (unsigned char)decompressedPayload[vertexFlagsOffset + i];

            int dx = 0;
            int dy = 0;
            int dz = 0;

            // bit 0 = X delta exists
            if (flag & 1) {
                dx = readSmart(xCursor);
            }

            // bit 1 = Y delta exists
            if (flag & 2) {
                dy = readSmart(yCursor);
            }

            // bit 2 = Z delta exists
            if (flag & 4) {
                dz = readSmart(zCursor);
            }

            currentX += dx;
            currentY += dy;
            currentZ += dz;

            vertices.push_back({
                currentX,
                currentY,
                currentZ
            });
        }

        std::cout
            << "\n\n=== decoded vertices ==="
            << "\nvertex count: "
            << vertices.size()
            << "\n";

        for (int i = 0; i < vertices.size(); i++) {

            std::cout
                << "vertex "
                << (i + 1)
                << ": x="
                << vertices[i].x
                << " y="
                << vertices[i].y
                << " z="
                << vertices[i].z
                << "\n";
        }

        std::cout
            << "\nvertex stream cursors after decode:"
            << "\nx cursor: "
            << xCursor
            << " / expected end: "
            << (xDataOffset + xDataLength)

            << "\ny cursor: "
            << yCursor
            << " / expected end: "
            << (yDataOffset + yDataLength)

            << "\nz cursor: "
            << zCursor
            << " / expected end: "
            << (zDataOffset + zDataLength)
            << "\n";

            // =========================
            // RENDERING MOVED TO SDL3
            // =========================
            // We wait until faces are decoded, then open an SDL3 window
            // and draw the model as real 2D lines instead of ASCII.

                // =========================
                // DECODE TRIANGLES / FACES
                // =========================

                struct DecodedFace {
                    int a;
                    int b;
                    int c;
                };

                std::vector<DecodedFace> faces;
                faces.reserve(triangleCount);

                int triangleDataCursor = triangleDataOffset;

                int lastA = 0;
                int lastB = 0;
                int lastC = 0;
                int lastIndex = 0;

                for (int i = 0; i < triangleCount; i++) {

                    unsigned char type =
                        (unsigned char)
                        decompressedPayload[triangleTypesOffset + i];

                    if (type == 1) {

                        lastA =
                            readSmart(triangleDataCursor)
                            + lastIndex;

                        lastIndex = lastA;

                        lastB =
                            readSmart(triangleDataCursor)
                            + lastIndex;

                        lastIndex = lastB;

                        lastC =
                            readSmart(triangleDataCursor)
                            + lastIndex;

                        lastIndex = lastC;
                    }

                    else if (type == 2) {

                        lastB = lastC;

                        lastC =
                            readSmart(triangleDataCursor)
                            + lastIndex;

                        lastIndex = lastC;
                    }

                    else if (type == 3) {

                        lastA = lastC;

                        lastC =
                            readSmart(triangleDataCursor)
                            + lastIndex;

                        lastIndex = lastC;
                    }

                    else if (type == 4) {

                        int oldA = lastA;

                        lastA = lastB;
                        lastB = oldA;

                        lastC =
                            readSmart(triangleDataCursor)
                            + lastIndex;

                        lastIndex = lastC;
                    }

                    else {

                        std::cerr
                            << "Unknown triangle type: "
                            << (int)type
                            << " at triangle "
                            << i
                            << "\n";

                        return 1;
                    }

                    faces.push_back({
                        lastA,
                        lastB,
                        lastC
                    });
                }

                std::cout
                    << "\n\n=== decoded faces ==="
                    << "\nface count: "
                    << faces.size()
                    << "\n";

                for (int i = 0;
                     i < faces.size();
                     i++) {

                    std::cout
                        << "face "
                        << (i + 1)
                        << ": "
                        << faces[i].a
                        << ", "
                        << faces[i].b
                        << ", "
                        << faces[i].c
                        << "\n";
                }

                std::cout
                    << "\ntriangle data cursor after decode: "
                    << triangleDataCursor
                    << " / expected end: "
                    << (triangleDataOffset + triangleDataLength)
                    << "\n";

                    // =========================
                    // SDL3 WIREFRAME RENDERER
                    // =========================

                    constexpr int WINDOW_WIDTH = 960;
                    constexpr int WINDOW_HEIGHT = 640;

                    if (!SDL_Init(SDL_INIT_VIDEO)) {
                        std::cerr << "SDL_Init failed: "
                                  << SDL_GetError()
                                  << "\n";
                        return 1;
                    }

                    SDL_Window* window = SDL_CreateWindow(
                        "RuneForge SDL3 wireframe",
                        WINDOW_WIDTH,
                        WINDOW_HEIGHT,
                        SDL_WINDOW_RESIZABLE
                    );

                    if (!window) {
                        std::cerr << "SDL_CreateWindow failed: "
                                  << SDL_GetError()
                                  << "\n";
                        SDL_Quit();
                        return 1;
                    }

                    SDL_Renderer* renderer = SDL_CreateRenderer(
                        window,
                        nullptr
                    );

                    if (!renderer) {
                        std::cerr << "SDL_CreateRenderer failed: "
                                  << SDL_GetError()
                                  << "\n";
                        SDL_DestroyWindow(window);
                        SDL_Quit();
                        return 1;
                    }

                    float renderAngle = 0.0f;
                    float scale = 4.0f;
                    bool running = true;

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

                        int windowWidth = 0;
                        int windowHeight = 0;
                        SDL_GetWindowSize(window, &windowWidth, &windowHeight);

                        float centerX = windowWidth * 0.5f;
                        float centerY = windowHeight * 0.5f;

                        float angleY = renderAngle;
                        float angleX =
                            0.45f +
                            std::sin(renderAngle * 0.7f) * 0.15f;

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
                                float y = -(float)vertex.y; // RuneScape-style Y flip
                                float z = (float)vertex.z;

                                // rotate around Y
                                float x1 = x * cosY + z * sinY;
                                float z1 = -x * sinY + z * cosY;

                                // rotate around X
                                float y2 = y * cosX - z1 * sinX;
                                float z2 = y * sinX + z1 * cosX;

                                outX = centerX + x1 * scale;
                                outY = centerY - y2 * scale;
                                outZ = z2;
                            };

                        SDL_SetRenderDrawColor(renderer, 12, 12, 16, 255);
                        SDL_RenderClear(renderer);

                        // Draw triangle edges.
                        for (int i = 0; i < faces.size(); i++) {
                            DecodedFace face = faces[i];

                            if (face.a < 0 || face.a >= vertices.size() ||
                                face.b < 0 || face.b >= vertices.size() ||
                                face.c < 0 || face.c >= vertices.size()) {
                                continue;
                            }

                            float ax, ay, az;
                            float bx, by, bz;
                            float cx, cy, cz;

                            projectVertex(vertices[face.a], ax, ay, az);
                            projectVertex(vertices[face.b], bx, by, bz);
                            projectVertex(vertices[face.c], cx, cy, cz);

                            float avgZ = (az + bz + cz) / 3.0f;

                            // Simple depth tint.
                            if (avgZ > 40) {
                                SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
                            }
                            else if (avgZ > 20) {
                                SDL_SetRenderDrawColor(renderer, 180, 220, 255, 255);
                            }
                            else if (avgZ > 0) {
                                SDL_SetRenderDrawColor(renderer, 120, 200, 180, 255);
                            }
                            else {
                                SDL_SetRenderDrawColor(renderer, 80, 120, 120, 255);
                            }

                            SDL_RenderLine(renderer, ax, ay, bx, by);
                            SDL_RenderLine(renderer, bx, by, cx, cy);
                            SDL_RenderLine(renderer, cx, cy, ax, ay);
                        }

                        // Draw vertices as small points.
                        SDL_SetRenderDrawColor(renderer, 255, 120, 80, 255);

                        for (int i = 0; i < vertices.size(); i++) {
                            float vx, vy, vz;
                            projectVertex(vertices[i], vx, vy, vz);

                            SDL_FRect point {
                                vx - 2.0f,
                                vy - 2.0f,
                                4.0f,
                                4.0f
                            };

                            SDL_RenderFillRect(renderer, &point);
                        }

                        SDL_RenderPresent(renderer);

                        renderAngle += 0.02f;

                        SDL_Delay(16);
                    }

                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    SDL_Quit();

    return 0;
}
