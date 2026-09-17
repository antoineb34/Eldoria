#pragma once

#include <cstdint>
#include <deque>

#include <cstddef>
#include <optional>
#include <vector>

#include "assets/AssetManager.h"
#include "Player.h"
#include "model/ModelData.h"
#include "world/Region.h"

#include "world/World.h"
#include "world/Pathfinder.h"
#include "map/WorldStreamer.h"
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

    const eld::world::Region* regionAt(
        int worldX,
        int worldY
    ) const;

    bool canMoveWorld(
        const eld::world::TilePosition& from,
        const eld::world::TilePosition& to
    ) const;
    void spawnPlayer();
    void movePlayer(int dx, int dy);

    void walkPlayerTo(
        const eld::world::TilePosition& destination
    );

    void beginNextPlayerStep();
    void syncPlayerRenderObject();

    void syncCameraToPlayer();

    std::optional<eld::world::TilePosition>
    pickTerrainTile(
        float mouseX,
        float mouseY
    ) const;
    const eld::world::Location*
    findInteractableLocationAt(
        const eld::world::TilePosition& tile
    ) const;


    void selectTerrainTile(
        const eld::world::TilePosition& tile,
        bool interactable
    );

    void drawClickCross();
    void updateMouseSelection();

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

    eld::world::World world_;

    eld::world::Pathfinder
        pathfinder_;

    eld::runtime::map::WorldStreamer
        worldStreamer_;

    // Render-space origin for the currently displayed world.
    //
    // Runtime Region ownership belongs exclusively to World.
    eld::world::TerrainOrigin
        sceneOrigin_{};

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
    std::deque<eld::world::TilePosition> playerPath_;
    bool playerMoving_ = false;

    eld::world::TilePosition
        playerMoveStartTile_{};

    eld::world::TilePosition
        playerMoveDestinationTile_{};

    float playerMoveElapsed_ = 0.0f;

    std::optional<eld::world::TilePosition>
        selectedTile_;

    std::optional<std::size_t>
        selectedTileObjectIndex_;

    std::optional<eld::render::ModelHandle>
        selectedTileModel_;


    bool selectedTileInteractable_ =
        false;

    bool leftMouseWasDown_ = false;

    bool clickCrossActive_ = false;
    bool clickCrossRed_ = false;

    float clickCrossX_ = 0.0f;
    float clickCrossY_ = 0.0f;

    std::uint64_t clickCrossStartMs_ = 0;

    bool cameraLocked_ = true;

    float cameraDistance_ = 12.0f;
    float cameraMinDistance_ = 4.0f;
    float cameraMaxDistance_ = 30.0f;

    float cameraTargetHeight_ = 1.0f;

    bool cameraDragging_ = false;
    bool cameraOrbitInitialized_ = false;

    eld::render::RenderScene
        scene_;

    eld::render::Renderer3D
        renderer_;
};

}
