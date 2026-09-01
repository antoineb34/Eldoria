#include "MapPreview.h"

#include <algorithm>
#include <cmath>

namespace eld::elforge {

namespace {

constexpr float Pi = 3.14159265358979323846f;
constexpr float MapTileUnits = 128.0f;
constexpr float MapCenterTiles =
    static_cast<float>(eld::map::RegionSize) * 0.5f;

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

}

void resetMapView(
    std::size_t& plane,
    float& yaw,
    float& pitch,
    float& distance
) {
    plane = 0;
    yaw = 0.75f;
    pitch = 0.62f;
    distance = 82.0f;
}

void updateMapPreviewScene(
    MapPreview& preview,
    std::size_t plane,
    bool showTerrain,
    bool showLocs,
    float yaw,
    float pitch,
    float distance,
    std::uint32_t viewportWidth,
    std::uint32_t viewportHeight
) {
    plane = std::min<std::size_t>(
        plane,
        eld::map::PlaneCount - 1
    );

    preview.scene.camera.viewportWidth =
        std::max<std::uint32_t>(viewportWidth, 1u);

    preview.scene.camera.viewportHeight =
        std::max<std::uint32_t>(viewportHeight, 1u);

    const eld::math::Vec3 target{
        0.0f,
        preview.averagePlaneHeights[plane],
        0.0f
    };

    const float horizontalDistance =
        std::cos(pitch) * distance;

    preview.scene.camera.position = {
        target.x + std::sin(yaw) * horizontalDistance,
        target.y + std::sin(pitch) * distance,
        target.z + std::cos(yaw) * horizontalDistance
    };

    preview.scene.camera.rotation = {
        pitch,
        yaw + Pi,
        0.0f
    };

    for (
        std::size_t currentPlane = 0;
        currentPlane < eld::map::PlaneCount;
        ++currentPlane
    ) {
        preview.scene.objects[
            preview.terrainObjectIndices[currentPlane]
        ].visible =
            showTerrain && currentPlane == plane;

        preview.scene.objects[
            preview.locObjectIndices[currentPlane]
        ].visible =
            showLocs && currentPlane == plane;
    }

    for (
        const MapCameraRenderVariant& variant :
        preview.cameraVariants
    ) {
        const bool onPlane =
            showLocs && variant.scenePlane == plane;

        const bool useInset =
            onPlane &&
            diagonalDecorationUsesInset(
                variant.rotation,
                variant.sceneX,
                variant.sceneZ,
                preview.scene.camera.position
            );

        preview.scene.objects[
            variant.insetObject
        ].visible =
            onPlane && useInset;

        preview.scene.objects[
            variant.outsetObject
        ].visible =
            onPlane && !useInset;
    }
}

}
