#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "assets/AssetManager.h"
#include "Player.h"
#include "model/ModelData.h"
#include "world/Region.h"

#include "host/sdl/SdlOpenGLContext.h"

#include "model/ModelSystem.h"
#include "texture/TextureSystem.h"

#include "Renderer3D.h"
#include "render/animation/AnimationPlayer.h"
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
    void movePlayer(int dx, int dy);
    void syncPlayerRenderObject();

    void syncCameraToPlayer();

    void buildPlayerIdleAnimation(
        const eld::model::ModelData& appearance
    );

    void updatePlayerAnimation(float dt);

    void setPlayerWalking(bool walking);
    void updatePlayerMovement(float dt);
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

    float playerGroundOffset_ =
        0.0f;


    eld::render::AnimationPlayer
        playerAnimation_;

    std::vector<eld::render::ModelHandle>
        playerAnimationModels_;

    std::vector<float>
        playerAnimationGroundOffsets_;

    double playerAnimationMillisecondRemainder_ =
        0.0;

    std::vector<eld::render::ModelHandle>
        playerWalkAnimationModels_;

    std::vector<float>
        playerWalkAnimationGroundOffsets_;

    bool playerWalking_ = false;
    bool playerMoving_ = false;

    eld::world::TilePosition
        playerMoveStartTile_{};

    eld::world::TilePosition
        playerMoveDestinationTile_{};

    float playerMoveElapsed_ = 0.0f;

    bool cameraLocked_ = true;

    float cameraFollowOffsetX_ = 0.0f;
    float cameraFollowOffsetY_ = 7.0f;
    float cameraFollowOffsetZ_ = 10.0f;

    eld::render::RenderScene
        scene_;

    eld::render::Renderer3D
        renderer_;
};

}
