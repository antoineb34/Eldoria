#include "OpenGLApi.h"

#include <stdexcept>
#include <string>

namespace eld::render::opengl {

namespace {

template <typename Function>
Function loadRequired(
    const char* name
) {
    const SDL_FunctionPointer pointer =
        SDL_GL_GetProcAddress(name);

    if (pointer == nullptr) {
        throw std::runtime_error(
            std::string("Missing OpenGL function: ") +
            name
        );
    }

    return reinterpret_cast<Function>(pointer);
}

}

void OpenGLApi::load() {
    getString = loadRequired<GetStringFn>("glGetString");
    viewport = loadRequired<ViewportFn>("glViewport");
    clearColor = loadRequired<ClearColorFn>("glClearColor");
    clear = loadRequired<ClearFn>("glClear");
    enable = loadRequired<EnableFn>("glEnable");
    disable = loadRequired<DisableFn>("glDisable");
    depthFunc = loadRequired<DepthFuncFn>("glDepthFunc");
    depthMask = loadRequired<DepthMaskFn>("glDepthMask");
    cullFace = loadRequired<CullFaceFn>("glCullFace");
    frontFace = loadRequired<FrontFaceFn>("glFrontFace");
    blendFunc = loadRequired<BlendFuncFn>("glBlendFunc");

    genVertexArrays =
        loadRequired<GenVertexArraysFn>("glGenVertexArrays");
    bindVertexArray =
        loadRequired<BindVertexArrayFn>("glBindVertexArray");
    deleteVertexArrays =
        loadRequired<DeleteVertexArraysFn>("glDeleteVertexArrays");

    genBuffers =
        loadRequired<GenBuffersFn>("glGenBuffers");
    bindBuffer =
        loadRequired<BindBufferFn>("glBindBuffer");
    bufferData =
        loadRequired<BufferDataFn>("glBufferData");
    deleteBuffers =
        loadRequired<DeleteBuffersFn>("glDeleteBuffers");

    enableVertexAttribArray =
        loadRequired<EnableVertexAttribArrayFn>(
            "glEnableVertexAttribArray"
        );
    vertexAttribPointer =
        loadRequired<VertexAttribPointerFn>(
            "glVertexAttribPointer"
        );

    createShader =
        loadRequired<CreateShaderFn>("glCreateShader");
    shaderSource =
        loadRequired<ShaderSourceFn>("glShaderSource");
    compileShader =
        loadRequired<CompileShaderFn>("glCompileShader");
    getShaderiv =
        loadRequired<GetShaderivFn>("glGetShaderiv");
    getShaderInfoLog =
        loadRequired<GetShaderInfoLogFn>("glGetShaderInfoLog");
    deleteShader =
        loadRequired<DeleteShaderFn>("glDeleteShader");

    createProgram =
        loadRequired<CreateProgramFn>("glCreateProgram");
    attachShader =
        loadRequired<AttachShaderFn>("glAttachShader");
    linkProgram =
        loadRequired<LinkProgramFn>("glLinkProgram");
    getProgramiv =
        loadRequired<GetProgramivFn>("glGetProgramiv");
    getProgramInfoLog =
        loadRequired<GetProgramInfoLogFn>("glGetProgramInfoLog");
    deleteProgram =
        loadRequired<DeleteProgramFn>("glDeleteProgram");
    useProgram =
        loadRequired<UseProgramFn>("glUseProgram");

    getUniformLocation =
        loadRequired<GetUniformLocationFn>("glGetUniformLocation");
    uniformMatrix4fv =
        loadRequired<UniformMatrix4fvFn>("glUniformMatrix4fv");
    uniform4f =
        loadRequired<Uniform4fFn>("glUniform4f");
    uniform1i =
        loadRequired<Uniform1iFn>("glUniform1i");
    uniform1f =
        loadRequired<Uniform1fFn>("glUniform1f");

    genTextures =
        loadRequired<GenTexturesFn>("glGenTextures");
    bindTexture =
        loadRequired<BindTextureFn>("glBindTexture");
    texImage2D =
        loadRequired<TexImage2DFn>("glTexImage2D");
    texParameteri =
        loadRequired<TexParameteriFn>("glTexParameteri");
    pixelStorei =
        loadRequired<PixelStoreiFn>("glPixelStorei");
    deleteTextures =
        loadRequired<DeleteTexturesFn>("glDeleteTextures");
    activeTexture =
        loadRequired<ActiveTextureFn>("glActiveTexture");

    drawElements =
        loadRequired<DrawElementsFn>("glDrawElements");

    readPixels =
        loadRequired<ReadPixelsFn>("glReadPixels");
}

}
