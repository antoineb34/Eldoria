#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL.h>

#include "../RenderBackend.h"
#include "OpenGLApi.h"
#include "math/Vec4.h"

namespace eld::render {

struct OpenGLBackendStats {
    std::size_t uploadedModels = 0;
    std::size_t uploadedMeshes = 0;
    std::size_t uploadedTextures = 0;
    std::size_t drawCalls = 0;
};

class OpenGLRenderBackend : public RenderBackend {
public:
    explicit OpenGLRenderBackend(
        SDL_Window* window
    );

    ~OpenGLRenderBackend() override;

    OpenGLRenderBackend(const OpenGLRenderBackend&) = delete;
    OpenGLRenderBackend& operator=(const OpenGLRenderBackend&) = delete;

    void beginFrame(
        const Camera& camera
    ) override;

    void draw(
        eld::render::ModelHandle model,
        const Transform& transform,
        const eld::render::GraphicsResources& resources
    ) override;

    void endFrame() override;

    void setClearColor(
        const eld::math::Vec4& color
    );

    void setPresentEnabled(
        bool enabled
    );

    void readColorBufferRgba(
        std::uint32_t width,
        std::uint32_t height,
        std::vector<std::uint8_t>& pixels
    );

    const std::string& rendererName() const;
    const std::string& versionString() const;
    const OpenGLBackendStats& stats() const;

private:
    struct GpuMesh {
        GLuint vao = 0;
        GLuint vertexBuffer = 0;
        GLuint indexBuffer = 0;
    };

    struct GpuModel {
        std::vector<GpuMesh> meshes;
    };

    GLuint compileShader(
        GLenum type,
        const char* source
    );

    GLuint createProgram();

    const GpuModel& ensureModel(
        eld::render::ModelHandle handle,
        const eld::render::RenderModel& model
    );

    GLuint ensureTexture(
        eld::render::TextureHandle handle,
        const eld::render::GraphicsResources& resources
    );

    void configureMaterial(
        const eld::render::RenderMaterial& material,
        const eld::render::GraphicsResources& resources
    );

    void destroyResources();

    SDL_Window* window_ = nullptr;
    opengl::OpenGLApi gl_;

    Camera camera_;

    eld::math::Vec4 clearColor_{
        0.094f,
        0.106f,
        0.125f,
        1.0f
    };

    GLuint program_ = 0;

    GLint mvpLocation_ = -1;
    GLint baseColorLocation_ = -1;
    GLint textureLocation_ = -1;
    GLint hasTextureLocation_ = -1;
    GLint alphaModeLocation_ = -1;
    GLint depthBiasLocation_ = -1;

    std::unordered_map<std::uint32_t, GpuModel>
        modelCache_;

    std::unordered_map<std::uint32_t, GLuint>
        textureCache_;

    std::string rendererName_;
    std::string versionString_;

    OpenGLBackendStats stats_;
    bool presentEnabled_ = true;
};

}
