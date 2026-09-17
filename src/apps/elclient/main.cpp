#include "map/LocationBuilder.h"
#include "model/ModelSystem.h"
#include <array>
#include <vector>
#include <iostream>
#include <cstdint>
#include <SDL3/SDL.h>

#include <algorithm>
#include <chrono>
#include <cmath>
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

int main()
{
    eld::host::SdlOpenGLContext sdl(
        "Eldoria",
        1000,
        700
    );

    if (!sdl.valid()) {
        return 1;
    }

    eld::asset::AssetManager assets;

    eld::render::TextureManager
        textureManager;

    eld::render::ModelManager
        modelManager;

    eld::graphics::TextureSystem textureSystem(
        assets.textures,
        textureManager
    );


    eld::graphics::ModelSystem modelSystem(
        assets.models,
        textureSystem,
        modelManager
    );

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
            assets.maps,
            assets.locations
        );


eld::graphics::TerrainBuilder
        terrainBuilder;

    auto terrainModel =
        terrainBuilder.build(
            worldRegion.terrain,
            0,
            assets.floors,
            textureSystem
        );

    const eld::render::ModelHandle
        terrainHandle =
            modelManager.create(
                std::move(
                    terrainModel
                )
            );

    eld::render::RenderScene scene;

    scene.camera.position = {
        31.5f,
        25.0f,
        85.0f
    };

    scene.camera.rotation = {
        -0.45f,
        0.0f,
        0.0f
    };

    scene.camera.viewportWidth =
        1000;

    scene.camera.viewportHeight =
        700;

    scene.objects.push_back({
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
            assets.locations,
            assets.models,
            modelSystem
        );


    scene.objects.insert(
        scene.objects.end(),
        locationBuild.objects.begin(),
        locationBuild.objects.end()
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


    eld::render::Renderer3D
        renderer(
            modelManager,
            textureManager
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
                renderer.toggleWireframe();
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
            sdl.window(),
            &width,
            &height
        );

        if (
            width > 0 &&
            height > 0
        ) {
            scene.camera.viewportWidth =
                static_cast<std::uint32_t>(
                    width
                );

            scene.camera.viewportHeight =
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
            scene.camera.rotation.y;

        const float cy =
            std::cos(yaw);

        const float sy =
            std::sin(yaw);

        scene.camera.position.x +=
            localX * cy -
            localZ * sy;

        scene.camera.position.z +=
            localX * sy +
            localZ * cy;

        scene.camera.position.y +=
            localY;

        const float turnSpeed =
            1.5f;

        const float turn =
            turnSpeed * dt;

        if (keys[SDL_SCANCODE_LEFT])
            scene.camera.rotation.y +=
                turn;

        if (keys[SDL_SCANCODE_RIGHT])
            scene.camera.rotation.y -=
                turn;

        if (keys[SDL_SCANCODE_UP])
            scene.camera.rotation.x +=
                turn;

        if (keys[SDL_SCANCODE_DOWN])
            scene.camera.rotation.x -=
                turn;

        constexpr float pitchLimit =
            1.55334306f;

        scene.camera.rotation.x =
            std::clamp(
                scene.camera.rotation.x,
                -pitchLimit,
                pitchLimit
            );

        renderer.render(scene);

        SDL_GL_SwapWindow(
            sdl.window()
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

            const std::string title =
                "Eldoria - " +
                std::to_string(
                    static_cast<int>(fps)
                ) +
                " FPS";

            SDL_SetWindowTitle(
                sdl.window(),
                title.c_str()
            );

            fpsStart = fpsNow;
            fpsFrameCount = 0;
        }
    }

    return 0;
}
