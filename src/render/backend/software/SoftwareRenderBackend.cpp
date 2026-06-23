#include "SoftwareRenderBackend.h"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace eld::render {

SoftwareRenderBackend::SoftwareRenderBackend(
    SDL_Renderer* renderer
)
    : renderer_(renderer) {
}

SoftwareRenderBackend::~SoftwareRenderBackend() {
    destroyTexture();
}

void SoftwareRenderBackend::destroyTexture() {
    if (texture_ == nullptr) {
        return;
    }

    SDL_DestroyTexture(
        texture_
    );

    texture_ = nullptr;
}

void SoftwareRenderBackend::ensureTexture() {
    if (
        texture_ != nullptr ||
        renderer_ == nullptr ||
        width_ <= 0 ||
        height_ <= 0
    ) {
        return;
    }

    texture_ =
        SDL_CreateTexture(
            renderer_,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING,
            width_,
            height_
        );
}

void SoftwareRenderBackend::beginFrame(
    const RenderCamera& camera
) {
    camera_ = camera;

    viewportX_ =
        camera.viewportX;

    viewportY_ =
        camera.viewportY;

    width_ =
        std::max(
            camera.viewportWidth,
            0
        );

    height_ =
        std::max(
            camera.viewportHeight,
            0
        );

    projectedMeshes_.clear();

    framebuffer_.resize(
        width_,
        height_
    );

    framebuffer_.clear();

    destroyTexture();
    ensureTexture();
}

const SoftwareProjectedMesh&
SoftwareRenderBackend::project(
    const RenderObject& object
) {
    const auto existing =
        projectedMeshes_.find(
            &object
        );

    if (
        existing !=
        projectedMeshes_.end()
    ) {
        return existing->second;
    }

    auto inserted =
        projectedMeshes_.emplace(
            &object,
            projector_.project(
                object,
                camera_
            )
        );

    return inserted.first->second;
}

void SoftwareRenderBackend::draw(
    const RenderItem& item
) {
    if (
        item.object == nullptr ||
        item.submesh == nullptr ||
        item.material == nullptr ||
        item.object->model == nullptr
    ) {
        return;
    }

    const RenderObject& object =
        *item.object;

    const RenderMesh& mesh =
        object.model->mesh;

    const RenderSubmesh& submesh =
        *item.submesh;

    if (
        submesh.firstIndex >
        mesh.indices.size()
    ) {
        return;
    }

    const std::size_t endIndex =
        submesh.firstIndex +
        submesh.indexCount;

    if (
        endIndex >
        mesh.indices.size()
    ) {
        return;
    }

    const SoftwareProjectedMesh&
        projectedMesh =
            project(object);

    for (
        std::size_t index =
            submesh.firstIndex;
        index + 2 < endIndex;
        index += 3
    ) {
        const std::uint32_t a =
            mesh.indices[index];

        const std::uint32_t b =
            mesh.indices[index + 1];

        const std::uint32_t c =
            mesh.indices[index + 2];

        if (
            a >= projectedMesh.vertices.size() ||
            b >= projectedMesh.vertices.size() ||
            c >= projectedMesh.vertices.size()
        ) {
            continue;
        }

        rasterizer_.drawTriangle(
            framebuffer_,
            projectedMesh.vertices[a],
            projectedMesh.vertices[b],
            projectedMesh.vertices[c],
            *item.material
        );
    }
}

void SoftwareRenderBackend::endFrame() {
    if (
        renderer_ == nullptr ||
        texture_ == nullptr ||
        width_ <= 0 ||
        height_ <= 0
    ) {
        return;
    }

    std::vector<ColorPixel> pixels;

    pixels.reserve(
        static_cast<std::size_t>(
            width_
        ) *
        static_cast<std::size_t>(
            height_
        )
    );

    for (
        int y = 0;
        y < height_;
        y++
    ) {
        for (
            int x = 0;
            x < width_;
            x++
        ) {
            pixels.push_back(
                framebuffer_.color().at(
                    x,
                    y
                )
            );
        }
    }

    SDL_UpdateTexture(
        texture_,
        nullptr,
        pixels.data(),
        width_ *
            static_cast<int>(
                sizeof(ColorPixel)
            )
    );

    SDL_FRect destination{
        static_cast<float>(
            viewportX_
        ),
        static_cast<float>(
            viewportY_
        ),
        static_cast<float>(
            width_
        ),
        static_cast<float>(
            height_
        )
    };

    SDL_RenderTexture(
        renderer_,
        texture_,
        nullptr,
        &destination
    );
}

}
