#pragma once

#include <cstddef>
#include <optional>

#include "assets/AssetManager.h"
#include "Player.h"
#include "world/Region.h"

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
    void buildWorld();
    void spawnPlayer();
    void processEvents(bool& running);
    void update(float dt);
    void render();

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

    std::optional<eld::world::Region>
        region_;

    Player
        player_;

    std::optional<std::size_t>
        playerObjectIndex_;

    eld::render::RenderScene
        scene_;

    eld::render::Renderer3D
        renderer_;
};

}
