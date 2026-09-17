#include "TerrainPicker.h"

#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>

#include "math/Vec3.h"


namespace eld::graphics
{

namespace
{

    struct Ray
    {
        eld::math::Vec3 origin;
        eld::math::Vec3 direction;
    };


    Ray buildCameraRay(
        const eld::render::Camera& camera,
        float mouseX,
        float mouseY
    )
    {
        const float width =
            static_cast<float>(
                camera.viewportWidth
            );

        const float height =
            static_cast<float>(
                camera.viewportHeight
            );


        const float ndcX =
            (
                mouseX /
                width
            ) *
                2.0f -
            1.0f;

        const float ndcY =
            1.0f -
            (
                mouseY /
                height
            ) *
                2.0f;


        const float aspect =
            width /
            height;


        const float tanHalfFov =
            std::tan(
                camera.verticalFov *
                0.5f
            );


        // ----------------------------------------------------
        // Match SimpleRenderer's camera basis exactly.
        //
        // yaw = 0, pitch = 0:
        //     forward = (0, 0, -1)
        // ----------------------------------------------------

        const float yaw =
            camera.rotation.y;

        const float pitch =
            camera.rotation.x;


        const float cyaw =
            std::cos(
                yaw
            );

        const float syaw =
            std::sin(
                yaw
            );

        const float cpitch =
            std::cos(
                pitch
            );

        const float spitch =
            std::sin(
                pitch
            );


        const eld::math::Vec3 forward{
            -syaw *
                cpitch,

            spitch,

            -cyaw *
                cpitch
        };


        const eld::math::Vec3 right{
            cyaw,
            0.0f,
            -syaw
        };


        const eld::math::Vec3 up{
            syaw *
                spitch,

            cpitch,

            cyaw *
                spitch
        };


        const float cameraX =
            ndcX *
            aspect *
            tanHalfFov;

        const float cameraY =
            ndcY *
            tanHalfFov;


        const eld::math::Vec3 direction =
            (
                right *
                    cameraX +
                up *
                    cameraY +
                forward
            ).normalized();


        return {
            camera.position,
            direction
        };
    }


    std::optional<float>
    intersectTriangle(
        const Ray& ray,
        const eld::math::Vec3& a,
        const eld::math::Vec3& b,
        const eld::math::Vec3& c
    )
    {
        // Möller-Trumbore ray/triangle intersection.
        // No back-face culling: terrain may be wound either
        // direction depending on its source geometry.

        constexpr float epsilon =
            0.000001f;


        const eld::math::Vec3 edge1 =
            b - a;

        const eld::math::Vec3 edge2 =
            c - a;


        const eld::math::Vec3 h =
            ray.direction.cross(
                edge2
            );


        const float determinant =
            edge1.dot(
                h
            );


        if (
            std::abs(
                determinant
            ) <
            epsilon
        ) {
            return std::nullopt;
        }


        const float inverseDeterminant =
            1.0f /
            determinant;


        const eld::math::Vec3 s =
            ray.origin -
            a;


        const float u =
            inverseDeterminant *
            s.dot(
                h
            );


        if (
            u < 0.0f ||
            u > 1.0f
        ) {
            return std::nullopt;
        }


        const eld::math::Vec3 q =
            s.cross(
                edge1
            );


        const float v =
            inverseDeterminant *
            ray.direction.dot(
                q
            );


        if (
            v < 0.0f ||
            u + v >
                1.0f
        ) {
            return std::nullopt;
        }


        const float distance =
            inverseDeterminant *
            edge2.dot(
                q
            );


        if (
            distance <=
            epsilon
        ) {
            return std::nullopt;
        }


        return distance;
    }

}


std::optional<
    eld::world::TilePosition
>
TerrainPicker::pick(
    const eld::world::World& world,
    const eld::render::Camera& camera,
    const eld::world::TerrainOrigin& sceneOrigin,
    float mouseX,
    float mouseY
) const
{
    if (
        camera.viewportWidth == 0 ||
        camera.viewportHeight == 0
    ) {
        return std::nullopt;
    }


    const Ray ray =
        buildCameraRay(
            camera,
            mouseX,
            mouseY
        );


    float nearestDistance =
        std::numeric_limits<
            float
        >::infinity();


    std::optional<
        eld::world::TilePosition
    > result;


    // --------------------------------------------------------
    // World owns all runtime Regions.
    //
    // Picking therefore queries the same World that movement,
    // collision and pathfinding query.
    // --------------------------------------------------------

    for (
        const auto regionId :
        world.loadedRegionIds()
    ) {
        const auto* region =
            world.region(
                regionId
            );


        if (!region) {
            continue;
        }


        const auto& terrain =
            region->terrain;


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


                // --------------------------------------------
                // Runtime world -> rendered scene space
                //
                // world X -> scene X
                // world Y -> -scene Z
                // terrain height -> scene Y
                // --------------------------------------------

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


                const eld::math::Vec3 southwest{
                    x0,
                    heights.southwest,
                    z0
                };

                const eld::math::Vec3 southeast{
                    x1,
                    heights.southeast,
                    z0
                };

                const eld::math::Vec3 northeast{
                    x1,
                    heights.northeast,
                    z1
                };

                const eld::math::Vec3 northwest{
                    x0,
                    heights.northwest,
                    z1
                };


                const auto consider =
                    [&](const std::optional<float>& hit)
                    {
                        if (
                            !hit.has_value() ||
                            *hit >=
                                nearestDistance
                        ) {
                            return;
                        }


                        nearestDistance =
                            *hit;


                        const auto& tile =
                            terrain.tile(
                                position
                            );


                        result =
                            eld::world::TilePosition{
                                position.x,
                                position.y,
                                static_cast<int>(
                                    tile.scenePlane
                                )
                            };
                    };


                consider(
                    intersectTriangle(
                        ray,
                        southwest,
                        southeast,
                        northeast
                    )
                );


                consider(
                    intersectTriangle(
                        ray,
                        southwest,
                        northeast,
                        northwest
                    )
                );
            }
        }
    }


    return result;
}

}
