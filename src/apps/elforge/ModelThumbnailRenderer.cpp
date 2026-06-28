#include "ModelThumbnailRenderer.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

#include "render/RenderPipeline.h"
#include "render/backend/software/SoftwareRenderBackend.h"
#include "render/scene/RenderObject.h"
#include "render/scene/RenderScene.h"

namespace eld::elforge {

namespace {

constexpr float Pi = 3.14159265358979323846f;

struct ModelBounds {
    eld::math::Vec3 minimum{
        0.0f,
        0.0f,
        0.0f
    };

    eld::math::Vec3 maximum{
        0.0f,
        0.0f,
        0.0f
    };

    bool valid = false;
};

ModelBounds calculateBounds(
    const eld::graphics::RenderModel& model
) {
    ModelBounds bounds;

    bounds.minimum = {
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()
    };

    bounds.maximum = {
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest()
    };

    for (const eld::graphics::RenderMesh& mesh : model.meshes) {
        for (const eld::graphics::RenderVertex& vertex : mesh.vertices) {
            bounds.minimum.x =
                std::min(bounds.minimum.x, vertex.position.x);
            bounds.minimum.y =
                std::min(bounds.minimum.y, vertex.position.y);
            bounds.minimum.z =
                std::min(bounds.minimum.z, vertex.position.z);

            bounds.maximum.x =
                std::max(bounds.maximum.x, vertex.position.x);
            bounds.maximum.y =
                std::max(bounds.maximum.y, vertex.position.y);
            bounds.maximum.z =
                std::max(bounds.maximum.z, vertex.position.z);

            bounds.valid = true;
        }
    }

    if (!bounds.valid) {
        bounds.minimum = {
            -1.0f,
            -1.0f,
            -1.0f
        };

        bounds.maximum = {
            1.0f,
            1.0f,
            1.0f
        };
    }

    return bounds;
}

eld::math::Vec3 boundsCenter(
    const ModelBounds& bounds
) {
    return {
        (bounds.minimum.x + bounds.maximum.x) * 0.5f,
        (bounds.minimum.y + bounds.maximum.y) * 0.5f,
        (bounds.minimum.z + bounds.maximum.z) * 0.5f
    };
}

float boundsLargestExtent(
    const ModelBounds& bounds
) {
    return std::max(
        {
            bounds.maximum.x - bounds.minimum.x,
            bounds.maximum.y - bounds.minimum.y,
            bounds.maximum.z - bounds.minimum.z,
            1.0f
        }
    );
}

float rsAngleToRadians(
    std::uint16_t angle
) {
    return
        static_cast<float>(angle) *
        2.0f *
        Pi /
        2048.0f;
}

eld::image::Image copyFramebuffer(
    const eld::render::Framebuffer& framebuffer
) {
    const eld::render::ColorBuffer& color =
        framebuffer.color();

    eld::image::Image image;

    image.width =
        static_cast<std::uint16_t>(
            std::min<std::uint32_t>(
                color.width(),
                65535
            )
        );

    image.height =
        static_cast<std::uint16_t>(
            std::min<std::uint32_t>(
                color.height(),
                65535
            )
        );

    image.pixels.resize(
        static_cast<std::size_t>(image.width) *
        image.height
    );

    for (std::uint16_t y = 0; y < image.height; ++y) {
        for (std::uint16_t x = 0; x < image.width; ++x) {
            const eld::render::ColorPixel& source =
                color.at(x, y);

            image.pixels[
                static_cast<std::size_t>(y) *
                image.width +
                x
            ] = eld::image::RgbaPixel{
                source.red,
                source.green,
                source.blue,
                source.alpha
            };
        }
    }

    return image;
}

}

eld::image::Image ModelThumbnailRenderer::render(
    eld::graphics::ModelHandle model,
    const eld::graphics::GraphicsResources& resources,
    std::uint16_t width,
    std::uint16_t height,
    std::uint16_t zoom,
    std::uint16_t rotationX,
    std::uint16_t rotationY
) const {
    eld::render::Camera camera;

    camera.viewportWidth =
        std::max<std::uint16_t>(1, width);

    camera.viewportHeight =
        std::max<std::uint16_t>(1, height);

    camera.position = {
        0.0f,
        0.0f,
        -512.0f
    };

    camera.rotation = {
        0.0f,
        0.0f,
        0.0f
    };

    camera.verticalFov = 0.75f;
    camera.nearPlane = 1.0f;
    camera.farPlane = 20000.0f;

    const eld::graphics::RenderModel& renderModel =
        resources.getModel(model);

    const ModelBounds bounds =
        calculateBounds(renderModel);

    const eld::math::Vec3 center =
        boundsCenter(bounds);

    const float largestExtent =
        boundsLargestExtent(bounds);

    const float widgetExtent =
        static_cast<float>(
            std::max<std::uint16_t>(
                std::min(width, height),
                1
            )
        );

    const float fitScale =
        widgetExtent /
        largestExtent *
        1.35f;

    const float zoomScale =
        static_cast<float>(
            std::max<std::uint16_t>(zoom, 128)
        ) /
        512.0f;

    const float scale =
        fitScale *
        zoomScale;

    eld::render::RenderObject object;

    object.model = model;
    object.transform.position = {
        -center.x * scale,
        center.y * scale,
        -center.z * scale
    };

    object.transform.rotation = {
        rsAngleToRadians(rotationX),
        rsAngleToRadians(rotationY),
        0.0f
    };

    object.transform.scale = {
        scale,
        -scale,
        scale
    };

    eld::render::RenderScene scene;
    scene.camera = camera;
    scene.objects.push_back(object);

    eld::render::SoftwareRenderBackend backend(nullptr);

    backend.setClearColor({
        0,
        0,
        0,
        0
    });

    eld::render::RenderPipeline pipeline;

    pipeline.render(
        scene,
        resources,
        backend
    );

    return copyFramebuffer(
        backend.framebuffer()
    );
}

}
