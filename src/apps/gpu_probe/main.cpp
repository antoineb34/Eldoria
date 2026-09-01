#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "cache/Cache.h"
#include "cache/Index.h"
#include "definition/DefinitionRepository.h"
#include "definition/floor/FloorRepository.h"
#include "definition/location/LocationRepository.h"
#include "GraphicsResources.h"
#include "map/MapLoader.h"
#include "map/SceneLocBuilder.h"
#include "map/SceneLocModelBuilder.h"
#include "map/SceneMapRenderModelBuilder.h"
#include "model/ModelRepository.h"
#include "sdl/SdlOpenGLContext.h"
#include "RenderPipeline.h"
#include "backend/opengl/OpenGLRenderBackend.h"
#include "scene/RenderScene.h"
#include "texture/TextureRepository.h"

namespace {

constexpr float Pi = 3.14159265358979323846f;
constexpr float TwoPi = Pi * 2.0f;
constexpr float MapTileUnits = 128.0f;
constexpr float MapCenterTiles =
    static_cast<float>(eld::map::RegionSize) * 0.5f;

std::uint16_t parseU16(
    const char* text,
    const char* label
) {
    const unsigned long value = std::stoul(text);

    if (value > std::numeric_limits<std::uint16_t>::max()) {
        throw std::out_of_range(
            std::string(label) + " exceeds uint16 range"
        );
    }

    return static_cast<std::uint16_t>(value);
}

struct TerrainNeighborhoodRegion {
    int offsetX = 0;
    int offsetY = 0;
    eld::map::MapRegion region;
};

struct TerrainNeighborhood {
    std::vector<TerrainNeighborhoodRegion> regions;
    std::vector<std::uint16_t> missingRegionIds;
};

TerrainNeighborhood loadTerrainNeighborhood(
    const eld::map::MapLoader& loader,
    const eld::map::MapIndexEntry& center
) {
    TerrainNeighborhood neighborhood;
    neighborhood.regions.reserve(9);

    for (int offsetX = -1; offsetX <= 1; ++offsetX) {
        for (int offsetY = -1; offsetY <= 1; ++offsetY) {
            const int regionX = center.regionX() + offsetX;
            const int regionY = center.regionY() + offsetY;

            if (
                regionX < 0 || regionX > 255 ||
                regionY < 0 || regionY > 255
            ) {
                continue;
            }

            const std::uint16_t candidateId =
                static_cast<std::uint16_t>(
                    (regionX << 8) | regionY
                );

            if (loader.find(candidateId) == nullptr) {
                neighborhood.missingRegionIds.push_back(candidateId);
                continue;
            }

            neighborhood.regions.push_back({
                offsetX,
                offsetY,
                loader.loadTerrain(candidateId)
            });
        }
    }

    return neighborhood;
}

const eld::map::MapRegion* neighborhoodTerrain(
    const TerrainNeighborhood& neighborhood,
    int offsetX,
    int offsetY
) {
    for (const TerrainNeighborhoodRegion& region : neighborhood.regions) {
        if (
            region.offsetX == offsetX &&
            region.offsetY == offsetY
        ) {
            return &region.region;
        }
    }

    return nullptr;
}

int regionOffsetForLocalCoordinate(
    int coordinate
) {
    if (coordinate >= 0) {
        return coordinate / static_cast<int>(eld::map::RegionSize);
    }

    return -(
        (-coordinate + static_cast<int>(eld::map::RegionSize) - 1) /
        static_cast<int>(eld::map::RegionSize)
    );
}

const eld::map::MapTile* sampleTerrainNeighborhood(
    const TerrainNeighborhood& neighborhood,
    std::size_t plane,
    int x,
    int y
) {
    if (plane >= eld::map::PlaneCount) {
        return nullptr;
    }

    const int offsetX = regionOffsetForLocalCoordinate(x);
    const int offsetY = regionOffsetForLocalCoordinate(y);

    if (
        offsetX < -1 || offsetX > 1 ||
        offsetY < -1 || offsetY > 1
    ) {
        return nullptr;
    }

    const eld::map::MapRegion* terrain =
        neighborhoodTerrain(
            neighborhood,
            offsetX,
            offsetY
        );

    if (terrain == nullptr) {
        return nullptr;
    }

    const int localX =
        x - offsetX * static_cast<int>(eld::map::RegionSize);
    const int localY =
        y - offsetY * static_cast<int>(eld::map::RegionSize);

    if (
        localX < 0 || localX >= static_cast<int>(eld::map::RegionSize) ||
        localY < 0 || localY >= static_cast<int>(eld::map::RegionSize)
    ) {
        return nullptr;
    }

    return &terrain->tile(
        plane,
        static_cast<std::size_t>(localX),
        static_cast<std::size_t>(localY)
    );
}

float terrainWorldHeight(
    int classicHeight
) {
    return -static_cast<float>(classicHeight) / MapTileUnits;
}

float averagePlaneHeight(
    const eld::map::MapRegion& region,
    std::size_t plane
) {
    if (plane >= eld::map::PlaneCount) {
        return 0.0f;
    }

    std::int64_t sum = 0;
    std::size_t count = 0;

    for (std::size_t x = 0; x < eld::map::RegionSize; ++x) {
        for (std::size_t y = 0; y < eld::map::RegionSize; ++y) {
            sum += region.tile(plane, x, y).height;
            ++count;
        }
    }

    if (count == 0) {
        return 0.0f;
    }

    return terrainWorldHeight(
        static_cast<int>(
            sum / static_cast<std::int64_t>(count)
        )
    );
}

eld::render::Transform classicMapTransform() {
    eld::render::Transform transform;
    transform.position = {
        -MapCenterTiles,
        0.0f,
        -MapCenterTiles
    };
    transform.scale = {
        1.0f / MapTileUnits,
        1.0f / MapTileUnits,
        1.0f / MapTileUnits
    };
    return transform;
}

bool diagonalDecorationUsesInset(
    int rotation,
    int sceneX,
    int sceneZ,
    const eld::math::Vec3& camera
) {
    const float eyeSceneX =
        (camera.x + MapCenterTiles) * MapTileUnits;
    const float eyeSceneZ =
        (camera.z + MapCenterTiles) * MapTileUnits;
    const float relativeX =
        static_cast<float>(sceneX) - eyeSceneX;
    const float relativeZ =
        static_cast<float>(sceneZ) - eyeSceneZ;

    const int cardinal = rotation & 3;
    const float nearestX =
        cardinal == 1 || cardinal == 2
            ? -relativeX
            : relativeX;
    const float nearestZ =
        cardinal == 2 || cardinal == 3
            ? -relativeZ
            : relativeZ;

    return nearestZ <= nearestX;
}

struct GpuCameraLoc {
    std::uint8_t scenePlane = 0;
    std::uint8_t rotation = 0;
    int sceneX = 0;
    int sceneZ = 0;
    std::size_t insetObject = 0;
    std::size_t outsetObject = 0;
};

int runModelProbe(
    const std::string& cacheRoot,
    std::optional<std::uint16_t> requestedModelId
) {
    eld::cache::Cache cache(cacheRoot);

    eld::model::ModelRepository modelRepository(
        cache.open(eld::cache::IndexId::Models)
    );
    eld::texture::TextureRepository textureRepository(
        cache.open(eld::cache::IndexId::Config)
    );

    const std::vector<std::uint16_t> ids =
        modelRepository.listIds();

    if (ids.empty()) {
        throw std::runtime_error(
            "model repository is empty"
        );
    }

    std::optional<std::uint16_t> selectedModelId =
        requestedModelId;

    if (selectedModelId.has_value()) {
        if (!modelRepository.contains(*selectedModelId)) {
            throw std::runtime_error(
                "model repository does not contain id " +
                std::to_string(*selectedModelId)
            );
        }
    }
    else {
        for (const std::uint16_t candidate : ids) {
            try {
                const eld::model::ModelMesh mesh =
                    modelRepository.getMesh(candidate);

                if (
                    !mesh.vertices.empty() &&
                    !mesh.faces.empty()
                ) {
                    selectedModelId = candidate;
                    break;
                }
            }
            catch (const std::exception&) {
                continue;
            }
        }
    }

    if (!selectedModelId.has_value()) {
        throw std::runtime_error(
            "no non-empty model could be selected"
        );
    }

    const std::uint16_t modelId = *selectedModelId;

    eld::graphics::GraphicsResources resources(
        modelRepository,
        textureRepository
    );
    const eld::graphics::ModelHandle modelHandle =
        resources.resolveModel(modelId);

    eld::platform::SdlOpenGLContext context(
        "Eldoria GPU Render Probe",
        900,
        700,
        3,
        3
    );

    if (!context.valid()) {
        return 1;
    }

    context.setSwapInterval(1);

    eld::render::OpenGLRenderBackend backend(
        context.window()
    );

    std::cout
        << "GPU renderer: " << backend.rendererName()
        << "\nOpenGL: " << backend.versionString()
        << "\nmodel: " << modelId
        << "\n";

    const eld::model::ModelMesh sourceMesh =
        modelRepository.getMesh(modelId);

    float modelExtent = 1.0f;
    for (const eld::model::Vertex& vertex : sourceMesh.vertices) {
        modelExtent = std::max(
            modelExtent,
            std::max({
                std::abs(static_cast<float>(vertex.x)),
                std::abs(static_cast<float>(vertex.y)),
                std::abs(static_cast<float>(vertex.z))
            })
        );
    }

    const float initialDistance =
        -std::max(300.0f, modelExtent * 4.0f);

    eld::render::RenderScene scene;
    scene.camera.position = {0.0f, 0.0f, initialDistance};
    scene.camera.nearPlane = 1.0f;
    scene.camera.farPlane =
        std::max(10000.0f, modelExtent * 20.0f);

    eld::render::RenderObject object;
    object.model = modelHandle;
    scene.objects.push_back(object);

    eld::render::RenderPipeline pipeline;

    float yaw = 0.0f;
    float pitch = 0.0f;
    float distance = initialDistance;
    bool running = true;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                continue;
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                switch (event.key.key) {
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    case SDLK_LEFT:
                        yaw -= 0.08f;
                        break;
                    case SDLK_RIGHT:
                        yaw += 0.08f;
                        break;
                    case SDLK_UP:
                        pitch -= 0.08f;
                        break;
                    case SDLK_DOWN:
                        pitch += 0.08f;
                        break;
                    case SDLK_EQUALS:
                    case SDLK_PLUS:
                        distance = std::min(distance + 25.0f, -25.0f);
                        break;
                    case SDLK_MINUS:
                        distance = std::max(distance - 25.0f, -5000.0f);
                        break;
                    default:
                        break;
                }
            }

            if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                distance = std::clamp(
                    distance + event.wheel.y * 30.0f,
                    -5000.0f,
                    -25.0f
                );
            }
        }

        int width = 1;
        int height = 1;
        if (!SDL_GetWindowSizeInPixels(context.window(), &width, &height)) {
            width = 1;
            height = 1;
        }

        scene.camera.viewportWidth =
            static_cast<std::uint32_t>(std::max(width, 1));
        scene.camera.viewportHeight =
            static_cast<std::uint32_t>(std::max(height, 1));
        scene.camera.position.z = distance;
        scene.objects.front().transform.rotation = {pitch, yaw, 0.0f};

        pipeline.render(scene, resources, backend);
    }

    const eld::render::OpenGLBackendStats& stats = backend.stats();
    std::cout
        << "GPU cache: models=" << stats.uploadedModels
        << " meshes=" << stats.uploadedMeshes
        << " textures=" << stats.uploadedTextures
        << "\n";

    return 0;
}

int runMapProbe(
    const std::string& cacheRoot,
    std::uint16_t regionId
) {
    using Clock = std::chrono::steady_clock;

    eld::cache::Cache cache(cacheRoot);

    const eld::definition::DefinitionRepository definitions(
        cache.open(eld::cache::IndexId::Config),
        2
    );
    const eld::definition::FloorRepository floors(
        definitions.get("flo")
    );
    const eld::definition::LocationRepository locations(
        definitions.get("loc")
    );

    eld::model::ModelRepository modelRepository(
        cache.open(eld::cache::IndexId::Models)
    );
    eld::texture::TextureRepository textureRepository(
        cache.open(eld::cache::IndexId::Config)
    );
    eld::graphics::GraphicsResources resources(
        modelRepository,
        textureRepository
    );

    const eld::map::MapLoader loader(cache);
    const eld::map::MapIndexEntry* indexEntry =
        loader.find(regionId);

    if (indexEntry == nullptr) {
        throw std::runtime_error(
            "map loader has no region " +
            std::to_string(regionId)
        );
    }

    std::cout
        << "GPU MAP SCENE BUILD\n"
        << "===================\n"
        << "region: " << regionId
        << " (" << indexEntry->regionX()
        << "," << indexEntry->regionY() << ")\n"
        << "loading terrain neighborhood...\n";

    const auto buildStart = Clock::now();

    const TerrainNeighborhood neighborhood =
        loadTerrainNeighborhood(loader, *indexEntry);
    const eld::map::MapRegion* centerTerrain =
        neighborhoodTerrain(neighborhood, 0, 0);

    if (centerTerrain == nullptr) {
        throw std::runtime_error(
            "center terrain is missing"
        );
    }

    const eld::map::MapRegion centerRegion =
        loader.load(regionId);

    const eld::graphics::map::TerrainTileSampler terrainSampler =
        [&](std::size_t plane, int x, int y) {
            return sampleTerrainNeighborhood(
                neighborhood,
                plane,
                x,
                y
            );
        };

    const eld::graphics::map::SceneLocTileSampler locSampler =
        [&](std::size_t plane, int x, int y) {
            return sampleTerrainNeighborhood(
                neighborhood,
                plane,
                x,
                y
            );
        };

    eld::graphics::map::SceneLocBuilder locBuilder;
    const std::vector<eld::graphics::map::SceneLocPlacement> sceneLocs =
        locBuilder.build(
            centerRegion.objects,
            locations,
            locSampler
        );

    eld::graphics::map::SceneLocModelBuilder locModelBuilder;
    const eld::graphics::map::SceneLocModelBuildResult locModels =
        locModelBuilder.build(
            sceneLocs,
            locations,
            modelRepository
        );

    std::vector<eld::graphics::ModelHandle> variantHandles;
    variantHandles.reserve(locModels.variants.size());

    for (const eld::graphics::map::SceneLocModelVariant& variant :
         locModels.variants) {
        variantHandles.push_back(
            resources.resolveModel(variant.mesh)
        );
    }

    eld::graphics::map::SceneMapRenderModelBuilder renderModelBuilder;

    std::array<eld::graphics::map::SceneTerrainRenderBuildResult,
               eld::map::PlaneCount>
        terrainBuilds;

    for (std::size_t plane = 0; plane < eld::map::PlaneCount; ++plane) {
        std::cout
            << "building GPU terrain plane " << plane << "...\n";
        terrainBuilds[plane] =
            renderModelBuilder.buildTerrainPlane(
                plane,
                terrainSampler,
                floors,
                resources
            );
    }

    std::cout << "batching loc geometry...\n";
    eld::graphics::map::SceneLocRenderBuildResult locRender =
        renderModelBuilder.buildLocs(
            locModels,
            variantHandles,
            resources
        );

    std::array<eld::graphics::ModelHandle, eld::map::PlaneCount>
        terrainHandles;
    std::array<eld::graphics::ModelHandle, eld::map::PlaneCount>
        locHandles;

    for (std::size_t plane = 0; plane < eld::map::PlaneCount; ++plane) {
        terrainHandles[plane] = resources.registerModel(
            std::move(terrainBuilds[plane].model)
        );
        locHandles[plane] = resources.registerModel(
            std::move(locRender.staticPlaneModels[plane])
        );
    }

    eld::render::RenderScene scene;
    scene.camera.verticalFov = 0.96f;
    scene.camera.nearPlane = 0.5f;
    scene.camera.farPlane = 512.0f;

    const eld::render::Transform mapTransform =
        classicMapTransform();

    std::array<std::size_t, eld::map::PlaneCount> terrainObjectIndices{};
    std::array<std::size_t, eld::map::PlaneCount> locObjectIndices{};

    for (std::size_t plane = 0; plane < eld::map::PlaneCount; ++plane) {
        terrainObjectIndices[plane] = scene.objects.size();
        scene.objects.push_back({
            terrainHandles[plane],
            mapTransform,
            false
        });

        locObjectIndices[plane] = scene.objects.size();
        scene.objects.push_back({
            locHandles[plane],
            mapTransform,
            false
        });
    }

    std::vector<GpuCameraLoc> cameraLocs;
    cameraLocs.reserve(locRender.cameraVariants.size());

    for (eld::graphics::map::SceneLocCameraRenderVariant& variant :
         locRender.cameraVariants) {
        const eld::graphics::ModelHandle insetHandle =
            resources.registerModel(
                std::move(variant.insetModel)
            );
        const eld::graphics::ModelHandle outsetHandle =
            resources.registerModel(
                std::move(variant.outsetModel)
            );

        const std::size_t insetObject = scene.objects.size();
        scene.objects.push_back({
            insetHandle,
            mapTransform,
            false
        });

        const std::size_t outsetObject = scene.objects.size();
        scene.objects.push_back({
            outsetHandle,
            mapTransform,
            false
        });

        cameraLocs.push_back({
            variant.scenePlane,
            variant.rotation,
            variant.sceneX,
            variant.sceneZ,
            insetObject,
            outsetObject
        });
    }

    const double buildMs =
        std::chrono::duration<double, std::milli>(
            Clock::now() - buildStart
        ).count();

    std::size_t terrainTriangles = 0;
    std::size_t terrainBuckets = 0;
    for (const auto& build : terrainBuilds) {
        terrainTriangles += build.stats.triangles;
        terrainBuckets += build.stats.drawBuckets;
    }

    std::cout
        << std::fixed << std::setprecision(2)
        << "build time: " << buildMs << " ms\n"
        << "neighborhood: " << neighborhood.regions.size() << "/9\n"
        << "loc placements: " << sceneLocs.size() << "\n"
        << "loc model instances: " << locModels.stats.instances
        << " parts=" << locModels.stats.parts
        << " variants=" << locModels.stats.variants << "\n"
        << "terrain triangles (all planes): " << terrainTriangles
        << " buckets=" << terrainBuckets << "\n"
        << "loc triangles (all planes): " << locRender.stats.triangles
        << " static buckets=" << locRender.stats.staticDrawBuckets
        << " camera-dependent=" << locRender.stats.cameraDependentParts
        << "\n";

    if (!neighborhood.missingRegionIds.empty()) {
        std::cout << "missing neighbor regions:";
        for (const std::uint16_t missing : neighborhood.missingRegionIds) {
            std::cout << ' ' << missing;
        }
        std::cout << "\n";
    }

    eld::platform::SdlOpenGLContext context(
        "Eldoria GPU Map Probe",
        1100,
        800,
        3,
        3
    );

    if (!context.valid()) {
        return 1;
    }

    bool vsync = true;
    context.setSwapInterval(1);

    eld::render::OpenGLRenderBackend backend(
        context.window()
    );
    eld::render::RenderPipeline pipeline;

    std::cout
        << "GPU renderer: " << backend.rendererName()
        << "\nOpenGL: " << backend.versionString()
        << "\n\nControls\n"
        << "--------\n"
        << "1 / 2 / 3 / 4 : scene plane\n"
        << "Left / Right   : orbit\n"
        << "Up / Down      : pitch\n"
        << "+ / -          : zoom\n"
        << "Mouse wheel    : zoom\n"
        << "T              : terrain on/off\n"
        << "M              : loc models on/off\n"
        << "V              : vsync on/off (off = raw benchmark)\n"
        << "R              : reset camera\n"
        << "Esc            : quit\n\n";

    std::size_t plane = 0;
    bool showTerrain = true;
    bool showLocs = true;
    float yaw = 0.75f;
    float pitch = 0.62f;
    float distance = 82.0f;

    bool running = true;

    auto sampleStart = Clock::now();
    std::size_t sampleFrames = 0;
    double fps = 0.0;
    double frameMs = 0.0;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                continue;
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                switch (event.key.scancode) {
                    case SDL_SCANCODE_ESCAPE:
                        running = false;
                        break;
                    case SDL_SCANCODE_1:
                        plane = 0;
                        break;
                    case SDL_SCANCODE_2:
                        plane = 1;
                        break;
                    case SDL_SCANCODE_3:
                        plane = 2;
                        break;
                    case SDL_SCANCODE_4:
                        plane = 3;
                        break;
                    case SDL_SCANCODE_LEFT:
                        yaw -= 0.08f;
                        break;
                    case SDL_SCANCODE_RIGHT:
                        yaw += 0.08f;
                        break;
                    case SDL_SCANCODE_UP:
                        pitch = std::clamp(
                            pitch + 0.05f,
                            0.12f,
                            1.35f
                        );
                        break;
                    case SDL_SCANCODE_DOWN:
                        pitch = std::clamp(
                            pitch - 0.05f,
                            0.12f,
                            1.35f
                        );
                        break;
                    case SDL_SCANCODE_EQUALS:
                    case SDL_SCANCODE_KP_PLUS:
                        distance = std::max(24.0f, distance - 4.0f);
                        break;
                    case SDL_SCANCODE_MINUS:
                    case SDL_SCANCODE_KP_MINUS:
                        distance = std::min(180.0f, distance + 4.0f);
                        break;
                    case SDL_SCANCODE_T:
                        showTerrain = !showTerrain;
                        break;
                    case SDL_SCANCODE_M:
                        showLocs = !showLocs;
                        break;
                    case SDL_SCANCODE_V:
                        vsync = !vsync;
                        context.setSwapInterval(vsync ? 1 : 0);
                        sampleStart = Clock::now();
                        sampleFrames = 0;
                        fps = 0.0;
                        frameMs = 0.0;
                        break;
                    case SDL_SCANCODE_R:
                        yaw = 0.75f;
                        pitch = 0.62f;
                        distance = 82.0f;
                        break;
                    default:
                        break;
                }
            }

            if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                distance = std::clamp(
                    distance - event.wheel.y * 4.0f,
                    24.0f,
                    180.0f
                );
            }
        }

        int width = 1;
        int height = 1;
        if (!SDL_GetWindowSizeInPixels(context.window(), &width, &height)) {
            width = 1;
            height = 1;
        }

        scene.camera.viewportWidth =
            static_cast<std::uint32_t>(std::max(width, 1));
        scene.camera.viewportHeight =
            static_cast<std::uint32_t>(std::max(height, 1));

        const float targetY =
            averagePlaneHeight(*centerTerrain, plane);
        const eld::math::Vec3 target{
            0.0f,
            targetY,
            0.0f
        };
        const float horizontalDistance =
            std::cos(pitch) * distance;

        scene.camera.position = {
            target.x + std::sin(yaw) * horizontalDistance,
            target.y + std::sin(pitch) * distance,
            target.z + std::cos(yaw) * horizontalDistance
        };
        scene.camera.rotation = {
            pitch,
            yaw + Pi,
            0.0f
        };

        for (std::size_t p = 0; p < eld::map::PlaneCount; ++p) {
            scene.objects[terrainObjectIndices[p]].visible =
                showTerrain && p == plane;
            scene.objects[locObjectIndices[p]].visible =
                showLocs && p == plane;
        }

        for (const GpuCameraLoc& loc : cameraLocs) {
            const bool onPlane =
                showLocs && loc.scenePlane == plane;
            const bool useInset =
                onPlane && diagonalDecorationUsesInset(
                    loc.rotation,
                    loc.sceneX,
                    loc.sceneZ,
                    scene.camera.position
                );

            scene.objects[loc.insetObject].visible =
                onPlane && useInset;
            scene.objects[loc.outsetObject].visible =
                onPlane && !useInset;
        }

        pipeline.render(scene, resources, backend);

        ++sampleFrames;
        const auto now = Clock::now();
        const double seconds =
            std::chrono::duration<double>(now - sampleStart).count();

        if (seconds >= 0.5) {
            fps =
                static_cast<double>(sampleFrames) / seconds;
            frameMs =
                seconds * 1000.0 /
                static_cast<double>(sampleFrames);

            sampleStart = now;
            sampleFrames = 0;

            std::ostringstream title;
            title
                << "Eldoria GPU Map | region " << regionId
                << " | plane " << plane
                << " | " << std::fixed << std::setprecision(1)
                << fps << " FPS / " << frameMs << " ms"
                << " | draws " << backend.stats().drawCalls
                << " | vsync " << (vsync ? "on" : "off");

            SDL_SetWindowTitle(
                context.window(),
                title.str().c_str()
            );
        }
    }

    const eld::render::OpenGLBackendStats& stats = backend.stats();
    std::cout
        << "GPU cache: models=" << stats.uploadedModels
        << " meshes=" << stats.uploadedMeshes
        << " textures=" << stats.uploadedTextures
        << " last-frame-draws=" << stats.drawCalls
        << "\n";

    return 0;
}

void printUsage() {
    std::cerr
        << "Usage:\n"
        << "  gpu_probe <cache-root> [model-id]\n"
        << "  gpu_probe <cache-root> model [model-id]\n"
        << "  gpu_probe <cache-root> map <region-id>\n";
}

}

int main(
    int argc,
    char** argv
) {
    try {
        if (argc < 2 || argc > 4) {
            printUsage();
            return 2;
        }

        const std::string cacheRoot = argv[1];

        if (argc >= 3 && std::string(argv[2]) == "map") {
            if (argc != 4) {
                printUsage();
                return 2;
            }

            return runMapProbe(
                cacheRoot,
                parseU16(argv[3], "region id")
            );
        }

        if (argc >= 3 && std::string(argv[2]) == "model") {
            if (argc == 4) {
                return runModelProbe(
                    cacheRoot,
                    parseU16(argv[3], "model id")
                );
            }

            return runModelProbe(cacheRoot, std::nullopt);
        }

        if (argc == 3) {
            return runModelProbe(
                cacheRoot,
                parseU16(argv[2], "model id")
            );
        }

        return runModelProbe(cacheRoot, std::nullopt);
    }
    catch (const std::exception& e) {
        std::cerr
            << "gpu_probe failed: "
            << e.what()
            << "\n";
        return 1;
    }
}
