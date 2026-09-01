#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

namespace eld::render::opengl {

struct OpenGLApi {
    using GetStringFn =
        const GLubyte* (APIENTRY *)(GLenum);
    using ViewportFn =
        void (APIENTRY *)(GLint, GLint, GLsizei, GLsizei);
    using ClearColorFn =
        void (APIENTRY *)(GLfloat, GLfloat, GLfloat, GLfloat);
    using ClearFn =
        void (APIENTRY *)(GLbitfield);
    using EnableFn =
        void (APIENTRY *)(GLenum);
    using DisableFn =
        void (APIENTRY *)(GLenum);
    using DepthFuncFn =
        void (APIENTRY *)(GLenum);
    using DepthMaskFn =
        void (APIENTRY *)(GLboolean);
    using CullFaceFn =
        void (APIENTRY *)(GLenum);
    using FrontFaceFn =
        void (APIENTRY *)(GLenum);
    using BlendFuncFn =
        void (APIENTRY *)(GLenum, GLenum);

    using GenVertexArraysFn =
        void (APIENTRY *)(GLsizei, GLuint*);
    using BindVertexArrayFn =
        void (APIENTRY *)(GLuint);
    using DeleteVertexArraysFn =
        void (APIENTRY *)(GLsizei, const GLuint*);

    using GenBuffersFn =
        void (APIENTRY *)(GLsizei, GLuint*);
    using BindBufferFn =
        void (APIENTRY *)(GLenum, GLuint);
    using BufferDataFn =
        void (APIENTRY *)(GLenum, GLsizeiptr, const void*, GLenum);
    using DeleteBuffersFn =
        void (APIENTRY *)(GLsizei, const GLuint*);

    using EnableVertexAttribArrayFn =
        void (APIENTRY *)(GLuint);
    using VertexAttribPointerFn =
        void (APIENTRY *)(
            GLuint,
            GLint,
            GLenum,
            GLboolean,
            GLsizei,
            const void*
        );

    using CreateShaderFn =
        GLuint (APIENTRY *)(GLenum);
    using ShaderSourceFn =
        void (APIENTRY *)(
            GLuint,
            GLsizei,
            const GLchar* const*,
            const GLint*
        );
    using CompileShaderFn =
        void (APIENTRY *)(GLuint);
    using GetShaderivFn =
        void (APIENTRY *)(GLuint, GLenum, GLint*);
    using GetShaderInfoLogFn =
        void (APIENTRY *)(GLuint, GLsizei, GLsizei*, GLchar*);
    using DeleteShaderFn =
        void (APIENTRY *)(GLuint);

    using CreateProgramFn =
        GLuint (APIENTRY *)();
    using AttachShaderFn =
        void (APIENTRY *)(GLuint, GLuint);
    using LinkProgramFn =
        void (APIENTRY *)(GLuint);
    using GetProgramivFn =
        void (APIENTRY *)(GLuint, GLenum, GLint*);
    using GetProgramInfoLogFn =
        void (APIENTRY *)(GLuint, GLsizei, GLsizei*, GLchar*);
    using DeleteProgramFn =
        void (APIENTRY *)(GLuint);
    using UseProgramFn =
        void (APIENTRY *)(GLuint);

    using GetUniformLocationFn =
        GLint (APIENTRY *)(GLuint, const GLchar*);
    using UniformMatrix4fvFn =
        void (APIENTRY *)(GLint, GLsizei, GLboolean, const GLfloat*);
    using Uniform4fFn =
        void (APIENTRY *)(
            GLint,
            GLfloat,
            GLfloat,
            GLfloat,
            GLfloat
        );
    using Uniform1iFn =
        void (APIENTRY *)(GLint, GLint);
    using Uniform1fFn =
        void (APIENTRY *)(GLint, GLfloat);

    using GenTexturesFn =
        void (APIENTRY *)(GLsizei, GLuint*);
    using BindTextureFn =
        void (APIENTRY *)(GLenum, GLuint);
    using TexImage2DFn =
        void (APIENTRY *)(
            GLenum,
            GLint,
            GLint,
            GLsizei,
            GLsizei,
            GLint,
            GLenum,
            GLenum,
            const void*
        );
    using TexParameteriFn =
        void (APIENTRY *)(GLenum, GLenum, GLint);
    using PixelStoreiFn =
        void (APIENTRY *)(GLenum, GLint);
    using DeleteTexturesFn =
        void (APIENTRY *)(GLsizei, const GLuint*);
    using ActiveTextureFn =
        void (APIENTRY *)(GLenum);

    using DrawElementsFn =
        void (APIENTRY *)(GLenum, GLsizei, GLenum, const void*);

    using ReadPixelsFn =
        void (APIENTRY *)(
            GLint,
            GLint,
            GLsizei,
            GLsizei,
            GLenum,
            GLenum,
            void*
        );

    GetStringFn getString = nullptr;
    ViewportFn viewport = nullptr;
    ClearColorFn clearColor = nullptr;
    ClearFn clear = nullptr;
    EnableFn enable = nullptr;
    DisableFn disable = nullptr;
    DepthFuncFn depthFunc = nullptr;
    DepthMaskFn depthMask = nullptr;
    CullFaceFn cullFace = nullptr;
    FrontFaceFn frontFace = nullptr;
    BlendFuncFn blendFunc = nullptr;

    GenVertexArraysFn genVertexArrays = nullptr;
    BindVertexArrayFn bindVertexArray = nullptr;
    DeleteVertexArraysFn deleteVertexArrays = nullptr;

    GenBuffersFn genBuffers = nullptr;
    BindBufferFn bindBuffer = nullptr;
    BufferDataFn bufferData = nullptr;
    DeleteBuffersFn deleteBuffers = nullptr;

    EnableVertexAttribArrayFn enableVertexAttribArray = nullptr;
    VertexAttribPointerFn vertexAttribPointer = nullptr;

    CreateShaderFn createShader = nullptr;
    ShaderSourceFn shaderSource = nullptr;
    CompileShaderFn compileShader = nullptr;
    GetShaderivFn getShaderiv = nullptr;
    GetShaderInfoLogFn getShaderInfoLog = nullptr;
    DeleteShaderFn deleteShader = nullptr;

    CreateProgramFn createProgram = nullptr;
    AttachShaderFn attachShader = nullptr;
    LinkProgramFn linkProgram = nullptr;
    GetProgramivFn getProgramiv = nullptr;
    GetProgramInfoLogFn getProgramInfoLog = nullptr;
    DeleteProgramFn deleteProgram = nullptr;
    UseProgramFn useProgram = nullptr;

    GetUniformLocationFn getUniformLocation = nullptr;
    UniformMatrix4fvFn uniformMatrix4fv = nullptr;
    Uniform4fFn uniform4f = nullptr;
    Uniform1iFn uniform1i = nullptr;
    Uniform1fFn uniform1f = nullptr;

    GenTexturesFn genTextures = nullptr;
    BindTextureFn bindTexture = nullptr;
    TexImage2DFn texImage2D = nullptr;
    TexParameteriFn texParameteri = nullptr;
    PixelStoreiFn pixelStorei = nullptr;
    DeleteTexturesFn deleteTextures = nullptr;
    ActiveTextureFn activeTexture = nullptr;

    DrawElementsFn drawElements = nullptr;
    ReadPixelsFn readPixels = nullptr;

    void load();
};

}
