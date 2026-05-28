#include "ModelRenderer.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "../software/camera/Projection.h"
#include "../software/color/Color.h"
#include "../software/math/Mat4.h"
#include "../software/raster/TriangleRasterizer.h"

#include "RenderFace.h"

namespace rf::render {

namespace {

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

bool isFinitePoint(
    const ScreenPoint& point
) {
    return
        std::isfinite(point.x) &&
        std::isfinite(point.y) &&
        std::isfinite(point.z);
}

rf::model::Vertex transformVertex(
    rf::model::Vertex vertex,
    const ModelTransform& transform
) {
    float x =
        static_cast<float>(vertex.x) * transform.scale;

    float y =
        static_cast<float>(vertex.y) * transform.scale;

    float z =
        static_cast<float>(vertex.z) * transform.scale;

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

    return {
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(z)
    };
}

TextureMappingPoint toTexturePoint(
    const rf::model::Vertex& vertex,
    const ModelTransform& modelTransform
) {
    rf::model::Vertex transformed =
        transformVertex(
            vertex,
            modelTransform
        );

    return {
        static_cast<float>(transformed.x),
        static_cast<float>(transformed.y),
        static_cast<float>(transformed.z)
    };
}

ScreenPoint projectModelVertex(
    const rf::model::Vertex& vertex,
    const ModelTransform& modelTransform,
    const Mat4& cameraTransform,
    const Camera& camera
) {
    rf::model::Vertex transformed =
        transformVertex(
            vertex,
            modelTransform
        );

    return projectVertex(
        transformed,
        cameraTransform,
        camera
    );
}

void drawWireframe(
    SDL_Renderer* renderer,
    const RenderFace& face
) {
    SDL_SetRenderDrawColor(
        renderer,
        255,
        255,
        255,
        255
    );

    SDL_RenderLine(renderer, face.a.x, face.a.y, face.b.x, face.b.y);
    SDL_RenderLine(renderer, face.b.x, face.b.y, face.c.x, face.c.y);
    SDL_RenderLine(renderer, face.c.x, face.c.y, face.a.x, face.a.y);
}

void drawVertices(
    SDL_Renderer* renderer,
    const rf::model::ModelAsset& model,
    const ModelTransform& modelTransform,
    const Mat4& cameraTransform,
    const Camera& camera
) {
    SDL_SetRenderDrawColor(
        renderer,
        255,
        120,
        80,
        255
    );

    for (const rf::model::Vertex& vertex : model.vertices) {
        ScreenPoint point =
            projectModelVertex(
                vertex,
                modelTransform,
                cameraTransform,
                camera
            );

        SDL_FRect rect {
            point.x - 2.0f,
            point.y - 2.0f,
            4.0f,
            4.0f
        };

        SDL_RenderFillRect(
            renderer,
            &rect
        );
    }
}

}

void drawModel(
    SDL_Renderer* renderer,
    DepthBuffer& depthBuffer,
    const rf::model::ModelAsset& model,
    const Camera& camera,
    const RenderOptions& options,
    const ModelTransform& modelTransform
) {
    Mat4 view =
        buildViewMatrix(camera);

    Mat4 projection =
        buildProjectionMatrix(camera);

    Mat4 cameraTransform =
        view * projection;

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
                cameraTransform,
                camera
            );

        ScreenPoint b =
            projectModelVertex(
                model.vertices[face.b],
                modelTransform,
                cameraTransform,
                camera
            );

        ScreenPoint c =
            projectModelVertex(
                model.vertices[face.c],
                modelTransform,
                cameraTransform,
                camera
            );

        if (
            !isFinitePoint(a) ||
            !isFinitePoint(b) ||
            !isFinitePoint(c)
        ) {
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
            if (a.face->priority != b.face->priority) {
                return a.face->priority < b.face->priority;
            }

            return a.depth < b.depth;
        }
    );

    if (options.fillTriangles) {
        for (const RenderFace& renderFace : renderFaces) {
            const rf::model::Face& face =
                *renderFace.face;

            RgbColor color =
                rsColorToRgb(face.color);

            uint8_t alpha = 255;

            if (options.useAlpha && face.alpha > 0) {
                alpha =
                    static_cast<uint8_t>(
                        255 - face.alpha
                    );
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
                    static_cast<int>(
                        model.textureUVMappings.size()
                    );

            bool isTexturedRenderType =
                face.renderType == 2 ||
                face.renderType == 3;

            bool textured =
                isTexturedRenderType &&
                hasMapping &&
                textureIt != model.textures.end();

            if (textured) {
                const rf::model::TextureUVMapping& mapping =
                    model.textureUVMappings[
                        face.textureUVMappingIndex
                    ];

                if (isValidTextureMapping(mapping, model.vertices)) {
                    TextureMappingPoint faceA =
                        toTexturePoint(
                            model.vertices[face.a],
                            modelTransform
                        );

                    TextureMappingPoint faceB =
                        toTexturePoint(
                            model.vertices[face.b],
                            modelTransform
                        );

                    TextureMappingPoint faceC =
                        toTexturePoint(
                            model.vertices[face.c],
                            modelTransform
                        );

                    TextureMappingPoint textureOrigin =
                        toTexturePoint(
                            model.vertices[mapping.originVertex],
                            modelTransform
                        );

                    TextureMappingPoint textureU =
                        toTexturePoint(
                            model.vertices[mapping.uVertex],
                            modelTransform
                        );

                    TextureMappingPoint textureV =
                        toTexturePoint(
                            model.vertices[mapping.vVertex],
                            modelTransform
                        );

                    fillTexturedTriangle(
                        renderer,
                        depthBuffer,
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
                else {
                    fillTriangle(
                        renderer,
                        depthBuffer,
                        renderFace.a,
                        renderFace.b,
                        renderFace.c
                    );
                }
            }
            else {
                fillTriangle(
                    renderer,
                    depthBuffer,
                    renderFace.a,
                    renderFace.b,
                    renderFace.c
                );
            }
        }
    }

    if (options.showWireframe) {
        for (const RenderFace& renderFace : renderFaces) {
            drawWireframe(
                renderer,
                renderFace
            );
        }
    }

    if (options.showVertices) {
        drawVertices(
            renderer,
            model,
            modelTransform,
            cameraTransform,
            camera
        );
    }
}

}
