#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <glad/gl.h>

#include "render/camera/Camera.h"
#include "render/model/ModelHandle.h"
#include "render/model/ModelResource.h"
#include "render/scene/Transform.h"
#include "render/texture/TextureManager.h"

namespace eld::render::opengl {

class SimpleRenderer {
public:
    explicit SimpleRenderer(
        const TextureManager& textures
    );

    ~SimpleRenderer();

    SimpleRenderer(
        const SimpleRenderer&
    ) = delete;

    SimpleRenderer& operator=(
        const SimpleRenderer&
    ) = delete;

    void beginFrame(
        const Camera& camera
    );

    void draw(
        ModelHandle handle,
        const ModelResource& model,
        const Transform& transform
    );

    void endFrame();

    void toggleWireframe();

private:
    struct GpuMesh {
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;

        GLsizei indexCount = 0;
    };

    struct GpuModel {
        std::vector<GpuMesh> meshes;
    };

    const GpuModel& ensureModel(
        ModelHandle handle,
        const ModelResource& model
    );

    GLuint ensureTexture(
        TextureHandle handle
    );

    void destroyGpuModel(
        GpuModel& model
    );

    static std::uint64_t modelKey(
        ModelHandle handle
    );

    static std::uint64_t textureKey(
        TextureHandle handle
    );

    const TextureManager& textures_;

    std::unordered_map<
        std::uint64_t,
        GpuModel
    > modelCache_;

    std::unordered_map<
        std::uint64_t,
        GLuint
    > textureCache_;

    GLuint program_ = 0;

    GLint modelLocation_ = -1;
    GLint viewLocation_ = -1;
    GLint projectionLocation_ = -1;

    GLint textureLocation_ = -1;
    GLint hasTextureLocation_ = -1;
    GLint alphaModeLocation_ = -1;
    GLint unlitLocation_ = -1;

    bool wireframe_ = false;
};

}
