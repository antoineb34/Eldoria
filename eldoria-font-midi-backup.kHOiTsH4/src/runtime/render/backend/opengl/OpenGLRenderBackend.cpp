#include "OpenGLRenderBackend.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "camera/Projection.h"
#include "render/model/RenderModel.h"
#include "render/texture/GraphicsTexture.h"
#include "render/texture/SamplerState.h"
#include "scene/Transform.h"

namespace eld::render {

namespace {

struct GpuVertex {
    float position[3];
    float uv[2];
    float color[4];
};

std::string glString(
    const GLubyte* value
) {
    if (value == nullptr) {
        return {};
    }

    return reinterpret_cast<const char*>(value);
}

GLint textureFilter(
    eld::render::TextureFilter filter
) {
    switch (filter) {
        case eld::render::TextureFilter::Linear:
            return GL_LINEAR;
        case eld::render::TextureFilter::Nearest:
        default:
            return GL_NEAREST;
    }
}

GLint textureAddressMode(
    eld::render::TextureAddressMode mode
) {
    switch (mode) {
        case eld::render::TextureAddressMode::Clamp:
            return GL_CLAMP_TO_EDGE;
        case eld::render::TextureAddressMode::Repeat:
        default:
            return GL_REPEAT;
    }
}

constexpr const char* VertexShaderSource = R"GLSL(
#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec4 aColor;

uniform mat4 uMvp;
uniform float uDepthBias;

out vec2 vUv;
out vec4 vColor;

void main() {
    gl_Position = uMvp * vec4(aPosition, 1.0);
    gl_Position.z += uDepthBias * gl_Position.w;

    vUv = aUv;
    vColor = aColor;
}
)GLSL";

constexpr const char* FragmentShaderSource = R"GLSL(
#version 330 core

in vec2 vUv;
in vec4 vColor;

uniform vec4 uBaseColor;
uniform sampler2D uTexture;
uniform int uHasTexture;
uniform int uAlphaMode;

out vec4 fragColor;

void main() {
    vec4 sampled = vec4(1.0);

    if (uHasTexture != 0) {
        sampled = texture(uTexture, vUv);
    }

    vec4 color = vColor * uBaseColor * sampled;

    if (uAlphaMode == 1) {
        if (color.a < 0.5) {
            discard;
        }
        color.a = 1.0;
    }
    else if (uAlphaMode == 0) {
        color.a = 1.0;
    }

    if (color.a <= 0.0) {
        discard;
    }

    fragColor = color;
}
)GLSL";

}

OpenGLRenderBackend::OpenGLRenderBackend(
    SDL_Window* window
)
    : window_(window) {
    if (window_ == nullptr) {
        throw std::invalid_argument(
            "OpenGLRenderBackend requires an SDL OpenGL window"
        );
    }

    gl_.load();

    rendererName_ =
        glString(gl_.getString(GL_RENDERER));
    versionString_ =
        glString(gl_.getString(GL_VERSION));

    program_ = createProgram();

    mvpLocation_ =
        gl_.getUniformLocation(program_, "uMvp");
    baseColorLocation_ =
        gl_.getUniformLocation(program_, "uBaseColor");
    textureLocation_ =
        gl_.getUniformLocation(program_, "uTexture");
    hasTextureLocation_ =
        gl_.getUniformLocation(program_, "uHasTexture");
    alphaModeLocation_ =
        gl_.getUniformLocation(program_, "uAlphaMode");
    depthBiasLocation_ =
        gl_.getUniformLocation(program_, "uDepthBias");

    if (
        mvpLocation_ < 0 ||
        baseColorLocation_ < 0 ||
        textureLocation_ < 0 ||
        hasTextureLocation_ < 0 ||
        alphaModeLocation_ < 0 ||
        depthBiasLocation_ < 0
    ) {
        throw std::runtime_error(
            "OpenGL backend shader uniforms are incomplete"
        );
    }

    gl_.useProgram(program_);
    gl_.uniform1i(textureLocation_, 0);

    gl_.enable(GL_DEPTH_TEST);
    gl_.depthFunc(GL_LEQUAL);
    gl_.depthMask(GL_TRUE);

    gl_.enable(GL_CULL_FACE);
    gl_.cullFace(GL_BACK);
    gl_.frontFace(GL_CCW);

    gl_.blendFunc(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA
    );
}

OpenGLRenderBackend::~OpenGLRenderBackend() {
    destroyResources();
}

GLuint OpenGLRenderBackend::compileShader(
    GLenum type,
    const char* source
) {
    const GLuint shader =
        gl_.createShader(type);

    if (shader == 0) {
        throw std::runtime_error(
            "glCreateShader returned 0"
        );
    }

    gl_.shaderSource(
        shader,
        1,
        &source,
        nullptr
    );
    gl_.compileShader(shader);

    GLint compiled = GL_FALSE;
    gl_.getShaderiv(
        shader,
        GL_COMPILE_STATUS,
        &compiled
    );

    if (compiled == GL_TRUE) {
        return shader;
    }

    GLint logLength = 0;
    gl_.getShaderiv(
        shader,
        GL_INFO_LOG_LENGTH,
        &logLength
    );

    std::string log(
        static_cast<std::size_t>(
            std::max(logLength, 1)
        ),
        '\0'
    );

    gl_.getShaderInfoLog(
        shader,
        static_cast<GLsizei>(log.size()),
        nullptr,
        log.data()
    );

    gl_.deleteShader(shader);

    throw std::runtime_error(
        "OpenGL shader compilation failed: " +
        log
    );
}

GLuint OpenGLRenderBackend::createProgram() {
    const GLuint vertex =
        compileShader(
            GL_VERTEX_SHADER,
            VertexShaderSource
        );

    const GLuint fragment =
        compileShader(
            GL_FRAGMENT_SHADER,
            FragmentShaderSource
        );

    const GLuint program =
        gl_.createProgram();

    if (program == 0) {
        gl_.deleteShader(vertex);
        gl_.deleteShader(fragment);
        throw std::runtime_error(
            "glCreateProgram returned 0"
        );
    }

    gl_.attachShader(program, vertex);
    gl_.attachShader(program, fragment);
    gl_.linkProgram(program);

    gl_.deleteShader(vertex);
    gl_.deleteShader(fragment);

    GLint linked = GL_FALSE;
    gl_.getProgramiv(
        program,
        GL_LINK_STATUS,
        &linked
    );

    if (linked == GL_TRUE) {
        return program;
    }

    GLint logLength = 0;
    gl_.getProgramiv(
        program,
        GL_INFO_LOG_LENGTH,
        &logLength
    );

    std::string log(
        static_cast<std::size_t>(
            std::max(logLength, 1)
        ),
        '\0'
    );

    gl_.getProgramInfoLog(
        program,
        static_cast<GLsizei>(log.size()),
        nullptr,
        log.data()
    );

    gl_.deleteProgram(program);

    throw std::runtime_error(
        "OpenGL program link failed: " +
        log
    );
}

const OpenGLRenderBackend::GpuModel&
OpenGLRenderBackend::ensureModel(
    eld::render::ModelHandle handle,
    const eld::render::RenderModel& model
) {
    const auto existing =
        modelCache_.find(handle.value);

    if (existing != modelCache_.end()) {
        return existing->second;
    }

    GpuModel gpuModel;
    gpuModel.meshes.reserve(model.meshes.size());

    for (
        const eld::render::RenderMesh& mesh :
        model.meshes
    ) {
        if (
            mesh.vertices.size() >
                static_cast<std::size_t>(
                    std::numeric_limits<GLsizei>::max()
                ) ||
            mesh.indices.size() >
                static_cast<std::size_t>(
                    std::numeric_limits<GLsizei>::max()
                )
        ) {
            throw std::overflow_error(
                "Render mesh is too large for OpenGL draw counts"
            );
        }

        std::vector<GpuVertex> vertices;
        vertices.reserve(mesh.vertices.size());

        for (
            const eld::render::RenderVertex& vertex :
            mesh.vertices
        ) {
            vertices.push_back({
                {
                    vertex.position.x,
                    vertex.position.y,
                    vertex.position.z
                },
                {
                    vertex.uv.x,
                    vertex.uv.y
                },
                {
                    vertex.color.x,
                    vertex.color.y,
                    vertex.color.z,
                    vertex.color.w
                }
            });
        }

        GpuMesh gpuMesh;

        gl_.genVertexArrays(1, &gpuMesh.vao);
        gl_.genBuffers(1, &gpuMesh.vertexBuffer);
        gl_.genBuffers(1, &gpuMesh.indexBuffer);

        gl_.bindVertexArray(gpuMesh.vao);

        gl_.bindBuffer(
            GL_ARRAY_BUFFER,
            gpuMesh.vertexBuffer
        );
        gl_.bufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(
                vertices.size() * sizeof(GpuVertex)
            ),
            vertices.data(),
            GL_STATIC_DRAW
        );

        gl_.bindBuffer(
            GL_ELEMENT_ARRAY_BUFFER,
            gpuMesh.indexBuffer
        );
        gl_.bufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(
                mesh.indices.size() *
                sizeof(std::uint32_t)
            ),
            mesh.indices.data(),
            GL_STATIC_DRAW
        );

        gl_.enableVertexAttribArray(0);
        gl_.vertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(GpuVertex),
            reinterpret_cast<const void*>(
                offsetof(GpuVertex, position)
            )
        );

        gl_.enableVertexAttribArray(1);
        gl_.vertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(GpuVertex),
            reinterpret_cast<const void*>(
                offsetof(GpuVertex, uv)
            )
        );

        gl_.enableVertexAttribArray(2);
        gl_.vertexAttribPointer(
            2,
            4,
            GL_FLOAT,
            GL_FALSE,
            sizeof(GpuVertex),
            reinterpret_cast<const void*>(
                offsetof(GpuVertex, color)
            )
        );

        gpuModel.meshes.push_back(gpuMesh);
        stats_.uploadedMeshes++;
    }

    gl_.bindVertexArray(0);
    gl_.bindBuffer(GL_ARRAY_BUFFER, 0);

    const auto [inserted, created] =
        modelCache_.emplace(
            handle.value,
            std::move(gpuModel)
        );

    (void)created;
    stats_.uploadedModels++;

    return inserted->second;
}

GLuint OpenGLRenderBackend::ensureTexture(
    eld::render::TextureHandle handle,
    const eld::render::GraphicsResources& resources
) {
    const auto existing =
        textureCache_.find(handle.value);

    if (existing != textureCache_.end()) {
        return existing->second;
    }

    const eld::render::GraphicsTexture& texture =
        resources.getTexture(handle);

    if (
        texture.format !=
            eld::render::TextureFormat::Rgba8
    ) {
        throw std::runtime_error(
            "OpenGL backend only supports RGBA8 textures"
        );
    }

    const std::size_t expectedBytes =
        static_cast<std::size_t>(texture.width) *
        static_cast<std::size_t>(texture.height) *
        4u;

    if (texture.pixels.size() != expectedBytes) {
        throw std::runtime_error(
            "GraphicsTexture byte count does not match RGBA8 dimensions"
        );
    }

    GLuint gpuTexture = 0;
    gl_.genTextures(1, &gpuTexture);
    gl_.bindTexture(GL_TEXTURE_2D, gpuTexture);
    gl_.pixelStorei(GL_UNPACK_ALIGNMENT, 1);

    gl_.texImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        static_cast<GLsizei>(texture.width),
        static_cast<GLsizei>(texture.height),
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        texture.pixels.data()
    );

    gl_.texParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_NEAREST
    );
    gl_.texParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_NEAREST
    );
    gl_.texParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_REPEAT
    );
    gl_.texParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_REPEAT
    );

    textureCache_.emplace(
        handle.value,
        gpuTexture
    );
    stats_.uploadedTextures++;

    return gpuTexture;
}

void OpenGLRenderBackend::configureMaterial(
    const eld::render::RenderMaterial& material,
    const eld::render::GraphicsResources& resources
) {
    gl_.uniform4f(
        baseColorLocation_,
        material.baseColor.x,
        material.baseColor.y,
        material.baseColor.z,
        material.baseColor.w
    );

    if (material.doubleSided) {
        gl_.disable(GL_CULL_FACE);
    }
    else {
        gl_.enable(GL_CULL_FACE);
    }

    switch (material.alphaMode) {
        case eld::render::AlphaMode::Blended:
            gl_.enable(GL_BLEND);
            gl_.depthMask(GL_TRUE);
            gl_.uniform1i(alphaModeLocation_, 2);
            break;

        case eld::render::AlphaMode::Masked:
            gl_.disable(GL_BLEND);
            gl_.depthMask(GL_TRUE);
            gl_.uniform1i(alphaModeLocation_, 1);
            break;

        case eld::render::AlphaMode::Opaque:
        default:
            gl_.disable(GL_BLEND);
            gl_.depthMask(GL_TRUE);
            gl_.uniform1i(alphaModeLocation_, 0);
            break;
    }

    gl_.activeTexture(GL_TEXTURE0);

    if (material.texture.has_value()) {
        const GLuint texture =
            ensureTexture(
                *material.texture,
                resources
            );

        gl_.bindTexture(
            GL_TEXTURE_2D,
            texture
        );

        const GLint filter =
            textureFilter(
                material.sampler.filter
            );

        gl_.texParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            filter
        );
        gl_.texParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            filter
        );
        gl_.texParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            textureAddressMode(
                material.sampler.addressU
            )
        );
        gl_.texParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            textureAddressMode(
                material.sampler.addressV
            )
        );

        gl_.uniform1i(hasTextureLocation_, 1);
    }
    else {
        gl_.bindTexture(GL_TEXTURE_2D, 0);
        gl_.uniform1i(hasTextureLocation_, 0);
    }
}

void OpenGLRenderBackend::beginFrame(
    const Camera& camera
) {
    camera_ = camera;
    stats_.drawCalls = 0;

    gl_.viewport(
        0,
        0,
        static_cast<GLsizei>(camera.viewportWidth),
        static_cast<GLsizei>(camera.viewportHeight)
    );

    gl_.clearColor(
        clearColor_.x,
        clearColor_.y,
        clearColor_.z,
        clearColor_.w
    );

    gl_.enable(GL_DEPTH_TEST);
    gl_.depthMask(GL_TRUE);
    gl_.clear(
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT
    );

    gl_.useProgram(program_);
}

void OpenGLRenderBackend::draw(
    eld::render::ModelHandle modelHandle,
    const Transform& transform,
    const eld::render::GraphicsResources& resources
) {
    const eld::render::RenderModel& model =
        resources.getModel(modelHandle);

    const GpuModel& gpuModel =
        ensureModel(
            modelHandle,
            model
        );

    const eld::math::Mat4 mvp =
        buildModelMatrix(transform) *
        buildViewMatrix(camera_) *
        buildProjectionMatrix(camera_);

    // Mat4 is row-major and Eldoria multiplies row vectors. OpenGL reads
    // this memory as a column-major matrix, which is exactly the transpose
    // needed for the equivalent GLSL column-vector transform.
    gl_.uniformMatrix4fv(
        mvpLocation_,
        1,
        GL_FALSE,
        &mvp.m[0][0]
    );

    const float depthRange =
        std::max(
            camera_.farPlane - camera_.nearPlane,
            1.0f
        );

    for (
        std::size_t meshIndex = 0;
        meshIndex < model.meshes.size();
        meshIndex++
    ) {
        const eld::render::RenderMesh& mesh =
            model.meshes.at(meshIndex);
        const GpuMesh& gpuMesh =
            gpuModel.meshes.at(meshIndex);

        gl_.bindVertexArray(gpuMesh.vao);

        for (
            const eld::render::RenderMeshSection& section :
            mesh.sections
        ) {
            if (
                section.materialIndex >=
                    model.materials.size() ||
                section.firstIndex >
                    mesh.indices.size() ||
                section.indexCount >
                    mesh.indices.size() -
                    section.firstIndex
            ) {
                continue;
            }

            const eld::render::RenderMaterial& material =
                model.materials.at(
                    section.materialIndex
                );

            configureMaterial(
                material,
                resources
            );

            gl_.uniform1f(
                depthBiasLocation_,
                section.depthBias / depthRange
            );

            gl_.drawElements(
                GL_TRIANGLES,
                static_cast<GLsizei>(
                    section.indexCount
                ),
                GL_UNSIGNED_INT,
                reinterpret_cast<const void*>(
                    static_cast<std::uintptr_t>(
                        section.firstIndex
                    ) * sizeof(std::uint32_t)
                )
            );

            stats_.drawCalls++;
        }
    }

    gl_.bindVertexArray(0);
}

void OpenGLRenderBackend::endFrame() {
    if (
        presentEnabled_ &&
        window_ != nullptr
    ) {
        SDL_GL_SwapWindow(window_);
    }
}

void OpenGLRenderBackend::setClearColor(
    const eld::math::Vec4& color
) {
    clearColor_ = color;
}

void OpenGLRenderBackend::setPresentEnabled(
    bool enabled
) {
    presentEnabled_ = enabled;
}

void OpenGLRenderBackend::readColorBufferRgba(
    std::uint32_t width,
    std::uint32_t height,
    std::vector<std::uint8_t>& pixels
) {
    const std::size_t byteCount =
        static_cast<std::size_t>(width) *
        static_cast<std::size_t>(height) *
        4u;

    pixels.resize(byteCount);

    if (
        width == 0 ||
        height == 0
    ) {
        return;
    }

    gl_.pixelStorei(
        GL_PACK_ALIGNMENT,
        1
    );

    gl_.readPixels(
        0,
        0,
        static_cast<GLsizei>(width),
        static_cast<GLsizei>(height),
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels.data()
    );
}

const std::string&
OpenGLRenderBackend::rendererName() const {
    return rendererName_;
}

const std::string&
OpenGLRenderBackend::versionString() const {
    return versionString_;
}

const OpenGLBackendStats&
OpenGLRenderBackend::stats() const {
    return stats_;
}

void OpenGLRenderBackend::destroyResources() {
    for (auto& [handle, model] : modelCache_) {
        (void)handle;

        for (GpuMesh& mesh : model.meshes) {
            if (mesh.indexBuffer != 0) {
                gl_.deleteBuffers(
                    1,
                    &mesh.indexBuffer
                );
            }
            if (mesh.vertexBuffer != 0) {
                gl_.deleteBuffers(
                    1,
                    &mesh.vertexBuffer
                );
            }
            if (mesh.vao != 0) {
                gl_.deleteVertexArrays(
                    1,
                    &mesh.vao
                );
            }
        }
    }

    modelCache_.clear();

    for (auto& [handle, texture] : textureCache_) {
        (void)handle;

        if (texture != 0) {
            gl_.deleteTextures(1, &texture);
        }
    }

    textureCache_.clear();

    if (program_ != 0) {
        gl_.deleteProgram(program_);
        program_ = 0;
    }
}

}
