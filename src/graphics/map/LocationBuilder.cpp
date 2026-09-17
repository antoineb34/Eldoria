#include "LocationBuilder.h"
#include "LocationModelBuilder.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace eld::graphics {

namespace {

constexpr float ModelScale =
    1.0f / 128.0f;

constexpr float QuarterTurnRadians =
    1.57079632679f;

constexpr float EighthTurnRadians =
    0.78539816339f;


std::uint8_t modelTypeForShape(
    std::uint8_t shape
) {
    if (shape == 11) {
        return 10;
    }

    if (
        shape >= 5 &&
        shape <= 8
    ) {
        return 4;
    }

    return shape;
}


eld::render::Transform baseTransform(
    const eld::world::Region& region,
    const eld::world::Location& location
) {
    const auto origin =
        region.terrain.origin();


    eld::render::Transform transform;


    transform.position = {
        location.position.x -
            origin.x,

        location.position.y,

        location.position.z -
            origin.y
    };


    transform.scale = {
        ModelScale,
        ModelScale,
        ModelScale
    };


    return transform;
}


eld::render::RenderObject objectFor(
    eld::render::ModelHandle model,
    eld::render::Transform transform
) {
    eld::render::RenderObject object;

    object.model =
        model;

    object.transform =
        transform;

    object.visible =
        true;

    return object;
}


void applyYaw(
    eld::render::Transform& transform,
    float radians
) {
    transform.rotation.y +=
        radians;
}


void applySceneOffset(
    eld::render::Transform& transform,
    int sourceX,
    int sourceZ
) {
    transform.position.x +=
        static_cast<float>(
            sourceX
        ) /
        128.0f;

    transform.position.z +=
        static_cast<float>(
            sourceZ
        ) /
        128.0f;
}

}


LocationBuildResult LocationBuilder::build(
    const eld::world::Region& region,
    std::size_t scenePlane,

    const eld::location::LocationLoader& definitions,
    eld::model::ModelLoader& models,
    eld::graphics::ModelSystem& modelSystem
) const {
    LocationBuildResult result;


    LocationModelBuilder modelBuilder(
        models,
        modelSystem
    );


for (
        const auto& location :
        region.locations
    ) {
        if (
            location.tile.plane !=
            static_cast<int>(
                scenePlane
            )
        ) {
            continue;
        }


        ++result.locations;


        const auto definition =
            definitions.find(
                location.id
            );


        if (!definition.has_value()) {
            ++result.missingDefinitions;
            continue;
        }


        const int rotation =
            static_cast<int>(
                eld::world::quarterTurns(
                    location.rotation
                )
            );

        const auto shape =
            location.shape;


        // ----------------------------------------------------
        // L-shaped wall:
        // two separately rotated model pieces.
        // ----------------------------------------------------

        if (shape == 2) {
            const int nextRotation =
                (rotation + 1) & 3;


            const auto first =
                modelBuilder.build(
                    location,
                    *definition,
                    2,
                    rotation + 4
                );


            if (first.has_value()) {
                result.objects.push_back(
                    objectFor(
                        *first,
                        baseTransform(
                            region,
                            location
                        )
                    )
                );
            }


            const auto second =
                modelBuilder.build(
                    location,
                    *definition,
                    2,
                    nextRotation
                );


            if (second.has_value()) {
                result.objects.push_back(
                    objectFor(
                        *second,
                        baseTransform(
                            region,
                            location
                        )
                    )
                );
            }


            continue;
        }


        // ----------------------------------------------------
        // Wall decorations 4..8 all use model type 4,
        // rotation 0. Their visual orientation happens here.
        // ----------------------------------------------------

        if (
            shape >= 4 &&
            shape <= 8
        ) {
            const auto variant =
                modelBuilder.build(
                    location,
                    *definition,
                    4,
                    0
                );


            if (!variant.has_value()) {
                continue;
            }


            if (
                shape == 4 ||
                shape == 5
            ) {
                auto transform =
                    baseTransform(
                        region,
                        location
                    );


                applyYaw(
                    transform,
                    static_cast<float>(
                        rotation
                    ) *
                    QuarterTurnRadians
                );


                result.objects.push_back(
                    objectFor(
                        *variant,
                        transform
                    )
                );


                continue;
            }


            constexpr std::array<int, 4>
                InsetX{
                    53, -53, -53, 53
                };

            constexpr std::array<int, 4>
                InsetZ{
                    -53, -53, 53, 53
                };

            constexpr std::array<int, 4>
                OutsetX{
                    -45, 45, 45, -45
                };

            constexpr std::array<int, 4>
                OutsetZ{
                    45, 45, -45, -45
                };


            auto inset =
                baseTransform(
                    region,
                    location
                );

            applySceneOffset(
                inset,
                InsetX[rotation],
                InsetZ[rotation]
            );

            applyYaw(
                inset,
                static_cast<float>(
                    rotation
                ) *
                    QuarterTurnRadians +
                EighthTurnRadians
            );


            auto outset =
                baseTransform(
                    region,
                    location
                );

            applySceneOffset(
                outset,
                OutsetX[rotation],
                OutsetZ[rotation]
            );

            applyYaw(
                outset,
                static_cast<float>(
                    rotation
                ) *
                    QuarterTurnRadians +
                5.0f *
                    EighthTurnRadians
            );


            if (shape == 6) {
                result.objects.push_back(
                    objectFor(
                        *variant,
                        inset
                    )
                );
            }
            else if (shape == 7) {
                result.objects.push_back(
                    objectFor(
                        *variant,
                        outset
                    )
                );
            }
            else {
                CameraDependentLocation
                    cameraLocation;

                cameraLocation.scenePlane =
                    static_cast<std::uint8_t>(
                        scenePlane
                    );

                cameraLocation.inset =
                    objectFor(
                        *variant,
                        inset
                    );

                cameraLocation.outset =
                    objectFor(
                        *variant,
                        outset
                    );

                result.cameraDependent.push_back(
                    std::move(
                        cameraLocation
                    )
                );
            }


            continue;
        }


        // ----------------------------------------------------
        // Ordinary objects, walls, roofs, ground decorations.
        // ----------------------------------------------------

        const auto variant =
            modelBuilder.build(
                location,
                *definition,
                modelTypeForShape(
                    shape
                ),
                rotation
            );


        if (!variant.has_value()) {
            continue;
        }


        auto transform =
            baseTransform(
                region,
                location
            );


        // Classic shape 11 gets an additional 45 degree yaw.
        if (shape == 11) {
            applyYaw(
                transform,
                EighthTurnRadians
            );
        }


        result.objects.push_back(
            objectFor(
                *variant,
                transform
            )
        );
    }


    result.modelVariants =
        modelBuilder.createdVariants();

    result.missingModels =
        modelBuilder.missingModels();


    return result;
}

}
