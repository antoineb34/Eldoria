#include "WireframeRenderer.h"
#include "Color.h"
#include "TriangleRasterizer.h"

#include <algorithm>
#include <vector>

namespace rf::render {

void drawWireframeModel(
    SDL_Renderer* renderer,
    DepthBuffer& depthBuffer,
    const std::vector<rf::model::Vertex>& vertices,
    const std::vector<rf::model::Face>& faces,
    const Camera& camera,
    bool showWireframe,
    bool showVertices,
    bool fillTriangles,
    bool useAlpha
) {
    SDL_SetRenderDrawColor(
        renderer,
        12,
        12,
        16,
        255
    );

    SDL_RenderClear(renderer);

    struct RenderFace {
        rf::model::Face face;
        ScreenPoint a;
        ScreenPoint b;
        ScreenPoint c;
        float depth = 0.0f;
    };

    std::vector<RenderFace> renderFaces;

    for (const rf::model::Face& face : faces) {

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

        ScreenPoint a =
            projectVertex(
                vertices[face.a],
                camera
            );

        ScreenPoint b =
            projectVertex(
                vertices[face.b],
                camera
            );

        ScreenPoint c =
            projectVertex(
                vertices[face.c],
                camera
            );

        float area =
            (b.x - a.x) * (c.y - a.y) -
            (b.y - a.y) * (c.x - a.x);

        if (area <= 0.0f) {
            continue;
        }

        float depth =
            (a.z + b.z + c.z) / 3.0f;

        renderFaces.push_back({
            face,
            a,
            b,
            c,
            depth
        });
    }

    std::sort(
        renderFaces.begin(),
        renderFaces.end(),
        [](const RenderFace& a,
           const RenderFace& b) {

            if (
                a.face.priority !=
                b.face.priority
            ) {
                return
                    a.face.priority <
                    b.face.priority;
            }

            return
                a.depth <
                b.depth;
        }
    );

    for (const RenderFace& renderFace : renderFaces) {

        const rf::model::Face& face =
            renderFace.face;

        ScreenPoint a =
            renderFace.a;

        ScreenPoint b =
            renderFace.b;

        ScreenPoint c =
            renderFace.c;

        RgbColor color =
            rsColorToRgb(
                face.color
            );

        uint8_t drawAlpha = 255;

        if (useAlpha && face.alpha > 0) {
            drawAlpha =
                255 - face.alpha;
        }

        SDL_SetRenderDrawColor(
            renderer,
            color.r,
            color.g,
            color.b,
            drawAlpha
        );

        if (fillTriangles) {
            fillTriangle(
                renderer,
                depthBuffer,
                a,
                b,
                c
            );
        }

        if (showWireframe) {

            SDL_SetRenderDrawColor(
                renderer,
                255,
                255,
                255,
                255
            );

            SDL_RenderLine(
                renderer,
                a.x, a.y,
                b.x, b.y
            );

            SDL_RenderLine(
                renderer,
                b.x, b.y,
                c.x, c.y
            );

            SDL_RenderLine(
                renderer,
                c.x, c.y,
                a.x, a.y
            );
        }
    }

    if (showVertices) {

        SDL_SetRenderDrawColor(
            renderer,
            255,
            120,
            80,
            255
        );

        for (const rf::model::Vertex& vertex : vertices) {

            ScreenPoint point =
                projectVertex(
                    vertex,
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

}
