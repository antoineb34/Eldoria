#include "Client.h"

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
#include "render/scene/RenderScene.h"
#include "render/texture/TextureManager.h"

namespace eld::client {

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


int Client::run()
{

    if (!sdl_.valid()) {
        return 1;
    }

    // ========================================================
    // === SINGLE REGION ===
    // ========================================================

    constexpr std::uint16_t regionId =
        12850;



    eld::runtime::map::RegionBuilder
        regionBuilder;


    const auto worldRegion =
        regionBuilder.build(
            regionId,
            assets_.maps,
            assets_.locations
        );


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

    eld::render::RenderScene scene_;

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


    eld::render::Renderer3D
        renderer_(
            modelManager_,
            textureManager_
        );

    bool running = true;

    using Clock =
        std::chrono::steady_clock;

    auto previousFrame =
        Clock::now();

    auto fpsStart =
        previousFrame;

    int fpsFrameCount = 0;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (
                event.type ==
                SDL_EVENT_QUIT
            ) {
                running = false;
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
        }

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

        renderer_.render(scene_);

        SDL_GL_SwapWindow(
            sdl_.window()
        );

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
