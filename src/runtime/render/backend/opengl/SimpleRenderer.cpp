#include "SimpleRenderer.h"

#include <SDL3/SDL.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "camera/Projection.h"
#include "scene/Transform.h"

namespace eld::render::opengl
{

    namespace
    {

        GLuint compileShader(
            GLenum type,
            const char *source)
        {
            const GLuint shader =
                glCreateShader(type);

            glShaderSource(
                shader,
                1,
                &source,
                nullptr);

            glCompileShader(shader);

            GLint success = GL_FALSE;

            glGetShaderiv(
                shader,
                GL_COMPILE_STATUS,
                &success);

            if (success == GL_TRUE)
            {
                return shader;
            }

            GLint length = 0;

            glGetShaderiv(
                shader,
                GL_INFO_LOG_LENGTH,
                &length);

            std::string log(
                static_cast<std::size_t>(length),
                '\0');

            if (length > 0)
            {
                glGetShaderInfoLog(
                    shader,
                    length,
                    nullptr,
                    log.data());
            }

            glDeleteShader(shader);

            throw std::runtime_error(
                "Shader compilation failed:\n" +
                log);
        }

    }

    SimpleRenderer::SimpleRenderer(
        const TextureManager &textures)
        : textures_(textures)
    {
        const int version =
            gladLoadGL(
                reinterpret_cast<GLADloadfunc>(
                    SDL_GL_GetProcAddress));

        if (version == 0)
        {
            throw std::runtime_error(
                "Failed to load OpenGL functions");
        }

        const char *vertexSource = R"(
        #version 330 core

        layout(location = 0) in vec3 aPosition;
        layout(location = 1) in vec4 aColor;
        layout(location = 2) in vec3 aNormal;
        layout(location = 3) in vec2 aUV;

        out vec4 vColor;
        out vec3 vNormal;
        out vec2 vUV;

        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;

        void main() {
            vColor = aColor;
            vNormal =
                mat3(uModel) *
                aNormal;

            vUV = aUV;

            gl_Position =
                uProjection *
                uView *
                uModel *
                vec4(
                    aPosition,
                    1.0
                );
        }
    )";

        const char *fragmentSource = R"(
        #version 330 core

        in vec4 vColor;
        in vec3 vNormal;
        in vec2 vUV;

        out vec4 FragColor;

        uniform sampler2D uTexture;
        uniform bool uHasTexture;
        uniform bool uUnlit;
        uniform int uAlphaMode;

        void main() {
            float brightness =
                1.0;

            if (!uUnlit)
            {
                vec3 lightDirection =
                    normalize(
                        vec3(
                            0.5,
                            1.0,
                            0.4
                        )
                    );

                vec3 normal =
                    normalize(vNormal);

                float diffuse =
                    max(
                        dot(
                            normal,
                            lightDirection
                        ),
                        0.0
                    );

                brightness =
                    0.60 +
                    diffuse * 0.50f;
            }

            vec4 sampled =
                uHasTexture
                    ? texture(
                          uTexture,
                          vUV
                      )
                    : vec4(1.0);

            vec4 surfaceColor =
                vColor * sampled;

            if (uAlphaMode == 1)
            {
                if (surfaceColor.a < 0.5)
                {
                    discard;
                }

                surfaceColor.a = 1.0;
            }
            else if (uAlphaMode == 0)
            {
                surfaceColor.a = 1.0;
            }

            if (surfaceColor.a <= 0.0)
            {
                discard;
            }

            FragColor =
                vec4(
                    surfaceColor.rgb *
                        brightness,
                    surfaceColor.a
                );
        }
    )";

        const GLuint vertexShader =
            compileShader(
                GL_VERTEX_SHADER,
                vertexSource);

        GLuint fragmentShader = 0;

        try
        {
            fragmentShader =
                compileShader(
                    GL_FRAGMENT_SHADER,
                    fragmentSource);
        }
        catch (...)
        {
            glDeleteShader(
                vertexShader);

            throw;
        }

        program_ =
            glCreateProgram();

        glAttachShader(
            program_,
            vertexShader);

        glAttachShader(
            program_,
            fragmentShader);

        glLinkProgram(
            program_);

        glDeleteShader(
            vertexShader);

        glDeleteShader(
            fragmentShader);

        GLint success = GL_FALSE;

        glGetProgramiv(
            program_,
            GL_LINK_STATUS,
            &success);

        if (success != GL_TRUE)
        {
            GLint length = 0;

            glGetProgramiv(
                program_,
                GL_INFO_LOG_LENGTH,
                &length);

            std::string log(
                static_cast<std::size_t>(
                    length),
                '\0');

            if (length > 0)
            {
                glGetProgramInfoLog(
                    program_,
                    length,
                    nullptr,
                    log.data());
            }

            throw std::runtime_error(
                "Shader program linking failed:\n" +
                log);
        }

        modelLocation_ =
            glGetUniformLocation(
                program_,
                "uModel");

        viewLocation_ =
            glGetUniformLocation(
                program_,
                "uView");

        projectionLocation_ =
            glGetUniformLocation(
                program_,
                "uProjection");

        textureLocation_ =
            glGetUniformLocation(
                program_,
                "uTexture");

        hasTextureLocation_ =
            glGetUniformLocation(
                program_,
                "uHasTexture");

        alphaModeLocation_ =
            glGetUniformLocation(
                program_,
                "uAlphaMode");

        unlitLocation_ =
            glGetUniformLocation(
                program_,
                "uUnlit");

        if (
            modelLocation_ == -1 ||
            viewLocation_ == -1 ||
            projectionLocation_ == -1 ||
            textureLocation_ == -1 ||
            hasTextureLocation_ == -1 ||
            alphaModeLocation_ == -1 ||
            unlitLocation_ == -1)
        {
            throw std::runtime_error(
                "Could not find renderer "
                "matrix uniforms");
        }

        glEnable(
            GL_DEPTH_TEST);

        glDepthFunc(
            GL_LESS);

        glBlendFunc(
            GL_SRC_ALPHA,
            GL_ONE_MINUS_SRC_ALPHA);
    }

    SimpleRenderer::~SimpleRenderer()
    {
        for (
            auto &[key, model] :
            modelCache_)
        {
            (void)key;

            destroyGpuModel(
                model);
        }

        for (
            auto &[key, texture] :
            textureCache_)
        {
            (void)key;

            if (texture != 0)
            {
                glDeleteTextures(
                    1,
                    &texture);
            }
        }

        if (program_ != 0)
        {
            glDeleteProgram(
                program_);
        }
    }

    std::uint64_t
    SimpleRenderer::modelKey(
        ModelHandle handle)
    {
        return (
                   static_cast<std::uint64_t>(
                       handle.generation)
                   << 32u) |
               static_cast<std::uint64_t>(
                   handle.index);
    }

    std::uint64_t
    SimpleRenderer::textureKey(
        TextureHandle handle)
    {
        return (
                   static_cast<std::uint64_t>(
                       handle.generation)
                   << 32u) |
               static_cast<std::uint64_t>(
                   handle.index);
    }

    void SimpleRenderer::destroyGpuModel(
        GpuModel &model)
    {
        for (
            GpuMesh &mesh :
            model.meshes)
        {
            if (mesh.ebo != 0)
            {
                glDeleteBuffers(
                    1,
                    &mesh.ebo);
            }

            if (mesh.vbo != 0)
            {
                glDeleteBuffers(
                    1,
                    &mesh.vbo);
            }

            if (mesh.vao != 0)
            {
                glDeleteVertexArrays(
                    1,
                    &mesh.vao);
            }
        }
    }

    const SimpleRenderer::GpuModel &
    SimpleRenderer::ensureModel(
        ModelHandle handle,
        const ModelResource &model)
    {
        const std::uint64_t key =
            modelKey(handle);

        const auto existing =
            modelCache_.find(key);

        if (
            existing !=
            modelCache_.end())
        {
            return existing->second;
        }

        GpuModel gpuModel;

        gpuModel.meshes.reserve(
            model.meshes.size());

        for (
            const RenderMesh &mesh :
            model.meshes)
        {
            GpuMesh gpuMesh;

            std::vector<float>
                vertices;

            vertices.reserve(
                mesh.vertices.size() *
                12);

            for (
                const RenderVertex &vertex :
                mesh.vertices)
            {
                vertices.push_back(
                    vertex.position.x);

                vertices.push_back(
                    vertex.position.y);

                vertices.push_back(
                    vertex.position.z);

                vertices.push_back(
                    vertex.color.x);

                vertices.push_back(
                    vertex.color.y);

                vertices.push_back(
                    vertex.color.z);

                vertices.push_back(
                    vertex.color.w);

                vertices.push_back(
                    vertex.normal.x);

                vertices.push_back(
                    vertex.normal.y);

                vertices.push_back(
                    vertex.normal.z);

                vertices.push_back(
                    vertex.uv.x);

                vertices.push_back(
                    vertex.uv.y);
            }

            glGenVertexArrays(
                1,
                &gpuMesh.vao);

            glBindVertexArray(
                gpuMesh.vao);

            glGenBuffers(
                1,
                &gpuMesh.vbo);

            glBindBuffer(
                GL_ARRAY_BUFFER,
                gpuMesh.vbo);

            glBufferData(
                GL_ARRAY_BUFFER,
                vertices.size() *
                    sizeof(float),
                vertices.data(),
                GL_STATIC_DRAW);

            glGenBuffers(
                1,
                &gpuMesh.ebo);

            glBindBuffer(
                GL_ELEMENT_ARRAY_BUFFER,
                gpuMesh.ebo);

            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                mesh.indices.size() *
                    sizeof(std::uint32_t),
                mesh.indices.data(),
                GL_STATIC_DRAW);

            constexpr GLsizei stride =
                12 * sizeof(float);

            glVertexAttribPointer(
                0,
                3,
                GL_FLOAT,
                GL_FALSE,
                stride,
                nullptr);

            glEnableVertexAttribArray(
                0);

            glVertexAttribPointer(
                1,
                4,
                GL_FLOAT,
                GL_FALSE,
                stride,
                reinterpret_cast<
                    const void *>(
                    3 * sizeof(float)));

            glEnableVertexAttribArray(
                1);

            glVertexAttribPointer(
                2,
                3,
                GL_FLOAT,
                GL_FALSE,
                stride,
                reinterpret_cast<
                    const void *>(
                    7 * sizeof(float)));

            glEnableVertexAttribArray(
                2);

            glVertexAttribPointer(
                3,
                2,
                GL_FLOAT,
                GL_FALSE,
                stride,
                reinterpret_cast<
                    const void *>(
                    10 * sizeof(float)));

            glEnableVertexAttribArray(
                3);

            gpuMesh.indexCount =
                static_cast<GLsizei>(
                    mesh.indices.size());

            glBindVertexArray(0);

            glBindBuffer(
                GL_ARRAY_BUFFER,
                0);

            gpuModel.meshes.push_back(
                std::move(gpuMesh));
        }

        auto [it, inserted] =
            modelCache_.emplace(
                key,
                std::move(gpuModel));

        (void)inserted;

        return it->second;
    }

    GLuint SimpleRenderer::ensureTexture(
        TextureHandle handle)
    {
        const std::uint64_t key =
            textureKey(handle);

        const auto existing =
            textureCache_.find(key);

        if (
            existing !=
            textureCache_.end())
        {
            return existing->second;
        }

        const TextureResource &resource =
            textures_.get(handle);

        if (
            resource.width == 0 ||
            resource.height == 0 ||
            resource.rgba.empty())
        {
            return 0;
        }

        const std::size_t
            expectedBytes =
                static_cast<std::size_t>(
                    resource.width) *
                static_cast<std::size_t>(
                    resource.height) *
                4u;

        if (
            resource.rgba.size() !=
            expectedBytes)
        {
            throw std::runtime_error(
                "Texture RGBA byte count "
                "does not match texture "
                "dimensions");
        }

        GLuint texture = 0;

        glGenTextures(
            1,
            &texture);

        glBindTexture(
            GL_TEXTURE_2D,
            texture);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_NEAREST);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            GL_NEAREST);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_REPEAT);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_REPEAT);

        glPixelStorei(
            GL_UNPACK_ALIGNMENT,
            1);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            static_cast<GLsizei>(
                resource.width),
            static_cast<GLsizei>(
                resource.height),
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            resource.rgba.data());

        textureCache_.emplace(
            key,
            texture);

        return texture;
    }

    void SimpleRenderer::beginFrame(
        const Camera &camera)
    {
        if (
            camera.viewportWidth == 0 ||
            camera.viewportHeight == 0)
        {
            return;
        }

        glViewport(
            0,
            0,
            static_cast<GLsizei>(
                camera.viewportWidth),
            static_cast<GLsizei>(
                camera.viewportHeight));

        glClearColor(
            0.1f,
            0.2f,
            0.3f,
            1.0f);

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

        // --------------------------------------------------------
        // VIEW
        //
        // Preserve the camera convention from the original
        // SimpleRenderer:
        //
        // yaw = 0, pitch = 0
        //     -> look toward negative Z.
        // --------------------------------------------------------

        const float yaw =
            camera.rotation.y;

        const float pitch =
            camera.rotation.x;

        const float cyaw =
            std::cos(yaw);

        const float syaw =
            std::sin(yaw);

        const float cpitch =
            std::cos(pitch);

        const float spitch =
            std::sin(pitch);

        const float fx =
            -syaw * cpitch;

        const float fy =
            spitch;

        const float fz =
            -cyaw * cpitch;

        const float rx =
            cyaw;

        const float ry =
            0.0f;

        const float rz =
            -syaw;

        const float ux =
            syaw * spitch;

        const float uy =
            cpitch;

        const float uz =
            cyaw * spitch;

        // OpenGL column-major view matrix.
        const float view[] = {
            rx, ux, -fx, 0.0f,
            ry, uy, -fy, 0.0f,
            rz, uz, -fz, 0.0f,

            -(
                rx * camera.position.x +
                ry * camera.position.y +
                rz * camera.position.z),

            -(
                ux * camera.position.x +
                uy * camera.position.y +
                uz * camera.position.z),

            fx * camera.position.x +
                fy * camera.position.y +
                fz * camera.position.z,

            1.0f};

        // --------------------------------------------------------
        // PROJECTION
        //
        // Standard OpenGL perspective projection.
        // OpenGL camera space looks toward negative Z.
        // --------------------------------------------------------

        const float aspect =
            static_cast<float>(
                camera.viewportWidth) /
            static_cast<float>(
                camera.viewportHeight);

        const float f =
            1.0f /
            std::tan(
                camera.verticalFov *
                0.5f);

        const float nearPlane =
            camera.nearPlane;

        const float farPlane =
            camera.farPlane;

        const float projection[] = {
            f / aspect,
            0.0f,
            0.0f,
            0.0f,

            0.0f,
            f,
            0.0f,
            0.0f,

            0.0f,
            0.0f,
            (
                farPlane +
                nearPlane) /
                (nearPlane -
                 farPlane),
            -1.0f,

            0.0f,
            0.0f,
            (
                2.0f *
                farPlane *
                nearPlane) /
                (nearPlane -
                 farPlane),
            0.0f};

        glUseProgram(
            program_);

        glUniformMatrix4fv(
            viewLocation_,
            1,
            GL_FALSE,
            view);

        glUniformMatrix4fv(
            projectionLocation_,
            1,
            GL_FALSE,
            projection);

        glActiveTexture(
            GL_TEXTURE0);

        glUniform1i(
            textureLocation_,
            0);

        glPolygonMode(
            GL_FRONT_AND_BACK,
            wireframe_
                ? GL_LINE
                : GL_FILL);
    }

    void SimpleRenderer::draw(
        ModelHandle handle,
        const ModelResource &model,
        const Transform &transform)
    {
        const GpuModel &gpuModel =
            ensureModel(
                handle,
                model);

        const eld::math::Mat4
            modelMatrix =
                buildModelMatrix(
                    transform);

        glUniformMatrix4fv(
            modelLocation_,
            1,
            GL_FALSE,
            &modelMatrix.m[0][0]);

        glUniform1i(
            unlitLocation_,
            GL_FALSE);

        for (
            std::size_t meshIndex = 0;
            meshIndex <
            model.meshes.size();
            ++meshIndex)
        {
            const RenderMesh &mesh =
                model.meshes.at(
                    meshIndex);

            const GpuMesh &gpuMesh =
                gpuModel.meshes.at(
                    meshIndex);

            glBindVertexArray(
                gpuMesh.vao);

            if (
                mesh.sections.empty())
            {
                glBindTexture(
                    GL_TEXTURE_2D,
                    0);

                glUniform1i(
                    hasTextureLocation_,
                    GL_FALSE);

                glUniform1i(
                    alphaModeLocation_,
                    0);

                glDisable(
                    GL_BLEND);

                glDepthMask(
                    GL_TRUE);

                glDrawElements(
                    GL_TRIANGLES,
                    gpuMesh.indexCount,
                    GL_UNSIGNED_INT,
                    nullptr);

                continue;
            }

            for (
                const RenderMeshSection &
                    section :
                mesh.sections)
            {
                if (
                    section.firstIndex >
                        mesh.indices.size() ||
                    section.indexCount >
                        mesh.indices.size() -
                            section.firstIndex)
                {
                    continue;
                }

                GLuint texture = 0;

                const RenderMaterial *material =
                    nullptr;

                if (
                    section.materialIndex <
                    model.materials.size())
                {
                    material =
                        &model.materials.at(
                            section.materialIndex);

                    if (
                        material->texture.has_value() &&
                        textures_.isValid(
                            *material->texture))
                    {
                        texture =
                            ensureTexture(
                                *material->texture);
                    }
                }

                glBindTexture(
                    GL_TEXTURE_2D,
                    texture);

                glUniform1i(
                    hasTextureLocation_,
                    texture != 0
                        ? GL_TRUE
                        : GL_FALSE);

                int alphaMode = 0;

                glDisable(
                    GL_BLEND);

                glDepthMask(
                    GL_TRUE);

                if (material != nullptr)
                {
                    switch (material->alphaMode)
                    {
                    case AlphaMode::Masked:
                        alphaMode = 1;
                        break;

                    case AlphaMode::Blended:
                        alphaMode = 2;

                        glEnable(
                            GL_BLEND);

                        break;

                    case AlphaMode::Opaque:
                    default:
                        alphaMode = 0;
                        break;
                    }

                    if (texture != 0)
                    {
                        const GLint filter =
                            material->sampler.filter ==
                                    TextureFilter::Linear
                                ? GL_LINEAR
                                : GL_NEAREST;

                        const GLint wrapU =
                            material->sampler.addressU ==
                                    TextureAddressMode::Clamp
                                ? GL_CLAMP_TO_EDGE
                                : GL_REPEAT;

                        const GLint wrapV =
                            material->sampler.addressV ==
                                    TextureAddressMode::Clamp
                                ? GL_CLAMP_TO_EDGE
                                : GL_REPEAT;

                        glTexParameteri(
                            GL_TEXTURE_2D,
                            GL_TEXTURE_MIN_FILTER,
                            filter);

                        glTexParameteri(
                            GL_TEXTURE_2D,
                            GL_TEXTURE_MAG_FILTER,
                            filter);

                        glTexParameteri(
                            GL_TEXTURE_2D,
                            GL_TEXTURE_WRAP_S,
                            wrapU);

                        glTexParameteri(
                            GL_TEXTURE_2D,
                            GL_TEXTURE_WRAP_T,
                            wrapV);
                    }
                }

                glUniform1i(
                    alphaModeLocation_,
                    alphaMode);

                glUniform1i(
                    unlitLocation_,
                    material != nullptr &&
                            material->unlit
                        ? GL_TRUE
                        : GL_FALSE);

                const std::uintptr_t
                    byteOffset =
                        static_cast<
                            std::uintptr_t>(
                            section.firstIndex) *
                        sizeof(
                            std::uint32_t);

                glDrawElements(
                    GL_TRIANGLES,
                    static_cast<GLsizei>(
                        section.indexCount),
                    GL_UNSIGNED_INT,
                    reinterpret_cast<
                        const void *>(
                        byteOffset));
            }
        }
    }

    void SimpleRenderer::endFrame()
    {
        glBindTexture(
            GL_TEXTURE_2D,
            0);

        glBindVertexArray(
            0);

        glUseProgram(
            0);

        glPolygonMode(
            GL_FRONT_AND_BACK,
            GL_FILL);
    }

    void SimpleRenderer::toggleWireframe()
    {
        wireframe_ =
            !wireframe_;
    }

}
