#pragma once

#include "assets/AssetManager.h"

#include "host/sdl/SdlOpenGLContext.h"

#include "model/ModelSystem.h"
#include "texture/TextureSystem.h"

#include "Renderer3D.h"
#include "render/model/ModelManager.h"
#include "render/scene/RenderScene.h"
#include "render/texture/TextureManager.h"

namespace eld::client {

class Client {
public:
    Client();

    int run();

private:
    // Order matters:
    // systems below hold references to objects declared above them.

    eld::host::SdlOpenGLContext
        sdl_;

    eld::asset::AssetManager
        assets_;

    eld::render::TextureManager
        textureManager_;

    eld::render::ModelManager
        modelManager_;

    eld::graphics::TextureSystem
        textureSystem_;

    eld::graphics::ModelSystem
        modelSystem_;

    eld::render::RenderScene
        scene_;

    eld::render::Renderer3D
        renderer_;
};

}
