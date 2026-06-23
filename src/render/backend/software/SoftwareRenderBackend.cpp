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

    SDL_DestroyTexture(texture_);
    texture_ = nullptr;
}

void SoftwareRenderBackend::ensureTexture() {
    if (
        texture_ != nullptr ||
        renderer_ == nullptr ||
        width_ == 0 ||
        height_ == 0
    ) {
        return;
    }

    texture_ =
        SDL_CreateTexture(
            renderer_,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING,
            static_cast<int>(width_),
            static_cast<int>(height_)
        );

    if (texture_ != nullptr) {
        SDL_SetTextureScaleMode(
            texture_,
            SDL_SCALEMODE_NEAREST
        );
    }
}

void SoftwareRenderBackend::beginFrame(
    const Camera& camera
) {
    camera_ = camera;

    if (
        width_ != camera.viewportWidth ||
        height_ != camera.viewportHeight
    ) {
        width_ = camera.viewportWidth;
        height_ = camera.viewportHeight;

        destroyTexture();
    }

    framebuffer_.resize(
        width_,
        height_
    );

    framebuffer_.clear(
        clearColor_
    );

    ensureTexture();
}

void SoftwareRenderBackend::draw(
    const eld::graphics::RenderModel& model,
    const Transform& transform,
    const eld::graphics::GraphicsResources& resources
) {
    for (
        const eld::graphics::RenderMesh& mesh :
        model.meshes
    ) {
        const SoftwareProjectedMesh projected =
            projector_.project(
                mesh,
                transform,
                camera_
            );

        std::vector<
            const eld::graphics::RenderMeshSection*
        > sections;

        sections.reserve(
            mesh.sections.size()
        );

        for (
            const eld::graphics::RenderMeshSection& section :
            mesh.sections
        ) {
            sections.push_back(
                &section
            );
        }

        std::stable_sort(
            sections.begin(),
            sections.end(),
            [](
                const auto* left,
                const auto* right
            ) {
                return
                    left->sortOrder <
                    right->sortOrder;
            }
        );

        for (
            const eld::graphics::RenderMeshSection* section :
            sections
        ) {
            if (
                section == nullptr ||
                section->materialIndex >=
                    model.materials.size() ||
                section->firstIndex >
                    mesh.indices.size() ||
                section->indexCount >
                    mesh.indices.size() -
                    section->firstIndex
            ) {
                continue;
            }

            const eld::graphics::RenderMaterial& material =
                model.materials.at(
                    section->materialIndex
                );

            const eld::graphics::GraphicsTexture* texture =
                nullptr;

            if (material.texture.has_value()) {
                texture =
                    &resources.getTexture(
                        *material.texture
                    );
            }

            const std::size_t endIndex =
                static_cast<std::size_t>(
                    section->firstIndex
                ) +
                static_cast<std::size_t>(
                    section->indexCount
                );

            for (
                std::size_t index =
                    section->firstIndex;
                index + 2 < endIndex;
                index += 3
            ) {
                const std::uint32_t a =
                    mesh.indices.at(index);

                const std::uint32_t b =
                    mesh.indices.at(index + 1);

                const std::uint32_t c =
                    mesh.indices.at(index + 2);

                if (
                    a >= projected.vertices.size() ||
                    b >= projected.vertices.size() ||
                    c >= projected.vertices.size()
                ) {
                    continue;
                }

                rasterizer_.drawTriangle(
                    framebuffer_,
                    projected.vertices.at(a),
                    projected.vertices.at(b),
                    projected.vertices.at(c),
                    material,
                    texture
                );
            }
        }
    }
}

void SoftwareRenderBackend::endFrame() {
    if (
        renderer_ == nullptr ||
        texture_ == nullptr ||
        width_ == 0 ||
        height_ == 0
    ) {
        return;
    }

    SDL_UpdateTexture(
        texture_,
        nullptr,
        framebuffer_.color().data(),
        static_cast<int>(
            width_ * sizeof(ColorPixel)
        )
    );

    const SDL_FRect destination{
        static_cast<float>(outputX_),
        static_cast<float>(outputY_),
        static_cast<float>(width_),
        static_cast<float>(height_)
    };

    SDL_RenderTexture(
        renderer_,
        texture_,
        nullptr,
        &destination
    );
}

void SoftwareRenderBackend::setOutputPosition(
    int x,
    int y
) {
    outputX_ = x;
    outputY_ = y;
}

void SoftwareRenderBackend::setClearColor(
    ColorPixel color
) {
    clearColor_ = color;
}

const Framebuffer&
SoftwareRenderBackend::framebuffer() const {
    return framebuffer_;
}

}
