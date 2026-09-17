#include "TerrainBuilder.h"

#include "ClassicTerrainShape.h"

#include <cmath>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace eld::graphics::terrain
{

    struct SurfaceAppearance
    {
        std::optional<
            eld::math::Vec4>
            color;

        std::optional<
            eld::render::TextureHandle>
            texture;
    };

    struct TileAppearance
    {
        SurfaceAppearance underlay;
        SurfaceAppearance overlay;

        int shape = 0;
        int rotation = 0;
    };

    std::optional<TileAppearance>
    resolveAppearance(
        const eld::world::Tile &tile,
        const eld::floor::FloorLoader &floors,
        eld::graphics::TextureSystem &textures);

}

namespace eld::graphics::terrain
{

    namespace
    {

        eld::math::Vec4 rgbToVec4(
            std::uint32_t rgb)
        {
            constexpr float scale =
                1.0f / 255.0f;

            return {
                static_cast<float>(
                    (rgb >> 16) &
                    0xFFu) *
                    scale,

                static_cast<float>(
                    (rgb >> 8) &
                    0xFFu) *
                    scale,

                static_cast<float>(
                    rgb &
                    0xFFu) *
                    scale,

                1.0f};
        }

        std::optional<eld::math::Vec4>
        floorColor(
            const eld::floor::FloorData *floor)
        {
            if (floor == nullptr)
            {
                return std::nullopt;
            }

            if (floor->rgb.has_value())
            {
                return rgbToVec4(
                    *floor->rgb);
            }

            if (
                floor->secondaryRgb.has_value())
            {
                return rgbToVec4(
                    *floor->secondaryRgb);
            }

            return std::nullopt;
        }

        const eld::floor::FloorData *
        floorFor(
            const std::optional<
                eld::world::FloorId> &floorId,

            const eld::floor::FloorLoader &floors)
        {
            if (!floorId.has_value())
            {
                return nullptr;
            }

            if (
                !floors.contains(
                    *floorId))
            {
                return nullptr;
            }

            return &floors.data(
                *floorId);
        }

        std::optional<
            eld::render::TextureHandle>
        textureFor(
            const eld::floor::FloorData *floor,
            eld::graphics::TextureSystem &textures)
        {
            if (
                floor == nullptr ||
                !floor->textureId.has_value())
            {
                return std::nullopt;
            }

            return textures.get(
                *floor->textureId);
        }

    }

    std::optional<TileAppearance>
    resolveAppearance(
        const eld::world::Tile &tile,
        const eld::floor::FloorLoader &floors,
        eld::graphics::TextureSystem &textures)
    {
        const auto *underlay =
            floorFor(
                tile.surface.underlay,
                floors);

        const auto *overlay =
            floorFor(
                tile.surface.overlay,
                floors);

        TileAppearance appearance;

        appearance.underlay.color =
            floorColor(
                underlay);

        appearance.underlay.texture =
            textureFor(
                underlay,
                textures);

        appearance.overlay.color =
            floorColor(
                overlay);

        appearance.overlay.texture =
            textureFor(
                overlay,
                textures);

        if (
            tile.surface.overlay
                .has_value())
        {
            if (
                tile.surface.shape >
                12)
            {
                return std::nullopt;
            }

            appearance.shape =
                static_cast<int>(
                    tile.surface.shape);

            appearance.rotation =
                static_cast<int>(
                    eld::world::quarterTurns(
                        tile.surface.rotation));
        }

        const bool visible =
            appearance.underlay.color
                .has_value() ||

            appearance.underlay.texture
                .has_value() ||

            appearance.overlay.color
                .has_value() ||

            appearance.overlay.texture
                .has_value();

        if (!visible)
        {
            return std::nullopt;
        }

        return appearance;
    }

}

namespace eld::graphics::terrain
{

    namespace
    {

        eld::render::RenderVertex
        vertexForPoint(
            const eld::world::Terrain &terrain,
            const eld::world::TerrainLayerPosition &position,
            int type)
        {
            const ClassicPoint &classic =
                point(type);

            const auto &origin =
                terrain.origin();

            eld::render::RenderVertex vertex{};

            // World coordinates are absolute.
            //
            // The render model itself is region-local, so Graphics
            // subtracts the terrain origin here.
            vertex.position = eld::math::Vec3{
                static_cast<float>(
                    position.x - origin.x) +
                    classic.local.x,

                terrain.heightAt(
                    position,
                    classic.local),

                static_cast<float>(
                    position.y - origin.y) +
                    classic.local.y};

            if (
                classic.heightRule !=
                HeightRule::Surface)
            {
                const auto heights =
                    terrain.cornerHeights(
                        position);

                const auto midpoint = [](
                                          float a,
                                          float b)
                {
                    return (a + b) * 0.5f;
                };

                switch (classic.heightRule)
                {
                case HeightRule::Surface:
                    break;

                case HeightRule::SouthEdge:
                    vertex.position.y =
                        midpoint(
                            heights.southwest,
                            heights.southeast);
                    break;

                case HeightRule::EastEdge:
                    vertex.position.y =
                        midpoint(
                            heights.southeast,
                            heights.northeast);
                    break;

                case HeightRule::NorthEdge:
                    vertex.position.y =
                        midpoint(
                            heights.northwest,
                            heights.northeast);
                    break;

                case HeightRule::WestEdge:
                    vertex.position.y =
                        midpoint(
                            heights.southwest,
                            heights.northwest);
                    break;

                case HeightRule::Southwest:
                    vertex.position.y =
                        heights.southwest;
                    break;

                case HeightRule::Southeast:
                    vertex.position.y =
                        heights.southeast;
                    break;

                case HeightRule::Northeast:
                    vertex.position.y =
                        heights.northeast;
                    break;

                case HeightRule::Northwest:
                    vertex.position.y =
                        heights.northwest;
                    break;
                }
            }

            return vertex;
        }

        std::uint32_t materialFor(
            eld::render::ModelResource &model,
            const std::optional<
                eld::render::TextureHandle> &texture)
        {
            for (
                std::size_t i = 0;
                i < model.materials.size();
                ++i)
            {
                if (
                    model.materials[i].texture ==
                    texture)
                {
                    return static_cast<std::uint32_t>(
                        i);
                }
            }

            eld::render::RenderMaterial material{};

            material.texture =
                texture;

            material.alphaMode =
                eld::render::AlphaMode::Opaque;

            material.doubleSided =
                true;

            material.sampler.filter =
                eld::render::TextureFilter::Nearest;

            material.sampler.addressU =
                eld::render::TextureAddressMode::Clamp;

            material.sampler.addressV =
                eld::render::TextureAddressMode::Clamp;

            model.materials.push_back(
                material);

            return static_cast<std::uint32_t>(
                model.materials.size() - 1);
        }

    }

    void appendTileGeometry(
        eld::render::ModelResource &model,
        eld::render::RenderMesh &mesh,

        const eld::world::Terrain &terrain,
        const eld::world::TerrainLayerPosition &position,

        const TileAppearance &appearance)
    {
        const int rotation =
            appearance.shape <= 1
                ? 0
                : appearance.rotation;

        const auto &points =
            pointPattern(
                appearance.shape);

        std::vector<
            eld::render::RenderVertex>
            vertices;

        vertices.reserve(
            points.size());

        for (int rawType : points)
        {
            const int type =
                rotatePointType(
                    rawType,
                    rotation);

            vertices.push_back(
                vertexForPoint(
                    terrain,
                    position,
                    type));
        }

        const auto &elements =
            elementPattern(
                appearance.shape);

        for (
            std::size_t i = 0;
            i < elements.size();
            i += 4)
        {
            const int layer =
                elements[i];

            int a =
                elements[i + 1];

            int b =
                elements[i + 2];

            int c =
                elements[i + 3];

            a = rotateTriangleIndex(
                a,
                rotation);

            b = rotateTriangleIndex(
                b,
                rotation);

            c = rotateTriangleIndex(
                c,
                rotation);

            const SurfaceAppearance &surface =
                layer == 1
                    ? appearance.overlay
                    : appearance.underlay;

            if (
                !surface.color.has_value() &&
                !surface.texture.has_value())
            {
                continue;
            }

            auto va =
                vertices.at(
                    static_cast<std::size_t>(a));

            auto vb =
                vertices.at(
                    static_cast<std::size_t>(b));

            auto vc =
                vertices.at(
                    static_cast<std::size_t>(c));

            // ---------------------------------------------
            // Texture mapping.
            // ---------------------------------------------

            if (surface.texture.has_value())
            {
                const auto heights =
                    terrain.cornerHeights(
                        position);

                const bool flat =
                    heights.southwest ==
                        heights.southeast &&
                    heights.southwest ==
                        heights.northeast &&
                    heights.southwest ==
                        heights.northwest;

                if (
                    appearance.shape == 1 ||
                    flat)
                {
                    const auto &origin =
                        vertices.at(0).position;

                    const auto &uPoint =
                        vertices.at(1).position;

                    const auto &vPoint =
                        vertices.at(3).position;

                    const double ux =
                        static_cast<double>(
                            uPoint.x -
                            origin.x);

                    const double uz =
                        static_cast<double>(
                            uPoint.z -
                            origin.z);

                    const double vx =
                        static_cast<double>(
                            vPoint.x -
                            origin.x);

                    const double vz =
                        static_cast<double>(
                            vPoint.z -
                            origin.z);

                    const double uu =
                        ux * ux +
                        uz * uz;

                    const double uv =
                        ux * vx +
                        uz * vz;

                    const double vv =
                        vx * vx +
                        vz * vz;

                    const double denominator =
                        uu * vv -
                        uv * uv;

                    if (
                        std::abs(
                            denominator) <= 0.000001)
                    {
                        throw std::runtime_error(
                            "Terrain texture basis is degenerate");
                    }

                    const auto projectUv =
                        [&](const eld::math::Vec3 &point)
                    {
                        const double rx =
                            static_cast<double>(
                                point.x -
                                origin.x);

                        const double rz =
                            static_cast<double>(
                                point.z -
                                origin.z);

                        const double ru =
                            rx * ux +
                            rz * uz;

                        const double rv =
                            rx * vx +
                            rz * vz;

                        return eld::math::Vec2{
                            static_cast<float>(
                                (
                                    vv * ru -
                                    uv * rv) /
                                denominator),

                            static_cast<float>(
                                (
                                    uu * rv -
                                    uv * ru) /
                                denominator)};
                    };

                    va.uv =
                        projectUv(
                            va.position);

                    vb.uv =
                        projectUv(
                            vb.position);

                    vc.uv =
                        projectUv(
                            vc.position);
                }
                else
                {
                    va.uv = {0.0f, 0.0f};
                    vb.uv = {1.0f, 0.0f};
                    vc.uv = {0.0f, 1.0f};
                }
            }

            // ---------------------------------------------
            // Normal.
            // ---------------------------------------------

            const auto edge1 =
                vb.position -
                va.position;

            const auto edge2 =
                vc.position -
                va.position;

            const auto normal =
                edge1
                    .cross(edge2)
                    .normalized();

            va.normal = normal;
            vb.normal = normal;
            vc.normal = normal;

            if (!surface.texture.has_value())
            {
                va.color =
                    *surface.color;

                vb.color =
                    *surface.color;

                vc.color =
                    *surface.color;
            }

            // ---------------------------------------------
            // Append triangle.
            // ---------------------------------------------

            const auto firstVertex =
                static_cast<std::uint32_t>(
                    mesh.vertices.size());

            const auto firstIndex =
                static_cast<std::uint32_t>(
                    mesh.indices.size());

            mesh.vertices.push_back(va);
            mesh.vertices.push_back(vb);
            mesh.vertices.push_back(vc);

            mesh.indices.push_back(
                firstVertex);

            mesh.indices.push_back(
                firstVertex + 1);

            mesh.indices.push_back(
                firstVertex + 2);

            eld::render::RenderMeshSection section{};

            section.firstIndex =
                firstIndex;

            section.indexCount =
                3;

            section.materialIndex =
                materialFor(
                    model,
                    surface.texture);

            mesh.sections.push_back(
                section);
        }
    }

}

namespace eld::graphics::terrain
{

    void batchSections(
        const eld::render::ModelResource &model,
        eld::render::RenderMesh &mesh)
    {
        struct Batch
        {
            std::uint32_t materialIndex = 0;
            float depthBias = 0.0f;

            std::vector<
                std::uint32_t>
                indices;
        };

        const std::size_t originalIndexCount =
            mesh.indices.size();

        std::vector<Batch> batches;

        batches.reserve(
            model.materials.size());

        for (
            const auto &section :
            mesh.sections)
        {
            if (
                section.firstIndex >
                    mesh.indices.size() ||
                section.indexCount >
                    mesh.indices.size() -
                        section.firstIndex)
            {
                continue;
            }

            Batch *batch =
                nullptr;

            for (
                auto &candidate :
                batches)
            {
                if (
                    candidate.materialIndex ==
                        section.materialIndex &&
                    candidate.depthBias ==
                        section.depthBias)
                {
                    batch =
                        &candidate;

                    break;
                }
            }

            if (batch == nullptr)
            {
                batches.push_back({.materialIndex =
                                       section.materialIndex,

                                   .depthBias =
                                       section.depthBias,

                                   .indices = {}});

                batch =
                    &batches.back();
            }

            const auto begin =
                mesh.indices.begin() +
                section.firstIndex;

            const auto end =
                begin +
                section.indexCount;

            batch->indices.insert(
                batch->indices.end(),
                begin,
                end);
        }

        std::vector<std::uint32_t>
            indices;

        std::vector<
            eld::render::RenderMeshSection>
            sections;

        indices.reserve(
            originalIndexCount);

        sections.reserve(
            batches.size());

        for (
            const auto &batch :
            batches)
        {
            if (batch.indices.empty())
            {
                continue;
            }

            const auto firstIndex =
                static_cast<std::uint32_t>(
                    indices.size());

            indices.insert(
                indices.end(),
                batch.indices.begin(),
                batch.indices.end());

            sections.push_back({.firstIndex =
                                    firstIndex,

                                .indexCount =
                                    static_cast<std::uint32_t>(
                                        batch.indices.size()),

                                .materialIndex =
                                    batch.materialIndex,

                                .depthBias =
                                    batch.depthBias});
        }

        if (
            indices.size() ==
            originalIndexCount)
        {
            mesh.indices =
                std::move(indices);

            mesh.sections =
                std::move(sections);
        }
        else
        {
            throw std::runtime_error(
                "Terrain batching index count mismatch");
        }
    }

}

namespace eld::graphics
{

    eld::render::ModelResource
    TerrainBuilder::build(
        const eld::world::Terrain &terrain,
        std::size_t scenePlane,
        const eld::floor::FloorLoader &floors,
        eld::graphics::TextureSystem &textures) const
    {
        if (
            scenePlane >=
            terrain.sourcePlaneCount())
        {
            throw std::out_of_range(
                "TerrainBuilder scene plane is outside terrain");
        }

        eld::render::ModelResource model;
        eld::render::RenderMesh mesh;

        // Terrain storage is organized by source layer.
        //
        // A bridge may map multiple source layers onto one
        // effective scene plane, so we must inspect all layers.
        for (
            std::size_t sourcePlane = 0;
            sourcePlane <
            terrain.sourcePlaneCount();
            ++sourcePlane)
        {
            for (
                std::size_t x = 0;
                x < terrain.width();
                ++x)
            {
                for (
                    std::size_t y = 0;
                    y < terrain.height();
                    ++y)
                {
                    const auto position =
                        terrain.layerPosition(
                            sourcePlane,
                            x,
                            y);

                    const auto &tile =
                        terrain.tile(
                            position);

                    if (
                        tile.scenePlane !=
                        scenePlane)
                    {
                        continue;
                    }

                    const auto appearance =
                        terrain::resolveAppearance(
                            tile,
                            floors,
                            textures);

                    if (!appearance.has_value())
                    {
                        continue;
                    }

                    terrain::appendTileGeometry(
                        model,
                        mesh,
                        terrain,
                        position,
                        *appearance);
                }
            }
        }

        terrain::batchSections(
            model,
            mesh);

        model.meshes.push_back(
            std::move(mesh));

        return model;
    }

}
