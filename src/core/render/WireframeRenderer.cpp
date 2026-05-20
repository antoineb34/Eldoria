#include "WireframeRenderer.h"
#include "Color.h"
#include "TriangleRasterizer.h"

namespace rf::render {

void drawWireframeModel(
    SDL_Renderer* renderer,
    const std::vector<rf::model::Vertex>& vertices,
    const std::vector<rf::model::Face>& faces,
    const Camera& camera,
    bool showWireframe,
    bool showVertices,
    bool fillTriangles
) {
    SDL_SetRenderDrawColor(
        renderer,
        12,
        12,
        16,
        255
    );

    SDL_RenderClear(renderer);

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

        RgbColor color =
            rsColorToRgb(face.color);

        SDL_SetRenderDrawColor(
            renderer,
            color.r,
            color.g,
            color.b,
            255
        );

        if (fillTriangles) {
            fillTriangle(
                renderer,
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
