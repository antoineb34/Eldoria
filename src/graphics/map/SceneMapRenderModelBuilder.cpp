#include "SceneMapRenderModelBuilder.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "ClassicTileRules.h"
#include "SceneTerrainBuilder.h"

namespace eld::graphics::map {
namespace {

constexpr float TwoPi = 6.28318530717958647692f;

struct RenderBucket {
    eld::graphics::RenderMaterial material;
    float depthBias = 0.0f;
    eld::graphics::RenderMesh mesh;
};

bool sameVec4(
    const eld::math::Vec4& a,
    const eld::math::Vec4& b
) {
    return
        a.x == b.x &&
        a.y == b.y &&
        a.z == b.z &&
        a.w == b.w;
}

bool sameMaterial(
    const eld::graphics::RenderMaterial& a,
    const eld::graphics::RenderMaterial& b
) {
    return
        sameVec4(a.baseColor, b.baseColor) &&
        a.texture == b.texture &&
        a.sampler.filter == b.sampler.filter &&
        a.sampler.addressU == b.sampler.addressU &&
        a.sampler.addressV == b.sampler.addressV &&
        a.alphaMode == b.alphaMode &&
        a.doubleSided == b.doubleSided;
}

RenderBucket& findOrCreateBucket(
    std::vector<RenderBucket>& buckets,
    const eld::graphics::RenderMaterial& material,
    float depthBias
) {
    for (RenderBucket& bucket : buckets) {
        if (
            bucket.depthBias == depthBias &&
            sameMaterial(bucket.material, material)
        ) {
            return bucket;
        }
    }

    buckets.push_back(RenderBucket{
        material,
        depthBias,
        {}
    });

    return buckets.back();
}

void appendTriangle(
    RenderBucket& bucket,
    eld::graphics::RenderVertex a,
    eld::graphics::RenderVertex b,
    eld::graphics::RenderVertex c
) {
    const std::uint32_t first =
        static_cast<std::uint32_t>(
            bucket.mesh.vertices.size()
        );

    bucket.mesh.vertices.push_back(std::move(a));
    bucket.mesh.vertices.push_back(std::move(b));
    bucket.mesh.vertices.push_back(std::move(c));

    bucket.mesh.indices.push_back(first);
    bucket.mesh.indices.push_back(first + 1u);
    bucket.mesh.indices.push_back(first + 2u);
}

eld::graphics::RenderModel finishBuckets(
    std::vector<RenderBucket> buckets
) {
    eld::graphics::RenderModel model;
    model.materials.reserve(buckets.size());
    model.meshes.reserve(buckets.size());

    for (RenderBucket& bucket : buckets) {
        if (bucket.mesh.indices.empty()) {
            continue;
        }

        const std::uint32_t materialIndex =
            static_cast<std::uint32_t>(
                model.materials.size()
            );

        bucket.mesh.sections.push_back({
            0u,
            static_cast<std::uint32_t>(
                bucket.mesh.indices.size()
            ),
            materialIndex,
            bucket.depthBias
        });

        model.materials.push_back(
            std::move(bucket.material)
        );
        model.meshes.push_back(
            std::move(bucket.mesh)
        );
    }

    return model;
}

eld::math::Vec4 rgbColor(
    std::uint32_t rgb
) {
    constexpr float Scale = 1.0f / 255.0f;

    return {
        static_cast<float>((rgb >> 16) & 0xFFu) * Scale,
        static_cast<float>((rgb >> 8) & 0xFFu) * Scale,
        static_cast<float>(rgb & 0xFFu) * Scale,
        1.0f
    };
}

eld::math::Vec3 faceNormal(
    const eld::math::Vec3& a,
    const eld::math::Vec3& b,
    const eld::math::Vec3& c
) {
    const eld::math::Vec3 normal =
        (b - a).cross(c - a);
    const float length = normal.length();

    if (length <= 0.000001f) {
        return {0.0f, 1.0f, 0.0f};
    }

    return normal * (1.0f / length);
}

eld::graphics::RenderMaterial terrainMaterial(
    const std::optional<eld::graphics::TextureHandle>& texture
) {
    eld::graphics::RenderMaterial material;
    material.texture = texture;
    material.alphaMode = eld::graphics::AlphaMode::Opaque;

    // Terrain triangles are authored using the classic shaped-tile winding.
    // Keep them two-sided here; source-specific visibility/collision is not a
    // hardware back-face rule, and the baked classic lighting is already in
    // the vertex colors.
    material.doubleSided = true;
    material.sampler.filter = eld::graphics::TextureFilter::Nearest;
    material.sampler.addressU = eld::graphics::TextureAddressMode::Clamp;
    material.sampler.addressV = eld::graphics::TextureAddressMode::Clamp;
    return material;
}

struct SceneLocationDrawTransform {
    int sceneX = 0;
    int sceneZ = 0;
    int yaw = 0;
};

SceneLocationDrawTransform locDrawTransform(
    const SceneLocationModelInstance& instance,
    const SceneLocationModelPart& part,
    bool diagonalInset
) {
    SceneLocationDrawTransform result{
        instance.sceneX,
        instance.sceneZ,
        part.sceneYaw
    };

    if (part.drawMode == SceneLocationDrawMode::Standard) {
        return result;
    }

    constexpr std::array<int, 4> InsetX{53, -53, -53, 53};
    constexpr std::array<int, 4> InsetZ{-53, -53, 53, 53};
    constexpr std::array<int, 4> OutsetX{-45, 45, 45, -45};
    constexpr std::array<int, 4> OutsetZ{45, 45, -45, -45};

    const int rotation = instance.rotation & 3;
    bool useInset =
        part.drawMode == SceneLocationDrawMode::WallDecorationInset;

    if (
        part.drawMode ==
        SceneLocationDrawMode::WallDecorationDiagonalBoth
    ) {
        useInset = diagonalInset;
    }

    if (useInset) {
        result.sceneX += InsetX[rotation];
        result.sceneZ += InsetZ[rotation];
        result.yaw = rotation * 512 + 256;
    }
    else {
        result.sceneX += OutsetX[rotation];
        result.sceneZ += OutsetZ[rotation];
        result.yaw = (rotation * 512 + 1280) & 0x7FF;
    }

    return result;
}

eld::math::Vec3 rotateYaw(
    const eld::math::Vec3& value,
    int yaw
) {
    const float angle =
        static_cast<float>(yaw & 0x7FF) *
        TwoPi / 2048.0f;
    const float sine = std::sin(angle);
    const float cosine = std::cos(angle);

    return {
        value.x * cosine + value.z * sine,
        value.y,
        value.z * cosine - value.x * sine
    };
}

float contourDelta(
    const SceneLocationModelInstance& instance,
    float modelX,
    float modelZ
) {
    const int southwest = instance.cornerHeights[0];
    const int southeast = instance.cornerHeights[1];
    const int northeast = instance.cornerHeights[2];
    const int northwest = instance.cornerHeights[3];

    const int x = static_cast<int>(modelX);
    const int z = static_cast<int>(modelZ);

    const int south =
        southwest +
        (southeast - southwest) * (x + 64) / 128;
    const int north =
        northwest +
        (northeast - northwest) * (x + 64) / 128;
    const int height =
        south + (north - south) * (z + 64) / 128;

    const int groundY =
        (southwest + southeast + northeast + northwest) / 4;

    return static_cast<float>(height - groundY);
}

eld::graphics::RenderVertex transformLocVertex(
    eld::graphics::RenderVertex vertex,
    const SceneLocationModelInstance& instance,
    const SceneLocationDrawTransform& draw
) {
    eld::math::Vec3 local = vertex.position;

    if (instance.contouredGround) {
        // RenderModelAssembler already changed source Y into graphics -Y.
        // Classic hillskew adds the terrain delta to source Y, so subtract it
        // from graphics Y before the scene yaw is applied.
        local.y -= contourDelta(
            instance,
            vertex.position.x,
            vertex.position.z
        );
    }

    local = rotateYaw(local, draw.yaw);

    vertex.position = {
        static_cast<float>(draw.sceneX) + local.x,
        -static_cast<float>(instance.sceneY) + local.y,
        static_cast<float>(draw.sceneZ) + local.z
    };

    vertex.normal = rotateYaw(vertex.normal, draw.yaw);
    return vertex;
}

std::size_t appendLocPart(
    std::vector<RenderBucket>& buckets,
    const SceneLocationModelInstance& instance,
    const SceneLocationModelPart& part,
    const SceneLocationDrawTransform& draw,
    const std::vector<eld::graphics::ModelHandle>& variantHandles,
    const eld::graphics::GraphicsResources& resources
) {
    if (part.variantIndex >= variantHandles.size()) {
        return 0;
    }

    const eld::graphics::RenderModel& model =
        resources.getModel(
            variantHandles[part.variantIndex]
        );

    std::size_t triangles = 0;

    for (const eld::graphics::RenderMesh& mesh : model.meshes) {
        for (
            const eld::graphics::RenderMeshSection& section :
            mesh.sections
        ) {
            if (
                section.materialIndex >= model.materials.size() ||
                section.firstIndex > mesh.indices.size() ||
                section.indexCount >
                    mesh.indices.size() - section.firstIndex
            ) {
                continue;
            }

            const eld::graphics::RenderMaterial& material =
                model.materials[section.materialIndex];
            RenderBucket& bucket =
                findOrCreateBucket(
                    buckets,
                    material,
                    section.depthBias
                );

            const std::size_t end =
                static_cast<std::size_t>(section.firstIndex) +
                static_cast<std::size_t>(section.indexCount);

            for (
                std::size_t index = section.firstIndex;
                index + 2 < end;
                index += 3
            ) {
                const std::uint32_t ia = mesh.indices[index];
                const std::uint32_t ib = mesh.indices[index + 1];
                const std::uint32_t ic = mesh.indices[index + 2];

                if (
                    ia >= mesh.vertices.size() ||
                    ib >= mesh.vertices.size() ||
                    ic >= mesh.vertices.size()
                ) {
                    continue;
                }

                appendTriangle(
                    bucket,
                    transformLocVertex(
                        mesh.vertices[ia],
                        instance,
                        draw
                    ),
                    transformLocVertex(
                        mesh.vertices[ib],
                        instance,
                        draw
                    ),
                    transformLocVertex(
                        mesh.vertices[ic],
                        instance,
                        draw
                    )
                );

                ++triangles;
            }
        }
    }

    return triangles;
}

}

SceneTerrainRenderBuildResult
SceneMapRenderModelBuilder::buildTerrainPlane(
    std::size_t scenePlane,
    const TerrainTileSampler& sample,
    const eld::definition::FloorRepository& floors,
    eld::graphics::GraphicsResources& resources
) const {
    if (scenePlane >= eld::map::PlaneCount) {
        throw std::out_of_range(
            "scene terrain plane is outside 0..3"
        );
    }

    SceneTerrainRenderBuildResult result;
    std::vector<RenderBucket> buckets;
    SceneTerrainBuilder terrainBuilder;
    ClassicTerrainAppearanceBuilder appearanceBuilder;

    const auto appendSourceTile = [&] (
        std::size_t sourcePlane,
        int tileX,
        int tileY
    ) {
        const eld::map::MapTile* sw =
            sample(sourcePlane, tileX, tileY);
        const eld::map::MapTile* se =
            sample(sourcePlane, tileX + 1, tileY);
        const eld::map::MapTile* ne =
            sample(sourcePlane, tileX + 1, tileY + 1);
        const eld::map::MapTile* nw =
            sample(sourcePlane, tileX, tileY + 1);

        if (
            sw == nullptr || se == nullptr ||
            ne == nullptr || nw == nullptr
        ) {
            return;
        }

        const SceneTileAppearance appearance =
            appearanceBuilder.build(
                sourcePlane,
                tileX,
                tileY,
                sample,
                floors
            );

        if (
            !appearance.underlayVisible &&
            !appearance.overlayVisible
        ) {
            return;
        }

        std::uint8_t sceneShape = 0;
        std::uint8_t rotation = 0;

        if (sw->overlayId != 0) {
            if (sw->overlayShape >= 12) {
                return;
            }

            sceneShape =
                static_cast<std::uint8_t>(
                    sw->overlayShape + 1u
                );
            rotation =
                static_cast<std::uint8_t>(
                    sw->overlayRotation & 3u
                );
        }

        const SceneTileMesh tile =
            terrainBuilder.buildTile(
                tileX,
                tileY,
                sceneShape,
                rotation,
                TerrainCornerHeights{
                    sw->height,
                    se->height,
                    ne->height,
                    nw->height
                },
                appearance.shades
            );

        ++result.stats.tiles;

        for (const SceneTerrainTriangle& triangle : tile.triangles) {
            if (
                triangle.surface == TerrainSurface::Underlay &&
                !appearance.underlayVisible
            ) {
                continue;
            }

            if (
                triangle.surface == TerrainSurface::Overlay &&
                !appearance.overlayVisible
            ) {
                continue;
            }

            const bool textured =
                triangle.surface == TerrainSurface::Overlay &&
                appearance.textureId.has_value();

            std::optional<eld::graphics::TextureHandle> texture;
            if (textured) {
                texture = resources.resolveTexture(
                    static_cast<std::uint16_t>(
                        *appearance.textureId
                    )
                );
            }

            RenderBucket& bucket =
                findOrCreateBucket(
                    buckets,
                    terrainMaterial(texture),
                    0.0f
                );

            std::array<eld::graphics::RenderVertex, 3> vertices;

            for (std::size_t corner = 0; corner < 3; ++corner) {
                const std::uint8_t vertexIndex =
                    triangle.indices[corner];
                if (vertexIndex >= tile.vertices.size()) {
                    throw std::runtime_error(
                        "scene terrain triangle index is invalid"
                    );
                }

                const SceneTerrainVertex& source =
                    tile.vertices[vertexIndex];
                const int shade =
                    triangle.surface == TerrainSurface::Overlay
                        ? source.overlayShade
                        : source.underlayShade;

                const std::uint32_t rgb =
                    textured
                        ? ClassicTerrainAppearanceBuilder::
                              textureModulationRgb(shade)
                        : ClassicTerrainAppearanceBuilder::
                              paletteRgb(shade);

                vertices[corner].position = {
                    static_cast<float>(source.x),
                    -static_cast<float>(source.y),
                    static_cast<float>(source.z)
                };
                vertices[corner].uv = {
                    textured
                        ? triangle.textureUvs[corner].u
                        : 0.0f,
                    textured
                        ? triangle.textureUvs[corner].v
                        : 0.0f
                };
                vertices[corner].color = rgbColor(rgb);
            }

            const eld::math::Vec3 normal =
                faceNormal(
                    vertices[0].position,
                    vertices[1].position,
                    vertices[2].position
                );
            vertices[0].normal = normal;
            vertices[1].normal = normal;
            vertices[2].normal = normal;

            appendTriangle(
                bucket,
                vertices[0],
                vertices[1],
                vertices[2]
            );

            ++result.stats.triangles;
            if (textured) {
                ++result.stats.texturedTriangles;
            }
        }
    };

    for (int x = 0; x < static_cast<int>(eld::map::RegionSize); ++x) {
        for (int y = 0; y < static_cast<int>(eld::map::RegionSize); ++y) {
            const eld::map::MapTile* levelOne = sample(1, x, y);
            const std::uint8_t levelOneSettings =
                levelOne == nullptr ? 0 : levelOne->settings;

            if (
                ClassicTileRules::hasBridgeGround(
                    scenePlane,
                    levelOneSettings
                )
            ) {
                appendSourceTile(0, x, y);
            }

            const std::optional<std::size_t> sourcePlane =
                ClassicTileRules::sourcePlaneForScenePlane(
                    scenePlane,
                    levelOneSettings
                );

            if (sourcePlane.has_value()) {
                appendSourceTile(*sourcePlane, x, y);
            }
        }
    }

    result.stats.drawBuckets = buckets.size();
    result.model = finishBuckets(std::move(buckets));
    return result;
}

SceneLocationRenderBuildResult
SceneMapRenderModelBuilder::buildLocs(
    const SceneLocationModelBuildResult& locModels,
    const std::vector<eld::graphics::ModelHandle>& variantHandles,
    const eld::graphics::GraphicsResources& resources
) const {
    SceneLocationRenderBuildResult result;
    std::array<std::vector<RenderBucket>, eld::map::PlaneCount>
        planeBuckets;

    result.cameraVariants.reserve(32);

    for (const SceneLocationModelInstance& instance : locModels.instances) {
        if (instance.scenePlane >= eld::map::PlaneCount) {
            continue;
        }

        ++result.stats.instances;

        for (const SceneLocationModelPart& part : instance.parts) {
            ++result.stats.parts;

            if (
                part.drawMode ==
                SceneLocationDrawMode::WallDecorationDiagonalBoth
            ) {
                std::vector<RenderBucket> insetBuckets;
                std::vector<RenderBucket> outsetBuckets;

                result.stats.triangles +=
                    appendLocPart(
                        insetBuckets,
                        instance,
                        part,
                        locDrawTransform(instance, part, true),
                        variantHandles,
                        resources
                    );

                appendLocPart(
                    outsetBuckets,
                    instance,
                    part,
                    locDrawTransform(instance, part, false),
                    variantHandles,
                    resources
                );

                SceneLocationCameraRenderVariant variant;
                variant.scenePlane = instance.scenePlane;
                variant.rotation = instance.rotation;
                variant.sceneX = instance.sceneX;
                variant.sceneZ = instance.sceneZ;
                variant.insetModel =
                    finishBuckets(std::move(insetBuckets));
                variant.outsetModel =
                    finishBuckets(std::move(outsetBuckets));

                result.cameraVariants.push_back(
                    std::move(variant)
                );
                ++result.stats.cameraDependentParts;
                continue;
            }

            result.stats.triangles +=
                appendLocPart(
                    planeBuckets[instance.scenePlane],
                    instance,
                    part,
                    locDrawTransform(instance, part, true),
                    variantHandles,
                    resources
                );
        }
    }

    for (std::size_t plane = 0; plane < eld::map::PlaneCount; ++plane) {
        result.stats.staticDrawBuckets +=
            planeBuckets[plane].size();
        result.staticPlaneModels[plane] =
            finishBuckets(
                std::move(planeBuckets[plane])
            );
    }

    return result;
}

}
