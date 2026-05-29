#include "ModelRenderer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

#include "../software/camera/Projection.h"
#include "../software/color/Color.h"
#include "../software/math/Mat4.h"
#include "../software/raster/TriangleRasterizer.h"

#include "RenderFace.h"

namespace rf::render {

namespace {

constexpr size_t PriorityBucketCount = 12;
constexpr float NoDepth = -std::numeric_limits<float>::infinity();

bool isValidFace(
    const rf::model::Face& face,
    const std::vector<rf::model::Vertex>& vertices
) {
    return
        face.a >= 0 &&
        face.b >= 0 &&
        face.c >= 0 &&
        face.a < static_cast<int>(vertices.size()) &&
        face.b < static_cast<int>(vertices.size()) &&
        face.c < static_cast<int>(vertices.size());
}

bool isValidTextureMapping(
    const rf::model::TextureUVMapping& mapping,
    const std::vector<rf::model::Vertex>& vertices
) {
    return
        mapping.originVertex >= 0 &&
        mapping.uVertex >= 0 &&
        mapping.vVertex >= 0 &&
        mapping.originVertex < static_cast<int>(vertices.size()) &&
        mapping.uVertex < static_cast<int>(vertices.size()) &&
        mapping.vVertex < static_cast<int>(vertices.size());
}

bool isFinitePoint(const ScreenPoint& point) {
    return
        std::isfinite(point.x) &&
        std::isfinite(point.y) &&
        std::isfinite(point.z);
}

rf::model::Vertex transformVertex(
    rf::model::Vertex vertex,
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
    const rf::model::Vertex& vertex,
    const ModelTransform& modelTransform
) {
    rf::model::Vertex transformed =
        transformVertex(vertex, modelTransform);

    return {
        static_cast<float>(transformed.x),
        static_cast<float>(transformed.y),
        static_cast<float>(transformed.z)
    };
}

ScreenPoint projectModelVertex(
    const rf::model::Vertex& vertex,
    const ModelTransform& modelTransform,
    const Mat4& view,
    const Mat4& projection,
    const Camera& camera
) {
    rf::model::Vertex transformed =
        transformVertex(vertex, modelTransform);

    return projectVertex(
        transformed,
        view,
        projection,
        camera
    );
}

void drawWireframe(
    SDL_Renderer* renderer,
    const RenderFace& face
) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_RenderLine(renderer, face.a.x, face.a.y, face.b.x, face.b.y);
    SDL_RenderLine(renderer, face.b.x, face.b.y, face.c.x, face.c.y);
    SDL_RenderLine(renderer, face.c.x, face.c.y, face.a.x, face.a.y);
}

void drawVertices(
    SDL_Renderer* renderer,
    const rf::model::ModelAsset& model,
    const ModelTransform& modelTransform,
    const Mat4& view,
    const Mat4& projection,
    const Camera& camera
) {
    SDL_SetRenderDrawColor(renderer, 255, 120, 80, 255);

    for (const rf::model::Vertex& vertex : model.vertices) {
        ScreenPoint point =
            projectModelVertex(
                vertex,
                modelTransform,
                view,
                projection,
                camera
            );

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
    const rf::model::ModelAsset& model,
    const ModelTransform& modelTransform,
    const RenderOptions& options,
    const RenderFace& renderFace
) {
    const rf::model::Face& face =
        *renderFace.face;

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
            renderFace.a,
            renderFace.b,
            renderFace.c
        );

        return;
    }

    const rf::model::TextureUVMapping& mapping =
        model.textureUVMappings[face.textureUVMappingIndex];

    if (!isValidTextureMapping(mapping, model.vertices)) {
        fillTriangle(
            renderer,
            renderFace.a,
            renderFace.b,
            renderFace.c
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
        renderFace.a,
        renderFace.b,
        renderFace.c,
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
        total += face.depth;
        count++;
    }

    for (const RenderFace& face : b) {
        total += face.depth;
        count++;
    }

    if (count == 0) {
        return NoDepth;
    }

    return total / static_cast<float>(count);
}

void drawPriorityBuckets(
    SDL_Renderer* renderer,
    const rf::model::ModelAsset& model,
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

        return buckets[highBucket][highIndex].depth;
    };

    auto drawNextHighPriority = [&]() {
        if (!hasHighPriorityFace()) {
            return;
        }

        drawRenderFace(
            renderer,
            model,
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

float screenArea(
    const ScreenPoint& a,
    const ScreenPoint& b,
    const ScreenPoint& c
) {
    return
        (b.x - a.x) * (c.y - a.y) -
        (b.y - a.y) * (c.x - a.x);
}

void drawModel(
    SDL_Renderer* renderer,
    const rf::model::ModelAsset& model,
    const Camera& camera,
    const RenderOptions& options,
    const ModelTransform& modelTransform
) {
    Mat4 view =
        buildViewMatrix(camera);

    Mat4 projection =
        buildProjectionMatrix(camera);

    std::vector<RenderFace> renderFaces;
    renderFaces.reserve(model.faces.size());

    for (const rf::model::Face& face : model.faces) {
        if (!isValidFace(face, model.vertices)) {
            continue;
        }

        ScreenPoint a =
            projectModelVertex(
                model.vertices[face.a],
                modelTransform,
                view,
                projection,
                camera
            );

        ScreenPoint b =
            projectModelVertex(
                model.vertices[face.b],
                modelTransform,
                view,
                projection,
                camera
            );

        ScreenPoint c =
            projectModelVertex(
                model.vertices[face.c],
                modelTransform,
                view,
                projection,
                camera
            );

        if (
            !isFinitePoint(a) ||
            !isFinitePoint(b) ||
            !isFinitePoint(c)
        ) {
            continue;
        }

        if (screenArea(a, b, c) <= 0.0f) {
            continue;
        }

        renderFaces.push_back({
            &face,
            a,
            b,
            c,
            (a.z + b.z + c.z) / 3.0f
        });
    }

    std::sort(
        renderFaces.begin(),
        renderFaces.end(),
        [](const RenderFace& a, const RenderFace& b) {
            return a.depth < b.depth;
        }
    );

    std::array<std::vector<RenderFace>, PriorityBucketCount> buckets;

    for (const RenderFace& face : renderFaces) {
        size_t priority =
            static_cast<size_t>(face.face->priority);

        if (priority >= PriorityBucketCount) {
            priority = 0;
        }

        buckets[priority].push_back(face);
    }

    if (options.fillTriangles) {
        drawPriorityBuckets(
            renderer,
            model,
            modelTransform,
            options,
            buckets
        );
    }

    if (options.showWireframe) {
        for (const std::vector<RenderFace>& bucket : buckets) {
            for (const RenderFace& face : bucket) {
                drawWireframe(renderer, face);
            }
        }
    }

    if (options.showVertices) {
        drawVertices(
            renderer,
            model,
            modelTransform,
            view,
            projection,
            camera
        );
    }
}

}
