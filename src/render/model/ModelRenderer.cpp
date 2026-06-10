#include "ModelRenderer.h"

#include <array>
#include <cmath>
#include <limits>
#include <vector>

#include "../software/color/Color.h"
#include "../software/raster/TriangleRasterizer.h"

#include "RenderMesh.h"
#include "../order/FaceOrderer.h"

namespace eld::render {

namespace {

constexpr float NoDepth = -std::numeric_limits<float>::infinity();

bool isValidTextureMapping(
    const eld::model::TextureUVMapping& mapping,
    const std::vector<eld::model::Vertex>& vertices
) {
    return
        mapping.originVertex >= 0 &&
        mapping.uVertex >= 0 &&
        mapping.vVertex >= 0 &&
        mapping.originVertex < static_cast<int>(vertices.size()) &&
        mapping.uVertex < static_cast<int>(vertices.size()) &&
        mapping.vVertex < static_cast<int>(vertices.size());
}

eld::model::Vertex transformVertex(
    eld::model::Vertex vertex,
    const ModelTransform& transform
) {
    float x = static_cast<float>(vertex.x) * transform.scale;
    float y = static_cast<float>(vertex.y) * transform.scale;
    float z = static_cast<float>(vertex.z) * transform.scale;

    float cosX = std::cos(transform.rotationX);
    float sinX = std::sin(transform.rotationX);

    float y1 = y * cosX - z * sinX;
    float z1 = y * sinX + z * cosX;

    y = y1;
    z = z1;

    float cosY = std::cos(transform.rotationY);
    float sinY = std::sin(transform.rotationY);

    float x1 = x * cosY + z * sinY;
    float z2 = -x * sinY + z * cosY;

    x = x1;
    z = z2;

    float cosZ = std::cos(transform.rotationZ);
    float sinZ = std::sin(transform.rotationZ);

    float x2 = x * cosZ - y * sinZ;
    float y2 = x * sinZ + y * cosZ;

    x = x2 + transform.offsetX;
    y = y2 + transform.offsetY;
    z = z + transform.offsetZ;

    return { x, y, z };
}

TextureMappingPoint toTexturePoint(
    const eld::model::Vertex& vertex,
    const ModelTransform& modelTransform
) {
    eld::model::Vertex transformed =
        transformVertex(vertex, modelTransform);

    return {
        static_cast<float>(transformed.x),
        static_cast<float>(transformed.y),
        static_cast<float>(transformed.z)
    };
}

void drawWireframe(
    SDL_Renderer* renderer,
    const RenderMesh& mesh,
    const RenderFace& face
) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    const ScreenPoint& a = mesh.vertices[face.a].screen;
    const ScreenPoint& b = mesh.vertices[face.b].screen;
    const ScreenPoint& c = mesh.vertices[face.c].screen;

    SDL_RenderLine(renderer, a.x, a.y, b.x, b.y);
    SDL_RenderLine(renderer, b.x, b.y, c.x, c.y);
    SDL_RenderLine(renderer, c.x, c.y, a.x, a.y);
}

void drawVertices(
    SDL_Renderer* renderer,
    const RenderMesh& mesh
) {
    SDL_SetRenderDrawColor(renderer, 255, 120, 80, 255);

    for (const RenderVertex& vertex : mesh.vertices) {
        const ScreenPoint& point = vertex.screen;

        SDL_FRect rect {
            point.x - 2.0f,
            point.y - 2.0f,
            4.0f,
            4.0f
        };

        SDL_RenderFillRect(renderer, &rect);
    }
}

void drawRenderFace(
    SDL_Renderer* renderer,
    const eld::model::ModelAsset& model,
    const RenderMesh& mesh,
    const ModelTransform& modelTransform,
    const RenderOptions& options,
    const RenderFace& renderFace
) {
    const eld::model::Face& face =
        *renderFace.source;

    const ScreenPoint& a =
        mesh.vertices[renderFace.a].screen;

    const ScreenPoint& b =
        mesh.vertices[renderFace.b].screen;

    const ScreenPoint& c =
        mesh.vertices[renderFace.c].screen;

    RgbColor color =
        rsColorToRgb(face.color);

    uint8_t alpha = 255;

    if (options.useAlpha && face.alpha > 0) {
        alpha = static_cast<uint8_t>(255 - face.alpha);
    }

    if (
        options.highlightTexturedFaces &&
        (
            face.renderType == 2 ||
            face.renderType == 3
        )
    ) {
        color = { 255, 0, 255 };
    }

    SDL_SetRenderDrawColor(
        renderer,
        color.r,
        color.g,
        color.b,
        alpha
    );

    auto textureIt =
        model.textures.find(face.color);

    bool hasMapping =
        face.textureUVMappingIndex >= 0 &&
        face.textureUVMappingIndex <
            static_cast<int>(model.textureUVMappings.size());

    bool isTexturedRenderType =
        face.renderType == 2 ||
        face.renderType == 3;

    bool textured =
        isTexturedRenderType &&
        hasMapping &&
        textureIt != model.textures.end();

    if (!textured) {
        fillTriangle(
            renderer,
            a,
            b,
            c
        );

        return;
    }

    const eld::model::TextureUVMapping& mapping =
        model.textureUVMappings[face.textureUVMappingIndex];

    if (!isValidTextureMapping(mapping, model.vertices)) {
        fillTriangle(
            renderer,
            a,
            b,
            c
        );

        return;
    }

    TextureMappingPoint faceA =
        toTexturePoint(model.vertices[face.a], modelTransform);

    TextureMappingPoint faceB =
        toTexturePoint(model.vertices[face.b], modelTransform);

    TextureMappingPoint faceC =
        toTexturePoint(model.vertices[face.c], modelTransform);

    TextureMappingPoint textureOrigin =
        toTexturePoint(model.vertices[mapping.originVertex], modelTransform);

    TextureMappingPoint textureU =
        toTexturePoint(model.vertices[mapping.uVertex], modelTransform);

    TextureMappingPoint textureV =
        toTexturePoint(model.vertices[mapping.vVertex], modelTransform);

    fillTexturedTriangle(
        renderer,
        a,
        b,
        c,
        faceA,
        faceB,
        faceC,
        textureOrigin,
        textureU,
        textureV,
        textureIt->second
    );
}

float averageBucketDepth(
    const std::vector<RenderFace>& a,
    const std::vector<RenderFace>& b
) {
    float total = 0.0f;
    size_t count = 0;

    for (const RenderFace& face : a) {
        total += face.depthAvg;
        count++;
    }

    for (const RenderFace& face : b) {
        total += face.depthAvg;
        count++;
    }

    if (count == 0) {
        return NoDepth;
    }

    return total / static_cast<float>(count);
}

void drawPriorityBuckets(
    SDL_Renderer* renderer,
    const eld::model::ModelAsset& model,
    const RenderMesh& mesh,
    const ModelTransform& modelTransform,
    const RenderOptions& options,
    const std::array<std::vector<RenderFace>, PriorityBucketCount>& buckets
) {
    float depth12 =
        averageBucketDepth(buckets[1], buckets[2]);

    float depth34 =
        averageBucketDepth(buckets[3], buckets[4]);

    float depth68 =
        averageBucketDepth(buckets[6], buckets[8]);

    size_t highBucket = 10;
    size_t highIndex = 0;

    auto hasHighPriorityFace = [&]() {
        while (
            highBucket < PriorityBucketCount &&
            highIndex >= buckets[highBucket].size()
        ) {
            highBucket++;
            highIndex = 0;
        }

        return highBucket < PriorityBucketCount;
    };

    auto highPriorityDepth = [&]() {
        if (!hasHighPriorityFace()) {
            return NoDepth;
        }

        return buckets[highBucket][highIndex].depthAvg;
    };

    auto drawNextHighPriority = [&]() {
        if (!hasHighPriorityFace()) {
            return;
        }

        drawRenderFace(
            renderer,
            model,
            mesh,
            modelTransform,
            options,
            buckets[highBucket][highIndex]
        );

        highIndex++;
    };

    for (size_t priority = 0; priority < 10; priority++) {
        while (
            priority == 0 &&
            highPriorityDepth() > depth12
        ) {
            drawNextHighPriority();
        }

        while (
            priority == 3 &&
            highPriorityDepth() > depth34
        ) {
            drawNextHighPriority();
        }

        while (
            priority == 5 &&
            highPriorityDepth() > depth68
        ) {
            drawNextHighPriority();
        }

        for (const RenderFace& face : buckets[priority]) {
            drawRenderFace(
                renderer,
                model,
                mesh,
                modelTransform,
                options,
                face
            );
        }
    }

    while (hasHighPriorityFace()) {
        drawNextHighPriority();
    }
}

}

void drawModel(
    SDL_Renderer* renderer,
    const eld::model::ModelAsset& model,
    const Camera& camera,
    const RenderOptions& options,
    const ModelTransform& modelTransform
) {
    RenderMeshBuilder meshBuilder;

    RenderMesh mesh =
        meshBuilder.build(
            model,
            camera,
            modelTransform
        );

    FaceOrderer faceOrderer;

    faceOrderer.order(
        mesh,
        options.faceOrderMode
    );

    auto buckets =
        faceOrderer.buildPriorityBuckets(mesh);

    if (options.fillTriangles) {
        drawPriorityBuckets(
            renderer,
            model,
            mesh,
            modelTransform,
            options,
            buckets
        );
    }

    if (options.showWireframe) {
        for (const std::vector<RenderFace>& bucket : buckets) {
            for (const RenderFace& face : bucket) {
                drawWireframe(renderer, mesh, face);
            }
        }
    }

    if (options.showVertices) {

        drawVertices(
            renderer,
            mesh
        );
    }
}

}
