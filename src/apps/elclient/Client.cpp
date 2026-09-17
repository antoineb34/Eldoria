#include "Client.h"
#include "render/camera/Projection.h"
#include "render/animation/ModelAnimator.h"
#include "PlayerAppearanceBuilder.h"

#include "map/LocationBuilder.h"
#include "map/LocationBatchBuilder.h"
#include "model/ModelSystem.h"
#include <array>
#include <vector>
#include <iostream>
#include <cstdint>
#include <SDL3/SDL.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

#include "assets/AssetManager.h"

#include "map/TerrainBuilder.h"
#include "map/RegionBuilder.h"
#include "texture/TextureSystem.h"

#include "host/sdl/SdlOpenGLContext.h"

#include "Renderer3D.h"
#include "render/model/ModelManager.h"
#include "render/model/ModelResource.h"
#include "render/scene/RenderScene.h"
#include "render/texture/TextureManager.h"

namespace eld::client {

namespace {

eld::render::ModelResource
buildPlayerMarkerModel()
{
    eld::render::ModelResource model;

    eld::render::RenderMaterial material;
    material.alphaMode =
        eld::render::AlphaMode::Opaque;
    material.doubleSided = true;
    material.unlit = true;

    model.materials.push_back(
        material
    );


    eld::render::RenderMesh mesh;

    constexpr float halfWidth =
        0.22f;

    constexpr float height =
        1.4f;

    const eld::math::Vec4 color{
        1.0f,
        0.15f,
        0.75f,
        1.0f
    };


    const auto makeVertex =
        [&](float x, float y, float z)
    {
        eld::render::RenderVertex vertex;

        vertex.position = {
            x,
            y,
            z
        };

        vertex.normal = {
            0.0f,
            1.0f,
            0.0f
        };

        vertex.color =
            color;

        return vertex;
    };


    mesh.vertices = {
        makeVertex(-halfWidth, 0.0f,   -halfWidth),
        makeVertex( halfWidth, 0.0f,   -halfWidth),
        makeVertex( halfWidth, 0.0f,    halfWidth),
        makeVertex(-halfWidth, 0.0f,    halfWidth),

        makeVertex(-halfWidth, height, -halfWidth),
        makeVertex( halfWidth, height, -halfWidth),
        makeVertex( halfWidth, height,  halfWidth),
        makeVertex(-halfWidth, height,  halfWidth)
    };


    mesh.indices = {
        0, 2, 1,
        0, 3, 2,

        4, 5, 6,
        4, 6, 7,

        3, 6, 2,
        3, 7, 6,

        0, 1, 5,
        0, 5, 4,

        0, 4, 7,
        0, 7, 3,

        1, 2, 6,
        1, 6, 5
    };


    eld::render::RenderMeshSection section;

    section.firstIndex = 0;

    section.indexCount =
        static_cast<std::uint32_t>(
            mesh.indices.size()
        );

    section.materialIndex = 0;

    mesh.sections.push_back(
        section
    );

    model.meshes.push_back(
        std::move(mesh)
    );

    return model;
}


constexpr float PlayerModelScale =
    1.0f / 128.0f;


float playerGroundOffset(
    const eld::model::ModelData& model
)
{
    if (model.vertices.empty()) {
        return 0.0f;
    }


    // ModelBuilder converts source coordinates:
    //
    //     source Y -> render -Y
    //
    // and the player is scaled by 1/128.
    //
    // Find the lowest point of the rendered model before
    // translation, then move the model upward by exactly
    // enough to put that point at terrain height.

    float lowestRenderedY =
        -model.vertices.front().y *
        PlayerModelScale;


    for (
        const auto& vertex :
        model.vertices
    ) {
        const float renderedY =
            -vertex.y *
            PlayerModelScale;

        lowestRenderedY =
            std::min(
                lowestRenderedY,
                renderedY
            );
    }


    return -lowestRenderedY;
}


float playerSupportHeight(
    const eld::world::Terrain& terrain,
    const eld::world::TilePosition& tile,
    const eld::world::TileLocalPosition& local,
    int sourcePlane
)
{
    // Approximate where the player's feet/body footprint
    // touches the terrain.
    //
    // One tile = 1 world unit.
    constexpr float radius =
        0.18f;


    const float worldX =
        static_cast<float>(tile.x) +
        local.x;

    const float worldY =
        static_cast<float>(tile.y) +
        local.y;


    constexpr std::array<
        std::array<float, 2>,
        5
    > offsets{{
        { 0.0f,    0.0f    },

        {-radius, -radius   },
        { radius, -radius   },
        { radius,  radius   },
        {-radius,  radius   }
    }};


    float highest =
        -std::numeric_limits<float>::infinity();

    bool found =
        false;


    for (
        const auto& offset :
        offsets
    ) {
        const float sampleX =
            worldX +
            offset[0];

        const float sampleY =
            worldY +
            offset[1];


        const int tileX =
            static_cast<int>(
                std::floor(sampleX)
            );

        const int tileY =
            static_cast<int>(
                std::floor(sampleY)
            );


        const eld::world::TerrainLayerPosition
            position{
                tileX,
                tileY,
                sourcePlane
            };


        if (
            !terrain.contains(
                position
            )
        ) {
            continue;
        }


        const eld::world::TileLocalPosition
            sampleLocal{
                sampleX -
                    static_cast<float>(
                        tileX
                    ),

                sampleY -
                    static_cast<float>(
                        tileY
                    )
            };


        highest =
            std::max(
                highest,
                terrain.heightAt(
                    position,
                    sampleLocal
                )
            );

        found =
            true;
    }


    if (found) {
        return highest;
    }


    // Fallback; normally unreachable while the player
    // remains inside the loaded terrain.
    const eld::world::TerrainLayerPosition
        center{
            tile.x,
            tile.y,
            sourcePlane
        };

    return terrain.heightAt(
        center,
        local
    );
}



}


Client::Client()
    : sdl_(
          "Eldoria",
          1000,
          700
      ),
      textureSystem_(
          assets_.textures,
          textureManager_
      ),
      modelSystem_(
          assets_.models,
          textureSystem_,
          modelManager_
      ),
      renderer_(
          modelManager_,
          textureManager_
      )
{
}


void Client::buildWorld()
{
    scene_.objects.clear();

// ========================================================
// === SINGLE REGION ===
// ========================================================

constexpr std::uint16_t regionId =
    12850;



eld::runtime::map::RegionBuilder
    regionBuilder;


region_.emplace(
    regionBuilder.build(
        regionId,
        assets_.maps,
        assets_.locations
    )
);

const auto& worldRegion =
    *region_;


eld::graphics::TerrainBuilder
    terrainBuilder;

auto terrainModel =
    terrainBuilder.build(
        worldRegion.terrain,
        0,
        assets_.floors,
        textureSystem_
    );

const eld::render::ModelHandle
    terrainHandle =
        modelManager_.create(
            std::move(
                terrainModel
            )
        );

scene_.camera.position = {
    31.5f,
    25.0f,
    85.0f
};

scene_.camera.rotation = {
    -0.45f,
    0.0f,
    0.0f
};

scene_.camera.viewportWidth =
    1000;

scene_.camera.viewportHeight =
    700;

scene_.objects.push_back({
    terrainHandle,
    {},
    true
});


// ========================================================
// === REGION LOCATIONS ===
// ========================================================

eld::graphics::LocationBuilder
    locationBuilder;


auto locationBuild =
    locationBuilder.build(
        worldRegion,
        0,
        assets_.locations,
        assets_.models,
        modelSystem_
    );


eld::graphics::LocationBatchBuilder
    locationBatchBuilder;

auto locationBatchBuild =
    locationBatchBuilder.build(
        locationBuild.objects,
        modelManager_
    );


for (
    auto& batch :
    locationBatchBuild.batches
) {
    const auto handle =
        modelManager_.create(
            std::move(batch)
        );

    scene_.objects.push_back({
        handle,
        {},
        true
    });
}


scene_.objects.insert(
    scene_.objects.end(),
    locationBatchBuild
        .passthroughObjects.begin(),
    locationBatchBuild
        .passthroughObjects.end()
);


spawnPlayer();


std::cout
    << "\n=== LOCATION BUILD ===\n"
    << "locations         = "
    << locationBuild.locations
    << "\n"
    << "render objects    = "
    << locationBuild.objects.size()
    << "\n"
    << "model variants    = "
    << locationBuild.modelVariants
    << "\n"
    << "camera-dependent  = "
    << locationBuild.cameraDependent.size()
    << "\n"
    << "missing defs      = "
    << locationBuild.missingDefinitions
    << "\n"
    << "missing models    = "
    << locationBuild.missingModels
    << "\n\n";


std::cout
    << "=== LOCATION BATCH BUILD ===\n"
    << "source objects     = "
    << locationBatchBuild.sourceObjects
    << "\n"
    << "batched objects    = "
    << locationBatchBuild.batchedObjects
    << "\n"
    << "passthrough        = "
    << locationBatchBuild
        .passthroughObjects.size()
    << "\n"
    << "chunk models       = "
    << locationBatchBuild.batches.size()
    << "\n"
    << "batch sections     = "
    << locationBatchBuild.batchSections
    << "\n\n";


}


void Client::processEvents(
    bool& running
)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {



        if (
            event.type ==
            SDL_EVENT_QUIT
        ) {
            running = false;
        }

        if (
            event.key.scancode ==
                SDL_SCANCODE_C
        ) {
            cameraLocked_ =
                !cameraLocked_;

            if (cameraLocked_) {

                syncCameraToPlayer();

                std::cout
                    << "camera = locked\\n";
            }
            else {
                std::cout
                    << "camera = free\\n";
            }
        }

        if (
            event.type ==
                SDL_EVENT_KEY_DOWN &&
            event.key.scancode ==
                SDL_SCANCODE_F &&
            !event.key.repeat
        ) {
            renderer_.toggleWireframe();
        }
    
        if (
            event.type ==
                SDL_EVENT_KEY_DOWN &&
            !event.key.repeat
        ) {
            switch (
                event.key.scancode
            ) {
            case SDL_SCANCODE_I:
                movePlayer(0, 1);
                break;

            case SDL_SCANCODE_K:
                movePlayer(0, -1);
                break;

            case SDL_SCANCODE_J:
                movePlayer(-1, 0);
                break;

            case SDL_SCANCODE_L:
                movePlayer(1, 0);
                break;

            default:
                break;
            }
        }

}
}


void Client::render()
{
    renderer_.render(scene_);

    SDL_GL_SwapWindow(
        sdl_.window()
    );
}



void Client::buildPlayerIdleAnimation(
    const eld::model::ModelData& appearance
)
{
    playerAnimation_.clear();

    playerAnimationModels_.clear();
    playerAnimationGroundOffsets_.clear();

    playerWalkAnimationModels_.clear();
    playerWalkAnimationGroundOffsets_.clear();

    playerAnimationMillisecondRemainder_ =
        0.0;


    const auto buildSequence =
        [&](
            std::uint16_t sequenceId,
            std::vector<
                eld::render::ModelHandle
            >& models,
            std::vector<float>& groundOffsets,
            const char* name
        )
    {
        if (
            !assets_.sequences.contains(
                sequenceId
            )
        ) {
            std::cout
                << name
                << " sequence "
                << sequenceId
                << " missing\n";

            return false;
        }


        const auto sequence =
            assets_.sequences.find(
                sequenceId
            );


        if (
            !sequence.has_value() ||
            sequence->resolvedFrames.empty()
        ) {
            std::cout
                << name
                << " sequence "
                << sequenceId
                << " has no frames\n";

            return false;
        }


        eld::render::ModelAnimator animator;


        models.reserve(
            sequence->resolvedFrames.size()
        );

        groundOffsets.reserve(
            sequence->resolvedFrames.size()
        );


        for (
            const auto& sequenceFrame :
            sequence->resolvedFrames
        ) {
            const auto frame =
                sequenceFrame.primary.resource();


            auto animated =
                animator.apply(
                    appearance,
                    frame.frame,
                    frame.skeleton
                );


            groundOffsets.push_back(
                playerGroundOffset(
                    animated.mesh
                )
            );


            models.push_back(
                modelSystem_.create(
                    animated.mesh
                )
            );
        }


        std::cout
            << name
            << " sequence      = "
            << sequenceId
            << "\n"
            << name
            << " frames        = "
            << models.size()
            << "\n";


        return true;
    };


    constexpr std::uint16_t idleSequenceId =
        808;

    constexpr std::uint16_t walkSequenceId =
        819;


    const bool idleReady =
        buildSequence(
            idleSequenceId,
            playerAnimationModels_,
            playerAnimationGroundOffsets_,
            "idle"
        );


    buildSequence(
        walkSequenceId,
        playerWalkAnimationModels_,
        playerWalkAnimationGroundOffsets_,
        "walk"
    );


    playerWalking_ = false;


    if (idleReady) {
        playerAnimation_.setSequence(
            assets_.sequences.resource(
                idleSequenceId
            )
        );

        playerAnimation_.setLooping(
            true
        );

        playerAnimation_.restart();
        playerAnimation_.play();
    }
}



void Client::updatePlayerAnimation(
    float dt
)
{
    if (!playerObjectIndex_.has_value()) {
        return;
    }


    const auto& models =
        playerWalking_
            ? playerWalkAnimationModels_
            : playerAnimationModels_;


    const auto& groundOffsets =
        playerWalking_
            ? playerWalkAnimationGroundOffsets_
            : playerAnimationGroundOffsets_;


    if (models.empty()) {
        return;
    }


    playerAnimationMillisecondRemainder_ +=
        static_cast<double>(dt) *
        1000.0;


    const auto milliseconds =
        static_cast<std::uint64_t>(
            playerAnimationMillisecondRemainder_
        );


    if (milliseconds == 0) {
        return;
    }


    playerAnimationMillisecondRemainder_ -=
        static_cast<double>(
            milliseconds
        );


    if (
        !playerAnimation_.update(
            milliseconds
        )
    ) {
        return;
    }


    const std::size_t frameIndex =
        playerAnimation_.frameIndex();


    if (
        frameIndex >= models.size() ||
        frameIndex >= groundOffsets.size()
    ) {
        return;
    }


    if (
        *playerObjectIndex_ >=
        scene_.objects.size()
    ) {
        return;
    }


    scene_.objects[
        *playerObjectIndex_
    ].model =
        models[
            frameIndex
        ];


    playerGroundOffset_ =
        groundOffsets[
            frameIndex
        ];


    syncPlayerRenderObject();
}


void Client::setPlayerWalking(
    bool walking
)
{
    if (walking == playerWalking_) {
        return;
    }


    constexpr std::uint16_t idleSequenceId =
        808;

    constexpr std::uint16_t walkSequenceId =
        819;


    const auto& models =
        walking
            ? playerWalkAnimationModels_
            : playerAnimationModels_;


    const auto& groundOffsets =
        walking
            ? playerWalkAnimationGroundOffsets_
            : playerAnimationGroundOffsets_;


    if (models.empty()) {
        return;
    }


    const std::uint16_t sequenceId =
        walking
            ? walkSequenceId
            : idleSequenceId;


    if (
        !assets_.sequences.contains(
            sequenceId
        )
    ) {
        return;
    }


    playerWalking_ =
        walking;


    playerAnimation_.setSequence(
        assets_.sequences.resource(
            sequenceId
        )
    );


    playerAnimation_.setLooping(
        true
    );

    playerAnimation_.restart();
    playerAnimation_.play();


    playerAnimationMillisecondRemainder_ =
        0.0;


    if (
        playerObjectIndex_.has_value() &&
        *playerObjectIndex_ <
            scene_.objects.size()
    ) {
        scene_.objects[
            *playerObjectIndex_
        ].model =
            models.front();


        playerGroundOffset_ =
            groundOffsets.front();


        syncPlayerRenderObject();
    }
}


void Client::updatePlayerMovement(
    float dt
)
{
    if (
        !playerMoving_ ||
        !region_.has_value()
    ) {
        return;
    }


    constexpr float moveDuration =
        0.6f;


    playerMoveElapsed_ +=
        dt;


    const float progress =
        std::clamp(
            playerMoveElapsed_ /
                moveDuration,
            0.0f,
            1.0f
        );


    const float startX =
        static_cast<float>(
            playerMoveStartTile_.x
        ) +
        0.5f;

    const float startY =
        static_cast<float>(
            playerMoveStartTile_.y
        ) +
        0.5f;


    const float destinationX =
        static_cast<float>(
            playerMoveDestinationTile_.x
        ) +
        0.5f;

    const float destinationY =
        static_cast<float>(
            playerMoveDestinationTile_.y
        ) +
        0.5f;


    const float previousWorldX =
        static_cast<float>(
            player_.tile.x
        ) +
        player_.local.x;

    const float previousWorldY =
        static_cast<float>(
            player_.tile.y
        ) +
        player_.local.y;


    const float worldX =
        std::lerp(
            startX,
            destinationX,
            progress
        );

    const float worldY =
        std::lerp(
            startY,
            destinationY,
            progress
        );


    const float playerDeltaX =
        worldX -
        previousWorldX;

    const float playerDeltaY =
        worldY -
        previousWorldY;


    // Keep the current camera offset/angle,
    // but translate it with the player.
    //
    // World Y maps to negative render Z.
    scene_.camera.position.x +=
        playerDeltaX;

    scene_.camera.position.z -=
        playerDeltaY;


    const int tileX =
        static_cast<int>(
            std::floor(worldX)
        );

    const int tileY =
        static_cast<int>(
            std::floor(worldY)
        );


    const eld::world::TerrainLayerPosition
        terrainPosition{
            tileX,
            tileY,
            0
        };


    const auto& terrain =
        region_->terrain;


    if (
        terrain.contains(
            terrainPosition
        )
    ) {
        const auto& tile =
            terrain.tile(
                terrainPosition
            );


        player_.tile = {
            tileX,
            tileY,
            tile.scenePlane
        };


        player_.local = {
            worldX -
                static_cast<float>(
                    tileX
                ),

            worldY -
                static_cast<float>(
                    tileY
                )
        };


        syncPlayerRenderObject();
    }


    if (progress < 1.0f) {
        return;
    }


    player_.tile =
        playerMoveDestinationTile_;


    player_.local = {
        0.5f,
        0.5f
    };


    playerMoving_ =
        false;

    playerMoveElapsed_ =
        0.0f;


    syncPlayerRenderObject();

    setPlayerWalking(
        false
    );


    std::cout
        << "player tile = "
        << player_.tile.x
        << ", "
        << player_.tile.y
        << ", plane "
        << player_.tile.plane
        << "\n";
}




std::optional<eld::world::TilePosition>
Client::pickTerrainTile(
    float mouseX,
    float mouseY
) const
{
    if (
        !region_.has_value() ||
        scene_.camera.viewportWidth == 0 ||
        scene_.camera.viewportHeight == 0
    ) {
        return std::nullopt;
    }


    const auto& terrain =
        region_->terrain;


    if (
        terrain.width() == 0 ||
        terrain.height() == 0
    ) {
        return std::nullopt;
    }


    // Convert SDL mouse coordinates into the screen-space
    // convention used by projectPoint().
    const float projectedMouseX =
        static_cast<float>(
            scene_.camera.viewportWidth
        ) -
        mouseX;

    const float projectedMouseY =
        static_cast<float>(
            scene_.camera.viewportHeight
        ) -
        mouseY;


    const auto view =
        eld::render::buildViewMatrix(
            scene_.camera
        );


    const auto projection =
        eld::render::buildProjectionMatrix(
            scene_.camera
        );


    const auto pointInTriangle =
        [](
            float px,
            float py,
            const eld::render::ScreenPoint& a,
            const eld::render::ScreenPoint& b,
            const eld::render::ScreenPoint& c
        )
        {
            const float denominator =
                (
                    b.y - c.y
                ) *
                (
                    a.x - c.x
                ) +
                (
                    c.x - b.x
                ) *
                (
                    a.y - c.y
                );


            if (
                std::abs(
                    denominator
                ) <
                0.00001f
            ) {
                return false;
            }


            const float alpha =
                (
                    (
                        b.y - c.y
                    ) *
                    (
                        px - c.x
                    ) +
                    (
                        c.x - b.x
                    ) *
                    (
                        py - c.y
                    )
                ) /
                denominator;


            const float beta =
                (
                    (
                        c.y - a.y
                    ) *
                    (
                        px - c.x
                    ) +
                    (
                        a.x - c.x
                    ) *
                    (
                        py - c.y
                    )
                ) /
                denominator;


            const float gamma =
                1.0f -
                alpha -
                beta;


            constexpr float epsilon =
                -0.0001f;


            return
                alpha >= epsilon &&
                beta >= epsilon &&
                gamma >= epsilon;
        };


    std::optional<
        eld::world::TilePosition
    > result;


    float bestDepth =
        std::numeric_limits<float>::infinity();


    for (
        std::size_t localY = 0;
        localY < terrain.height();
        ++localY
    ) {
        for (
            std::size_t localX = 0;
            localX < terrain.width();
            ++localX
        ) {
            const auto position =
                terrain.layerPosition(
                    0,
                    localX,
                    localY
                );


            const auto heights =
                terrain.cornerHeights(
                    position
                );


            const float x0 =
                static_cast<float>(
                    localX
                );

            const float x1 =
                x0 + 1.0f;


            const float z0 =
                -static_cast<float>(
                    localY
                );

            const float z1 =
                -static_cast<float>(
                    localY + 1
                );


            const auto sw =
                eld::render::projectPoint(
                    {
                        x0,
                        heights.southwest,
                        z0
                    },
                    view,
                    projection,
                    scene_.camera
                );


            const auto se =
                eld::render::projectPoint(
                    {
                        x1,
                        heights.southeast,
                        z0
                    },
                    view,
                    projection,
                    scene_.camera
                );


            const auto ne =
                eld::render::projectPoint(
                    {
                        x1,
                        heights.northeast,
                        z1
                    },
                    view,
                    projection,
                    scene_.camera
                );


            const auto nw =
                eld::render::projectPoint(
                    {
                        x0,
                        heights.northwest,
                        z1
                    },
                    view,
                    projection,
                    scene_.camera
                );


            const auto consider =
                [&](
                    const eld::render::ScreenPoint& a,
                    const eld::render::ScreenPoint& b,
                    const eld::render::ScreenPoint& c
                )
                {
                    if (
                        !std::isfinite(a.x) ||
                        !std::isfinite(a.y) ||
                        !std::isfinite(b.x) ||
                        !std::isfinite(b.y) ||
                        !std::isfinite(c.x) ||
                        !std::isfinite(c.y)
                    ) {
                        return;
                    }


                    // Visible geometry in Eldoria is in
                    // negative view-space Z.
                    //
                    // Reject triangles behind the camera or
                    // crossing the near plane before doing the
                    // screen-space containment test.
                    if (
                        a.depth > -scene_.camera.nearPlane ||
                        b.depth > -scene_.camera.nearPlane ||
                        c.depth > -scene_.camera.nearPlane
                    ) {
                        return;
                    }


                    if (
                        a.depth < -scene_.camera.farPlane ||
                        b.depth < -scene_.camera.farPlane ||
                        c.depth < -scene_.camera.farPlane
                    ) {
                        return;
                    }


                    const float triangleDepth =
                        -(
                            a.depth +
                            b.depth +
                            c.depth
                        ) /
                        3.0f;


                    if (
                        !pointInTriangle(
                            projectedMouseX,
                            projectedMouseY,
                            a,
                            b,
                            c
                        )
                    ) {
                        return;
                    }


                    if (
                        triangleDepth >=
                        bestDepth
                    ) {
                        return;
                    }


                    bestDepth =
                        triangleDepth;


                    const auto& tile =
                        terrain.tile(
                            position
                        );


                    result =
                        eld::world::TilePosition{
                            position.x,
                            position.y,
                            tile.scenePlane
                        };
                };


            // Tile quad:
            //
            // NW ------ NE
            // |       / |
            // |     /   |
            // |   /     |
            // SW ------ SE
            //
            consider(
                sw,
                se,
                ne
            );


            consider(
                sw,
                ne,
                nw
            );
        }
    }


    return result;
}


void Client::selectTerrainTile(
    const eld::world::TilePosition& tile
)
{
    if (!region_.has_value()) {
        return;
    }


    auto& terrain =
        region_->terrain;


    const eld::world::TerrainLayerPosition
        position{
            tile.x,
            tile.y,
            0
        };


    if (!terrain.contains(position)) {
        return;
    }


    const auto heights =
        terrain.cornerHeights(
            position
        );


    constexpr float highlightLift =
        0.08f;


    eld::render::ModelResource resource;


    eld::render::RenderMaterial material;

    // Debug selection color: deliberately obnoxious so
    // there is zero ambiguity about which tile was picked.
    material.baseColor = {
        1.0f,
        1.0f,
        0.0f,
        1.0f
    };

    material.alphaMode =
        eld::render::AlphaMode::Opaque;

    material.doubleSided =
        true;

    material.unlit =
        true;


    resource.materials.push_back(
        material
    );


    eld::render::RenderMesh mesh;


    const auto addVertex =
        [&](
            float x,
            float y,
            float z
        )
        {
            eld::render::RenderVertex vertex{};

            vertex.position = {
                x,
                y + highlightLift,
                z
            };

            vertex.normal = {
                0.0f,
                1.0f,
                0.0f
            };

            vertex.color = {
                1.0f,
                1.0f,
                0.0f,
                1.0f
            };

            mesh.vertices.push_back(
                vertex
            );
        };


    addVertex(
        0.0f,
        heights.southwest,
        0.0f
    );

    addVertex(
        1.0f,
        heights.southeast,
        0.0f
    );

    addVertex(
        1.0f,
        heights.northeast,
        -1.0f
    );

    addVertex(
        0.0f,
        heights.northwest,
        -1.0f
    );


    mesh.indices = {
        0, 1, 2,
        0, 2, 3
    };


    eld::render::RenderMeshSection section;

    section.firstIndex = 0;
    section.indexCount = 6;
    section.materialIndex = 0;
    section.depthBias = -0.02f;


    mesh.sections.push_back(
        section
    );


    resource.meshes.push_back(
        mesh
    );


    if (
        selectedTileModel_.has_value() &&
        modelManager_.isValid(
            *selectedTileModel_
        )
    ) {
        modelManager_.destroy(
            *selectedTileModel_
        );
    }


    selectedTileModel_ =
        modelManager_.create(
            resource
        );


    eld::render::RenderObject object;

    object.model =
        *selectedTileModel_;


    const auto& origin =
        terrain.origin();


    object.transform.position = {
        static_cast<float>(
            tile.x -
            origin.x
        ),

        0.0f,

        -static_cast<float>(
            tile.y -
            origin.y
        )
    };


    if (
        selectedTileObjectIndex_.has_value() &&
        *selectedTileObjectIndex_ <
            scene_.objects.size()
    ) {
        scene_.objects[
            *selectedTileObjectIndex_
        ] = object;
    }
    else {
        selectedTileObjectIndex_ =
            scene_.objects.size();

        scene_.objects.push_back(
            object
        );
    }


    selectedTile_ =
        tile;


    std::cout
        << "clicked tile = "
        << tile.x
        << ", "
        << tile.y
        << ", plane "
        << tile.plane
        << "\n";
}


void Client::updateMouseSelection()
{
    float mouseX = 0.0f;
    float mouseY = 0.0f;


    const SDL_MouseButtonFlags buttons =
        SDL_GetMouseState(
            &mouseX,
            &mouseY
        );


    const bool leftMouseDown =
        (
            buttons &
            SDL_BUTTON_MASK(
                SDL_BUTTON_LEFT
            )
        ) != 0;


    if (
        leftMouseDown &&
        !leftMouseWasDown_
    ) {
        // SDL_GetMouseState already gives us coordinates
        // relative to this window. Do not apply another
        // window -> pixel scaling here.
        std::cout
            << "mouse click = "
            << mouseX
            << ", "
            << mouseY
            << "\n";


        const auto pickedTile =
            pickTerrainTile(
                mouseX,
                mouseY
            );


        if (pickedTile.has_value()) {
            selectTerrainTile(
                *pickedTile
            );
        }
        else {
            std::cout
                << "clicked tile = none\n";
        }
    }


    leftMouseWasDown_ =
        leftMouseDown;
}


void Client::syncCameraToPlayer()
{
    if (
        !cameraLocked_ ||
        !playerObjectIndex_.has_value()
    ) {
        return;
    }

    if (
        *playerObjectIndex_ >=
        scene_.objects.size()
    ) {
        return;
    }

    const auto& playerPosition =
        scene_.objects[
            *playerObjectIndex_
        ].transform.position;

    scene_.camera.position = {
        playerPosition.x +
            cameraFollowOffsetX_,

        playerPosition.y +
            cameraFollowOffsetY_,

        playerPosition.z +
            cameraFollowOffsetZ_
    };
}


void Client::syncPlayerRenderObject()
{
    if (
        !region_.has_value() ||
        !playerObjectIndex_.has_value()
    ) {
        return;
    }

    if (
        *playerObjectIndex_ >=
        scene_.objects.size()
    ) {
        return;
    }


    auto& terrain =
        region_->terrain;


    constexpr int sourcePlane =
        0;


    const eld::world::TerrainLayerPosition
        terrainPosition{
            player_.tile.x,
            player_.tile.y,
            sourcePlane
        };


    if (
        !terrain.contains(
            terrainPosition
        )
    ) {
        return;
    }


    const float groundHeight =
        playerSupportHeight(
            terrain,
            player_.tile,
            player_.local,
            sourcePlane
        );


    const auto& origin =
        terrain.origin();


    auto& object =
        scene_.objects.at(
            *playerObjectIndex_
        );


    constexpr float quarterTurn =
        1.57079632679f;

    constexpr float halfTurn =
        3.14159265359f;

    switch (player_.facing) {
    case FacingDirection::North:
        object.transform.rotation.y =
            halfTurn;
        break;

    case FacingDirection::East:
        object.transform.rotation.y =
            halfTurn - quarterTurn;
        break;

    case FacingDirection::South:
        object.transform.rotation.y =
            0.0f;
        break;

    case FacingDirection::West:
        object.transform.rotation.y =
            halfTurn + quarterTurn;
        break;
    }


    object.transform.position = {
        static_cast<float>(
            player_.tile.x -
            origin.x
        ) + player_.local.x,

        groundHeight +
            playerGroundOffset_,

        -(
            static_cast<float>(
                player_.tile.y -
                origin.y
            ) + player_.local.y
        )
    };
}


void Client::movePlayer(
    int dx,
    int dy
)
{
    if (
        !region_.has_value() ||
        playerMoving_
    ) {
        return;
    }


    if (
        dx == 0 &&
        dy == 0
    ) {
        return;
    }


    const int destinationX =
        player_.tile.x +
        dx;

    const int destinationY =
        player_.tile.y +
        dy;


    const eld::world::TerrainLayerPosition
        destination{
            destinationX,
            destinationY,
            0
        };


    const auto& terrain =
        region_->terrain;


    if (
        !terrain.contains(
            destination
        )
    ) {
        return;
    }


    const auto& destinationTile =
        terrain.tile(
            destination
        );


    if (dx > 0) {
        player_.facing =
            FacingDirection::East;
    }
    else if (dx < 0) {
        player_.facing =
            FacingDirection::West;
    }
    else if (dy > 0) {
        player_.facing =
            FacingDirection::North;
    }
    else {
        player_.facing =
            FacingDirection::South;
    }


    playerMoveStartTile_ =
        player_.tile;


    playerMoveDestinationTile_ = {
        destinationX,
        destinationY,
        destinationTile.scenePlane
    };


    playerMoveElapsed_ =
        0.0f;

    playerMoving_ =
        true;


    setPlayerWalking(
        true
    );


    // Apply the new facing immediately,
    // before the first movement frame.
    syncPlayerRenderObject();
}



void Client::spawnPlayer()
{
    if (!region_.has_value()) {
        return;
    }

    auto& terrain =
        region_->terrain;


    // Middle-ish of the loaded 64x64 region.
    constexpr std::size_t localX =
        32;

    constexpr std::size_t localY =
        32;

    constexpr std::size_t sourcePlane =
        0;


    const auto terrainPosition =
        terrain.layerPosition(
            sourcePlane,
            localX,
            localY
        );


    const auto& tile =
        terrain.tile(
            terrainPosition
        );


    player_.tile = {
        terrainPosition.x,
        terrainPosition.y,
        static_cast<int>(
            tile.scenePlane
        )
    };

    player_.local = {
        0.5f,
        0.5f
    };


    const float groundHeight =
        playerSupportHeight(
            terrain,
            player_.tile,
            player_.local,
            sourcePlane
        );


    const auto& origin =
        terrain.origin();


    PlayerAppearanceBuilder
        appearanceBuilder;

    const auto appearance =
        appearanceBuilder.buildDefaultMale(
            assets_.identityKits,
            assets_.models
        );


    eld::render::ModelHandle
        playerModel;


    bool usingDebugMarker =
        false;


    if (appearance.has_value()) {
        playerGroundOffset_ =
            playerGroundOffset(
                *appearance
            );

        buildPlayerIdleAnimation(
            *appearance
        );

        if (!playerAnimationModels_.empty()) {
            playerModel =
                playerAnimationModels_.front();

            playerGroundOffset_ =
                playerAnimationGroundOffsets_.front();
        }
        else {
            playerModel =
                modelSystem_.create(
                    *appearance
                );
        }
    } else {
        usingDebugMarker =
            true;

        playerGroundOffset_ =
            0.0f;

        playerModel =
            modelManager_.create(
                buildPlayerMarkerModel()
            );
    }


    eld::render::RenderObject object;

    object.model =
        playerModel;

    if (!usingDebugMarker) {
        object.transform.scale = {
            PlayerModelScale,
            PlayerModelScale,
            -PlayerModelScale
        };
    }

    object.transform.position = {
        static_cast<float>(
            player_.tile.x -
            origin.x
        ) + player_.local.x,

        groundHeight +
            playerGroundOffset_,

        -(
            static_cast<float>(
                player_.tile.y -
                origin.y
            ) + player_.local.y
        )
    };

    object.visible =
        true;


    playerObjectIndex_ =
        scene_.objects.size();

    scene_.objects.push_back(
        object
    );

    syncPlayerRenderObject();


    std::cout
        << "=== PLAYER ===\n"
        << "tile              = "
        << player_.tile.x
        << ", "
        << player_.tile.y
        << ", plane "
        << player_.tile.plane
        << "\n"
        << "region local      = "
        << localX
        << ", "
        << localY
        << "\n"
        << "ground height     = "
        << groundHeight
        << "\n"
        << "ground offset     = "
        << playerGroundOffset_
        << "\n"
        << "appearance        = "
        << (
            usingDebugMarker
                ? "debug marker"
                : "default male identity kits"
        )
        << "\n\n";
}


int Client::run()
{

    if (!sdl_.valid()) {
        return 1;
    }

    buildWorld();

    bool running = true;

    using Clock =
        std::chrono::steady_clock;

    auto previousFrame =
        Clock::now();

    auto fpsStart =
        previousFrame;

    int fpsFrameCount = 0;

    while (running) {
        processEvents(running);

        const auto now =
            Clock::now();

        float dt =
            std::chrono::duration<float>(
                now - previousFrame
            ).count();

        previousFrame = now;

        if (dt > 0.1f) {
            dt = 0.1f;
        }

        int width = 0;
        int height = 0;

        SDL_GetWindowSizeInPixels(
            sdl_.window(),
            &width,
            &height
        );

        if (
            width > 0 &&
            height > 0
        ) {
            scene_.camera.viewportWidth =
                static_cast<std::uint32_t>(
                    width
                );

            scene_.camera.viewportHeight =
                static_cast<std::uint32_t>(
                    height
                );
        }

        const bool* keys =
            SDL_GetKeyboardState(
                nullptr
            );

        const float speed = 20.0f;
        const float step = speed * dt;

        float localX = 0.0f;
        float localY = 0.0f;
        float localZ = 0.0f;

        if (!cameraLocked_) {
            if (keys[SDL_SCANCODE_W])
                localZ -= step;

            if (keys[SDL_SCANCODE_S])
                localZ += step;

            if (keys[SDL_SCANCODE_A])
                localX -= step;

            if (keys[SDL_SCANCODE_D])
                localX += step;

            if (keys[SDL_SCANCODE_Q])
                localY -= step;

            if (keys[SDL_SCANCODE_E])
                localY += step;
        }

        const float yaw =
            scene_.camera.rotation.y;

        const float cy =
            std::cos(yaw);

        const float sy =
            std::sin(yaw);

        scene_.camera.position.x +=
            localX * cy -
            localZ * sy;

        scene_.camera.position.z +=
            localX * sy +
            localZ * cy;

        scene_.camera.position.y +=
            localY;

        const float turnSpeed =
            1.5f;

        const float turn =
            turnSpeed * dt;

        if (keys[SDL_SCANCODE_LEFT])
            scene_.camera.rotation.y +=
                turn;

        if (keys[SDL_SCANCODE_RIGHT])
            scene_.camera.rotation.y -=
                turn;

        if (keys[SDL_SCANCODE_UP])
            scene_.camera.rotation.x +=
                turn;

        if (keys[SDL_SCANCODE_DOWN])
            scene_.camera.rotation.x -=
                turn;

        constexpr float pitchLimit =
            1.55334306f;

        scene_.camera.rotation.x =
            std::clamp(
                scene_.camera.rotation.x,
                -pitchLimit,
                pitchLimit
            );

        updatePlayerMovement(
            dt
        );

        updatePlayerAnimation(
            dt
        );

        syncCameraToPlayer();

        updateMouseSelection();

        render();

        ++fpsFrameCount;

        const auto fpsNow =
            Clock::now();

        const float fpsElapsed =
            std::chrono::duration<float>(
                fpsNow - fpsStart
            ).count();

        if (fpsElapsed >= 0.5f) {
            const float fps =
                static_cast<float>(
                    fpsFrameCount
                ) /
                fpsElapsed;

            const auto& stats =
                renderer_.stats();

            std::ostringstream titleStream;

            titleStream
                << "Eldoria | "
                << static_cast<int>(fps)
                << " FPS | "
                << std::fixed
                << std::setprecision(2)
                << (1000.0f / fps)
                << " ms"
                << " | obj "
                << stats.objects
                << " | draws "
                << stats.drawCalls
                << " | tris "
                << stats.triangles
                << " | models "
                << stats.uniqueModels
                << " | tex "
                << stats.uniqueTextures
                << " | sections "
                << stats.sections
                << " | binds "
                << stats.textureBinds
                << " | sampler "
                << stats.samplerUpdates
                << " | upload M/T "
                << stats.modelUploads
                << "/"
                << stats.textureUploads
                << " | terrain draws "
                << renderer_.terrainDrawCalls()
                << " | location draws "
                << renderer_.locationDrawCalls();

            const std::string title =
                titleStream.str();

            std::cout
                << title
                << '\n';

            fpsStart = fpsNow;
            fpsFrameCount = 0;
        }
    }

    return 0;
}


}
