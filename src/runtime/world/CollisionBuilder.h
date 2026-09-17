#pragma once

#include <algorithm>

#include "CollisionMap.h"
#include "Location.h"
#include "Terrain.h"

namespace eld::world
{

    class CollisionBuilder
    {
    public:
        static CollisionMap build(
            const Terrain& terrain,
            const std::vector<Location>& locations
        )
        {
            CollisionMap collision(
                terrain.origin(),
                terrain.width(),
                terrain.height(),
                terrain.sourcePlaneCount()
            );


            // ------------------------------------------------
            // Terrain clipping.
            // ------------------------------------------------

            for (
                std::size_t sourcePlane = 0;
                sourcePlane <
                    terrain.sourcePlaneCount();
                ++sourcePlane
            ) {
                for (
                    std::size_t x = 0;
                    x < terrain.width();
                    ++x
                ) {
                    for (
                        std::size_t y = 0;
                        y < terrain.height();
                        ++y
                    ) {
                        const auto position =
                            terrain.layerPosition(
                                sourcePlane,
                                x,
                                y
                            );

                        if (
                            !terrain.contains(
                                position
                            )
                        ) {
                            continue;
                        }


                        const auto& tile =
                            terrain.tile(
                                position
                            );


                        if (!tile.flags.solid) {
                            continue;
                        }


                        collision.blockTile({
                            position.x,
                            position.y,
                            tile.scenePlane
                        });
                    }
                }
            }


            // ------------------------------------------------
            // Location clipping.
            // ------------------------------------------------

            for (
                const auto& location :
                locations
            ) {
                if (!location.solid) {
                    continue;
                }


                const int rotation =
                    static_cast<int>(
                        location.rotation
                    ) & 3;


                // --------------------------------------------
                // Straight wall.
                // --------------------------------------------

                if (location.shape == 0) {
                    switch (rotation) {
                    case 0:
                        collision.blockEdge(
                            location.tile,
                            -1,
                            0
                        );
                        break;

                    case 1:
                        collision.blockEdge(
                            location.tile,
                            0,
                            1
                        );
                        break;

                    case 2:
                        collision.blockEdge(
                            location.tile,
                            1,
                            0
                        );
                        break;

                    case 3:
                        collision.blockEdge(
                            location.tile,
                            0,
                            -1
                        );
                        break;
                    }

                    continue;
                }


                // --------------------------------------------
                // Diagonal wall.
                //
                // Shapes 1 and 3 use diagonal clipping.
                // --------------------------------------------

                if (
                    location.shape == 1 ||
                    location.shape == 3
                ) {
                    switch (rotation) {
                    case 0:
                        collision.blockEdge(
                            location.tile,
                            -1,
                            1
                        );
                        break;

                    case 1:
                        collision.blockEdge(
                            location.tile,
                            1,
                            1
                        );
                        break;

                    case 2:
                        collision.blockEdge(
                            location.tile,
                            1,
                            -1
                        );
                        break;

                    case 3:
                        collision.blockEdge(
                            location.tile,
                            -1,
                            -1
                        );
                        break;
                    }

                    continue;
                }


                // --------------------------------------------
                // Corner / L wall.
                // --------------------------------------------

                if (location.shape == 2) {
                    switch (rotation) {
                    case 0:
                        collision.blockEdge(
                            location.tile,
                            -1,
                            0
                        );

                        collision.blockEdge(
                            location.tile,
                            0,
                            1
                        );
                        break;

                    case 1:
                        collision.blockEdge(
                            location.tile,
                            0,
                            1
                        );

                        collision.blockEdge(
                            location.tile,
                            1,
                            0
                        );
                        break;

                    case 2:
                        collision.blockEdge(
                            location.tile,
                            1,
                            0
                        );

                        collision.blockEdge(
                            location.tile,
                            0,
                            -1
                        );
                        break;

                    case 3:
                        collision.blockEdge(
                            location.tile,
                            0,
                            -1
                        );

                        collision.blockEdge(
                            location.tile,
                            -1,
                            0
                        );
                        break;
                    }

                    continue;
                }


                // Wall decorations 4..8 do not occupy a
                // walking tile themselves.
                if (
                    location.shape >= 4 &&
                    location.shape <= 8
                ) {
                    continue;
                }


                // Type 22 is floor decoration. Classic clients
                // apply extra rules depending on interaction
                // metadata, so leave it alone for this first
                // movement-collision pass.
                if (location.shape == 22) {
                    continue;
                }


                // Type 9 and normal game objects occupy tiles.
                if (
                    location.shape == 9 ||
                    (
                        location.shape >= 10 &&
                        location.shape <= 21
                    )
                ) {
                    const int width =
                        std::max(
                            1,
                            location.footprintWidth
                        );

                    const int length =
                        std::max(
                            1,
                            location.footprintLength
                        );


                    for (
                        int dx = 0;
                        dx < width;
                        ++dx
                    ) {
                        for (
                            int dy = 0;
                            dy < length;
                            ++dy
                        ) {
                            collision.blockTile({
                                location.tile.x + dx,
                                location.tile.y + dy,
                                location.tile.plane
                            });
                        }
                    }
                }
            }


            return collision;
        }
    };

}
