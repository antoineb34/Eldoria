#include <glad/gl.h>
#include "Client.h"
#include <exception>
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
#include <stdexcept>
#include <string>
#include <utility>

#include "assets/AssetManager.h"

#include "map/TerrainBuilder.h"
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


constexpr std::uint16_t InitialRegionId =
    12850;


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
      worldStreamer_(
          world_,
          assets_.maps,
          assets_.locations
      ),
      playerController_(
          player_,
          world_,
          pathfinder_
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

    // --------------------------------------------------------
    // Runtime world ownership
    // --------------------------------------------------------

    world_ =
        eld::world::World{};


    const auto centerCoord =
        eld::world::World::
            regionCoordFromId(
                InitialRegionId
            );


    const eld::world::TilePosition
        initialPlayerTile{
            centerCoord.x *
                eld::world::World::RegionSize +
                32,

            centerCoord.y *
                eld::world::World::RegionSize +
                32,

            0
        };


    const auto initialStreaming =
        worldStreamer_.update(
            initialPlayerTile
        );


    for (
        const auto& failure :
        initialStreaming.failures
    ) {
        std::cout
            << "world load failed | region="
            << failure.regionId
            << " | "
            << failure.message
            << "\n";
    }


    const auto* centerRegion =
        world_.region(
            InitialRegionId
        );


    if (!centerRegion) {
        throw std::runtime_error(
            "Initial region 12850 failed to load"
        );
    }


    sceneOrigin_ =
        centerRegion
            ->terrain
            .origin();


    // --------------------------------------------------------
    // Camera
    // --------------------------------------------------------

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


    // --------------------------------------------------------
    // Graphics builders
    //
    // These consume Regions owned by World.
    // They do not own runtime map state.
    // --------------------------------------------------------

    eld::graphics::TerrainBuilder
        terrainBuilder;

    eld::graphics::LocationBuilder
        locationBuilder;

    eld::graphics::LocationBatchBuilder
        locationBatchBuilder;


    std::cout
        << "\n=== WORLD SCENE BUILD ===\n";


    for (
        const auto id :
        world_.loadedRegionIds()
    ) {
        const auto* region =
            world_.region(
                id
            );


        if (!region) {
            continue;
        }


        const auto& origin =
            region
                ->terrain
                .origin();


        const float sceneOffsetX =
            static_cast<float>(
                origin.x -
                sceneOrigin_.x
            );

        const float sceneOffsetZ =
            -static_cast<float>(
                origin.y -
                sceneOrigin_.y
            );


        // ----------------------------------------------------
        // Terrain
        // ----------------------------------------------------

        auto terrainModel =
            terrainBuilder.build(
                region->terrain,
                0,
                assets_.floors,
                textureSystem_
            );


        const auto terrainHandle =
            modelManager_.create(
                std::move(
                    terrainModel
                )
            );


        eld::render::RenderObject
            terrainObject;

        terrainObject.model =
            terrainHandle;

        terrainObject.transform.position = {
            sceneOffsetX,
            0.0f,
            sceneOffsetZ
        };

        terrainObject.visible =
            true;


        scene_.objects.push_back(
            terrainObject
        );


        // ----------------------------------------------------
        // Locations
        // ----------------------------------------------------

        auto locationBuild =
            locationBuilder.build(
                *region,
                0,
                assets_.locations,
                assets_.models,
                modelSystem_
            );


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
                    std::move(
                        batch
                    )
                );


            eld::render::RenderObject
                object;

            object.model =
                handle;

            object.transform.position = {
                sceneOffsetX,
                0.0f,
                sceneOffsetZ
            };

            object.visible =
                true;


            scene_.objects.push_back(
                object
            );
        }


        for (
            auto object :
            locationBatchBuild
                .passthroughObjects
        ) {
            object.transform.position.x +=
                sceneOffsetX;

            object.transform.position.z +=
                sceneOffsetZ;


            scene_.objects.push_back(
                std::move(
                    object
                )
            );
        }


        std::cout
            << "region "
            << id
            << " | origin="
            << origin.x
            << ","
            << origin.y
            << " | offset="
            << sceneOffsetX
            << ","
            << sceneOffsetZ
            << " | locations="
            << region->locations.size()
            << "\n";
    }


    std::cout
        << "world regions = "
        << world_.loadedRegionCount()
        << "\n"
        << "=========================\n\n";


    spawnPlayer();
}

void Client::processEvents(
    bool& running
)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        // ----------------------------------------------------
        // Camera zoom
        // ----------------------------------------------------

        if (
            event.type ==
            SDL_EVENT_MOUSE_WHEEL
        ) {
            float wheel =
                event.wheel.y;


            if (
                event.wheel.direction ==
                SDL_MOUSEWHEEL_FLIPPED
            ) {
                wheel =
                    -wheel;
            }


            constexpr float zoomStep =
                1.5f;


            cameraDistance_ =
                std::clamp(
                    cameraDistance_ -
                        wheel *
                        zoomStep,
                    cameraMinDistance_,
                    cameraMaxDistance_
                );
        }


        // ----------------------------------------------------
        // Middle-mouse orbit
        // ----------------------------------------------------

        if (
            event.type ==
                SDL_EVENT_MOUSE_BUTTON_DOWN &&
            event.button.button ==
                SDL_BUTTON_MIDDLE
        ) {
            cameraDragging_ =
                true;
        }


        if (
            event.type ==
                SDL_EVENT_MOUSE_BUTTON_UP &&
            event.button.button ==
                SDL_BUTTON_MIDDLE
        ) {
            cameraDragging_ =
                false;
        }


        if (
            event.type ==
                SDL_EVENT_MOUSE_MOTION &&
            cameraDragging_
        ) {
            constexpr float sensitivity =
                0.005f;


            scene_.camera.rotation.y -=
                event.motion.xrel *
                sensitivity;


            scene_.camera.rotation.x +=
                event.motion.yrel *
                sensitivity;


            constexpr float minPitch =
                -1.35f;

            constexpr float maxPitch =
                -0.15f;


            scene_.camera.rotation.x =
                std::clamp(
                    scene_.camera.rotation.x,
                    minPitch,
                    maxPitch
                );
        }




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

    drawClickCross();

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
    const auto movement =
        playerController_.update(
            dt
        );


    if (
        movement.positionChanged
    ) {
        syncPlayerRenderObject();
    }


    if (
        !movement.stepCompleted
    ) {
        return;
    }


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


    if (
        playerController_.continuePath()
    ) {
        setPlayerWalking(
            true
        );

        syncPlayerRenderObject();
    }
}

std::optional<eld::world::TilePosition>
Client::pickTerrainTile(
    float mouseX,
    float mouseY
) const
{
    const auto* pickRegion =
        world_.region(
            InitialRegionId
        );

    if (
        !pickRegion ||
        scene_.camera.viewportWidth == 0 ||
        scene_.camera.viewportHeight == 0
    ) {
        return std::nullopt;
    }


    const auto viewMatrix =
        eld::render::buildViewMatrix(
            scene_.camera
        );

    const auto projectionMatrix =
        eld::render::buildProjectionMatrix(
            scene_.camera
        );


    // This flip is already proven correct for the existing
    // renderer / SDL coordinate convention.
    // WINDOW TO VIEWPORT MOUSE SCALE
    int windowWidth = 0;
    int windowHeight = 0;

    SDL_GetWindowSize(
        sdl_.window(),
        &windowWidth,
        &windowHeight
    );


    if (
        windowWidth > 0 &&
        windowHeight > 0
    ) {
        mouseX *=
            static_cast<float>(
                scene_.camera.viewportWidth
            ) /
            static_cast<float>(
                windowWidth
            );

        mouseY *=
            static_cast<float>(
                scene_.camera.viewportHeight
            ) /
            static_cast<float>(
                windowHeight
            );
    }


    const float projectedMouseX =
        mouseX;

    const float projectedMouseY =
        mouseY;


    const auto pointInTriangle =
        [](
            float px,
            float py,
            const eld::render::ScreenPoint& a,
            const eld::render::ScreenPoint& b,
            const eld::render::ScreenPoint& c
        )
        {
            const auto edge =
                [](
                    float px,
                    float py,
                    const eld::render::ScreenPoint& p0,
                    const eld::render::ScreenPoint& p1
                )
                {
                    return
                        (
                            px - p1.x
                        ) *
                            (
                                p0.y - p1.y
                            ) -
                        (
                            p0.x - p1.x
                        ) *
                            (
                                py - p1.y
                            );
                };


            const float d1 =
                edge(
                    px,
                    py,
                    a,
                    b
                );

            const float d2 =
                edge(
                    px,
                    py,
                    b,
                    c
                );

            const float d3 =
                edge(
                    px,
                    py,
                    c,
                    a
                );


            const bool hasNegative =
                d1 < 0.0f ||
                d2 < 0.0f ||
                d3 < 0.0f;

            const bool hasPositive =
                d1 > 0.0f ||
                d2 > 0.0f ||
                d3 > 0.0f;


            return !(
                hasNegative &&
                hasPositive
            );
        };


    float bestDepth =
        std::numeric_limits<
            float
        >::infinity();


    std::optional<
        eld::world::TilePosition
    > result;


    const auto& sceneOrigin =
        sceneOrigin_;


    const auto testTerrain =
        [&](
            const eld::world::Terrain& terrain
        )
        {
            for (
                std::size_t localY = 0;
                localY <
                    terrain.height();
                ++localY
            ) {
                for (
                    std::size_t localX = 0;
                    localX <
                        terrain.width();
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


                    // Convert absolute world tile position into
                    // the same scene coordinates used when the
                    // neighbouring chunks were rendered.
                    const float x0 =
                        static_cast<float>(
                            position.x -
                            sceneOrigin.x
                        );

                    const float x1 =
                        x0 +
                        1.0f;


                    const float z0 =
                        -static_cast<float>(
                            position.y -
                            sceneOrigin.y
                        );

                    const float z1 =
                        z0 -
                        1.0f;


                    const auto southwest =
                        eld::render::projectPoint(
                            {
                                x0,
                                heights.southwest,
                                z0
                            },
                            viewMatrix,
                            projectionMatrix,
                            scene_.camera
                        );

                    const auto southeast =
                        eld::render::projectPoint(
                            {
                                x1,
                                heights.southeast,
                                z0
                            },
                            viewMatrix,
                            projectionMatrix,
                            scene_.camera
                        );

                    const auto northeast =
                        eld::render::projectPoint(
                            {
                                x1,
                                heights.northeast,
                                z1
                            },
                            viewMatrix,
                            projectionMatrix,
                            scene_.camera
                        );

                    const auto northwest =
                        eld::render::projectPoint(
                            {
                                x0,
                                heights.northwest,
                                z1
                            },
                            viewMatrix,
                            projectionMatrix,
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


                            const float depth =
                                std::abs(
                                    (
                                        a.depth +
                                        b.depth +
                                        c.depth
                                    ) /
                                    3.0f
                                );


                            if (
                                depth >=
                                bestDepth
                            ) {
                                return;
                            }


                            bestDepth =
                                depth;


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


                    consider(
                        southwest,
                        southeast,
                        northeast
                    );

                    consider(
                        southwest,
                        northeast,
                        northwest
                    );
                }
            }
        };


    testTerrain(
        pickRegion->terrain
    );


    // TEMP PICKER: CENTER REGION ONLY
    //
    // Neighbor terrain picking is disabled while we
    // isolate the incorrect click destination.


    return result;
}


const eld::world::Location*
Client::findInteractableLocationAt(
    const eld::world::TilePosition& tile
) const
{
    const auto* owner =
        world_.regionAt(
            tile
        );


    if (!owner) {
        return nullptr;
    }


    for (
        const auto& location :
        owner->locations
    ) {
        if (
            !location.interactable ||
            location.tile.plane !=
                tile.plane
        ) {
            continue;
        }


        const int minX =
            location.tile.x;

        const int minY =
            location.tile.y;

        const int maxX =
            minX +
            location.footprintWidth;

        const int maxY =
            minY +
            location.footprintLength;


        if (
            tile.x >= minX &&
            tile.x < maxX &&
            tile.y >= minY &&
            tile.y < maxY
        ) {
            return &location;
        }
    }


    return nullptr;
}

void Client::selectTerrainTile(
    const eld::world::TilePosition& tile,
    bool interactable
)
{
    auto* owner =
        world_.regionAt(
            tile
        );

    if (!owner) {
        return;
    }


    auto& terrain =
        owner->terrain;


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
    if (interactable) {
        material.baseColor = {
            1.0f,
            0.0f,
            0.0f,
            1.0f
        };
    }
    else {
        material.baseColor = {
            1.0f,
            1.0f,
            0.0f,
            1.0f
        };
    }

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

            if (interactable) {
                vertex.color = {
                    1.0f,
                    0.0f,
                    0.0f,
                    1.0f
                };
            }
            else {
                vertex.color = {
                    1.0f,
                    1.0f,
                    0.0f,
                    1.0f
                };
            }

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
        sceneOrigin_;


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
}


void Client::drawClickCross()
{
    if (!clickCrossActive_) {
        return;
    }


    const std::uint64_t now =
        static_cast<std::uint64_t>(
            SDL_GetTicks()
        );

    const std::uint64_t elapsed =
        now >= clickCrossStartMs_
            ? now - clickCrossStartMs_
            : 0;


    // Classic client:
    //
    // four frames,
    // approximately 100 ms each.
    if (elapsed >= 400) {
        clickCrossActive_ = false;
        return;
    }


    const std::uint16_t animationFrame =
        static_cast<std::uint16_t>(
            elapsed / 100
        );

    const std::uint16_t frameId =
        static_cast<std::uint16_t>(
            (clickCrossRed_ ? 4 : 0) +
            animationFrame
        );


    const auto sprite =
        assets_.sprites.find(
            "cross.dat",
            frameId
        );


    if (!sprite.has_value()) {
        return;
    }


    const auto& image =
        sprite->image;


    if (
        image.width == 0 ||
        image.height == 0 ||
        image.pixels.empty()
    ) {
        return;
    }


    const float viewportWidth =
        static_cast<float>(
            scene_.camera.viewportWidth
        );

    const float viewportHeight =
        static_cast<float>(
            scene_.camera.viewportHeight
        );


    if (
        viewportWidth <= 0.0f ||
        viewportHeight <= 0.0f
    ) {
        return;
    }


    struct CrossGlState
    {
        GLuint program = 0;

        GLuint vao = 0;
        GLuint vbo = 0;

        GLint textureLocation = -1;

        std::array<GLuint, 8>
            textures{};

        bool failed = false;
    };


    static CrossGlState gl;


    // --------------------------------------------------------
    // Lazy-create tiny screen-space sprite renderer.
    // --------------------------------------------------------

    if (
        gl.program == 0 &&
        !gl.failed
    ) {
        const auto compileShader =
            [](
                GLenum type,
                const char* source
            ) -> GLuint
        {
            const GLuint shader =
                glCreateShader(
                    type
                );

            glShaderSource(
                shader,
                1,
                &source,
                nullptr
            );

            glCompileShader(
                shader
            );


            GLint success =
                GL_FALSE;

            glGetShaderiv(
                shader,
                GL_COMPILE_STATUS,
                &success
            );


            if (success != GL_TRUE) {
                glDeleteShader(
                    shader
                );

                return 0;
            }


            return shader;
        };


        const char* vertexSource =
            R"(
                #version 330 core

                layout(location = 0)
                in vec2 aPosition;

                layout(location = 1)
                in vec2 aUV;

                out vec2 vUV;

                void main()
                {
                    vUV = aUV;

                    gl_Position =
                        vec4(
                            aPosition,
                            0.0,
                            1.0
                        );
                }
            )";


        const char* fragmentSource =
            R"(
                #version 330 core

                in vec2 vUV;

                out vec4 FragColor;

                uniform sampler2D uTexture;

                void main()
                {
                    vec4 pixel =
                        texture(
                            uTexture,
                            vUV
                        );

                    if (pixel.a <= 0.0) {
                        discard;
                    }

                    FragColor =
                        pixel;
                }
            )";


        const GLuint vertexShader =
            compileShader(
                GL_VERTEX_SHADER,
                vertexSource
            );

        const GLuint fragmentShader =
            compileShader(
                GL_FRAGMENT_SHADER,
                fragmentSource
            );


        if (
            vertexShader == 0 ||
            fragmentShader == 0
        ) {
            if (vertexShader != 0) {
                glDeleteShader(
                    vertexShader
                );
            }

            if (fragmentShader != 0) {
                glDeleteShader(
                    fragmentShader
                );
            }

            gl.failed =
                true;

            std::cout
                << "click cross shader failed\n";

            return;
        }


        gl.program =
            glCreateProgram();

        glAttachShader(
            gl.program,
            vertexShader
        );

        glAttachShader(
            gl.program,
            fragmentShader
        );

        glLinkProgram(
            gl.program
        );


        glDeleteShader(
            vertexShader
        );

        glDeleteShader(
            fragmentShader
        );


        GLint linked =
            GL_FALSE;

        glGetProgramiv(
            gl.program,
            GL_LINK_STATUS,
            &linked
        );


        if (linked != GL_TRUE) {
            glDeleteProgram(
                gl.program
            );

            gl.program = 0;
            gl.failed = true;

            std::cout
                << "click cross program link failed\n";

            return;
        }


        gl.textureLocation =
            glGetUniformLocation(
                gl.program,
                "uTexture"
            );


        glGenVertexArrays(
            1,
            &gl.vao
        );

        glGenBuffers(
            1,
            &gl.vbo
        );


        glBindVertexArray(
            gl.vao
        );

        glBindBuffer(
            GL_ARRAY_BUFFER,
            gl.vbo
        );


        // 6 vertices * (x, y, u, v)
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(float) *
                6 *
                4,
            nullptr,
            GL_DYNAMIC_DRAW
        );


        constexpr GLsizei stride =
            4 *
            sizeof(float);


        glVertexAttribPointer(
            0,
            2,
            GL_FLOAT,
            GL_FALSE,
            stride,
            nullptr
        );

        glEnableVertexAttribArray(
            0
        );


        glVertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            stride,
            reinterpret_cast<void*>(
                2 *
                sizeof(float)
            )
        );

        glEnableVertexAttribArray(
            1
        );


        glBindBuffer(
            GL_ARRAY_BUFFER,
            0
        );

        glBindVertexArray(
            0
        );
    }


    if (
        gl.failed ||
        gl.program == 0
    ) {
        return;
    }


    // --------------------------------------------------------
    // Upload this cache frame the first time it is used.
    // --------------------------------------------------------

    GLuint& texture =
        gl.textures[
            static_cast<std::size_t>(
                frameId
            )
        ];


    if (texture == 0) {
        static_assert(
            sizeof(
                eld::image::RgbaPixel
            ) == 4
        );


        glGenTextures(
            1,
            &texture
        );

        glBindTexture(
            GL_TEXTURE_2D,
            texture
        );


        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_NEAREST
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            GL_NEAREST
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE
        );


        glPixelStorei(
            GL_UNPACK_ALIGNMENT,
            1
        );


        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            static_cast<GLsizei>(
                image.width
            ),
            static_cast<GLsizei>(
                image.height
            ),
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            image.pixels.data()
        );
    }


    // --------------------------------------------------------
    // Cross is centered on the actual click location.
    // --------------------------------------------------------

    const float halfWidth =
        static_cast<float>(
            image.width
        ) *
        0.5f;

    const float halfHeight =
        static_cast<float>(
            image.height
        ) *
        0.5f;


    const float left =
        clickCrossX_ -
        halfWidth;

    const float right =
        clickCrossX_ +
        halfWidth;

    const float top =
        clickCrossY_ -
        halfHeight;

    const float bottom =
        clickCrossY_ +
        halfHeight;


    const float x0 =
        (
            left /
            viewportWidth
        ) *
            2.0f -
        1.0f;

    const float x1 =
        (
            right /
            viewportWidth
        ) *
            2.0f -
        1.0f;

    const float y0 =
        1.0f -
        (
            top /
            viewportHeight
        ) *
            2.0f;

    const float y1 =
        1.0f -
        (
            bottom /
            viewportHeight
        ) *
            2.0f;


    // top-left origin for the decoded cache image
    const float vertices[] = {
        x0, y0, 0.0f, 0.0f,
        x0, y1, 0.0f, 1.0f,
        x1, y1, 1.0f, 1.0f,

        x0, y0, 0.0f, 0.0f,
        x1, y1, 1.0f, 1.0f,
        x1, y0, 1.0f, 0.0f
    };


    glDisable(
        GL_DEPTH_TEST
    );

    glDepthMask(
        GL_FALSE
    );

    glEnable(
        GL_BLEND
    );

    glBlendFunc(
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA
    );

    glPolygonMode(
        GL_FRONT_AND_BACK,
        GL_FILL
    );


    glUseProgram(
        gl.program
    );


    glActiveTexture(
        GL_TEXTURE0
    );

    glBindTexture(
        GL_TEXTURE_2D,
        texture
    );


    glUniform1i(
        gl.textureLocation,
        0
    );


    glBindVertexArray(
        gl.vao
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        gl.vbo
    );

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(vertices),
        vertices
    );


    glDrawArrays(
        GL_TRIANGLES,
        0,
        6
    );


    glBindBuffer(
        GL_ARRAY_BUFFER,
        0
    );

    glBindVertexArray(
        0
    );

    glBindTexture(
        GL_TEXTURE_2D,
        0
    );

    glUseProgram(
        0
    );


    glDisable(
        GL_BLEND
    );

    glDepthMask(
        GL_TRUE
    );

    glEnable(
        GL_DEPTH_TEST
    );
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
        const auto pickedTile =
            pickTerrainTile(
                mouseX,
                mouseY
            );


        const eld::world::Location*
            pickedLocation =
                nullptr;


        if (pickedTile.has_value()) {
            pickedLocation =
                findInteractableLocationAt(
                    *pickedTile
                );
        }


        if (!pickedTile.has_value()) {
            std::cout
                << "clicked target = none\n";
        }
        else {
            clickCrossActive_ =
                true;

            clickCrossRed_ =
                pickedLocation !=
                nullptr;

            clickCrossX_ =
                mouseX;

            clickCrossY_ =
                mouseY;

            clickCrossStartMs_ =
                static_cast<std::uint64_t>(
                    SDL_GetTicks()
                );


            if (pickedLocation) {
                std::cout
                    << "clicked location = "
                    << pickedLocation->id
                    << " | tile="
                    << pickedLocation->tile.x
                    << ","
                    << pickedLocation->tile.y
                    << " | shape="
                    << static_cast<int>(
                        pickedLocation->shape
                    )
                    << "\n";
            }
            else {
                std::cout
                    << "clicked ground = "
                    << pickedTile->x
                    << ", "
                    << pickedTile->y
                    << ", plane "
                    << pickedTile->plane
                    << "\n";


                walkPlayerTo(
                    *pickedTile
                );
            }
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


    if (!cameraOrbitInitialized_) {
        scene_.camera.rotation.x =
            -0.55f;

        scene_.camera.rotation.z =
            0.0f;

        cameraOrbitInitialized_ =
            true;
    }


    const auto& playerPosition =
        scene_.objects[
            *playerObjectIndex_
        ].transform.position;


    const float yaw =
        scene_.camera.rotation.y;

    const float pitch =
        scene_.camera.rotation.x;


    const float sinYaw =
        std::sin(yaw);

    const float cosYaw =
        std::cos(yaw);

    const float sinPitch =
        std::sin(pitch);

    const float cosPitch =
        std::cos(pitch);


    const float targetX =
        playerPosition.x;

    const float targetY =
        playerPosition.y +
        cameraTargetHeight_;

    const float targetZ =
        playerPosition.z;


    scene_.camera.position = {
        targetX +
            sinYaw *
            cosPitch *
            cameraDistance_,

        targetY -
            sinPitch *
            cameraDistance_,

        targetZ +
            cosYaw *
            cosPitch *
            cameraDistance_
    };
}


void Client::syncPlayerRenderObject()
{
    if (
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


    const auto* playerRegion =
        regionAt(
            player_.tile.x,
            player_.tile.y
        );


    if (!playerRegion) {
        return;
    }


    const auto& terrain =
        playerRegion->terrain;


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


    // IMPORTANT:
    //
    // All nine rendered regions were positioned relative to
    // the original center map square. Keep the player in that
    // same scene coordinate system even after crossing into
    // another region.
    const auto& sceneOrigin =
        sceneOrigin_;


    auto& object =
        scene_.objects.at(
            *playerObjectIndex_
        );


    constexpr float eighthTurn =
        0.78539816339f;

    constexpr float quarterTurn =
        1.57079632679f;

    constexpr float halfTurn =
        3.14159265359f;


    switch (player_.facing) {
    case FacingDirection::North:
        object.transform.rotation.y =
            halfTurn;
        break;

    case FacingDirection::NorthEast:
        object.transform.rotation.y =
            halfTurn -
            eighthTurn;
        break;

    case FacingDirection::East:
        object.transform.rotation.y =
            quarterTurn;
        break;

    case FacingDirection::SouthEast:
        object.transform.rotation.y =
            eighthTurn;
        break;

    case FacingDirection::South:
        object.transform.rotation.y =
            0.0f;
        break;

    case FacingDirection::SouthWest:
        object.transform.rotation.y =
            -eighthTurn;
        break;

    case FacingDirection::West:
        object.transform.rotation.y =
            halfTurn +
            quarterTurn;
        break;

    case FacingDirection::NorthWest:
        object.transform.rotation.y =
            halfTurn +
            eighthTurn;
        break;
    }


    object.transform.position = {
        static_cast<float>(
            player_.tile.x -
            sceneOrigin.x
        ) +
            player_.local.x,

        groundHeight +
            playerGroundOffset_,

        -(
            static_cast<float>(
                player_.tile.y -
                sceneOrigin.y
            ) +
            player_.local.y
        )
    };
}


void Client::walkPlayerTo(
    const eld::world::TilePosition& destination
)
{
    const auto walk =
        playerController_.walkTo(
            destination
        );


    const auto& source =
        walk.source;

    const auto& result =
        walk.path;


    switch (result.status) {
    case eld::world::PathStatus::Success:
        break;


    case eld::world::PathStatus::RegionMissing:
        std::cout
            << "path region missing | "
            << source.x
            << ","
            << source.y
            << " -> "
            << destination.x
            << ","
            << destination.y
            << "\n";

        return;


    case eld::world::PathStatus::
        CrossRegionUnsupported:
        std::cout
            << "path crosses region seam | "
            << source.x
            << ","
            << source.y
            << " -> "
            << destination.x
            << ","
            << destination.y
            << "\n";

        return;


    case eld::world::PathStatus::NotFound:
        std::cout
            << "path not found | "
            << source.x
            << ","
            << source.y
            << " -> "
            << destination.x
            << ","
            << destination.y
            << "\n";

        return;


    case eld::world::PathStatus::
        BrokenParentChain:
        std::cout
            << "broken path parent chain\n";

        return;


    case eld::world::PathStatus::SameTile:
    case eld::world::PathStatus::PlaneMismatch:
    case eld::world::PathStatus::InvalidRegion:
        return;
    }


    std::cout
        << "path | "
        << source.x
        << ","
        << source.y
        << " -> "
        << destination.x
        << ","
        << destination.y
        << " steps="
        << result.steps
        << " turns="
        << result.turns
        << "\n";


    if (playerController_.moving()) {
        setPlayerWalking(
            true
        );

        syncPlayerRenderObject();
    }
}

void Client::movePlayer(
    int dx,
    int dy
)
{
    if (
        !playerController_.moveBy(
            dx,
            dy
        )
    ) {
        return;
    }


    setPlayerWalking(
        true
    );

    syncPlayerRenderObject();
}

const eld::world::Region*
Client::regionAt(
    int worldX,
    int worldY
) const
{
    return
        world_.regionAt({
            worldX,
            worldY,
            0
        });
}



void Client::spawnPlayer()
{
    const auto* spawnRegion =
        world_.region(
            InitialRegionId
        );

    if (!spawnRegion) {
        return;
    }

    const auto& terrain =
        spawnRegion->terrain;


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
        sceneOrigin_;


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

        constexpr float minCameraPitch =
            -1.35f;

        constexpr float maxCameraPitch =
            -0.15f;

        scene_.camera.rotation.x =
            std::clamp(
                scene_.camera.rotation.x,
                minCameraPitch,
                maxCameraPitch
            );

        updatePlayerMovement(
            dt
        );

        const auto streaming =
            worldStreamer_.update(
                player_.tile
            );


        for (
            const auto& failure :
            streaming.failures
        ) {
            std::cout
                << "world load failed | region="
                << failure.regionId
                << " | "
                << failure.message
                << "\n";
        }


        if (
            streaming.plan.centerChanged ||
            !streaming.loaded.empty() ||
            !streaming.unloaded.empty() ||
            !streaming.failures.empty()
        ) {
            for (
                const auto id :
                streaming.loaded
            ) {
                std::cout
                    << "world load | region="
                    << id
                    << "\n";
            }


            for (
                const auto id :
                streaming.unloaded
            ) {
                std::cout
                    << "world unload | region="
                    << id
                    << "\n";
            }


            std::cout
                << "=== WORLD STREAM ===\n"
                << "player tile   = "
                << player_.tile.x
                << ","
                << player_.tile.y
                << "\n"
                << "center region = "
                << streaming.plan.center.x
                << ","
                << streaming.plan.center.y
                << "\n"
                << "world regions = "
                << world_.loadedRegionCount()
                << "\n"
                << "====================\n";
        }


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
