#pragma once

#include "model/ModelData.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace eld::elclient::debug {


// ------------------------------------------------------------
// RGB -> RuneScape packed HSL
// ------------------------------------------------------------

inline std::uint16_t rgbToRsColor(
    float red,
    float green,
    float blue
) {
    red = std::clamp(red, 0.0f, 1.0f);
    green = std::clamp(green, 0.0f, 1.0f);
    blue = std::clamp(blue, 0.0f, 1.0f);

    const float maximum =
        std::max({red, green, blue});

    const float minimum =
        std::min({red, green, blue});

    const float delta =
        maximum - minimum;

    const float lightness =
        (maximum + minimum) * 0.5f;

    float hue = 0.0f;
    float saturation = 0.0f;

    if (delta > 0.000001f) {
        saturation =
            lightness > 0.5f
                ? delta /
                    (2.0f - maximum - minimum)
                : delta /
                    (maximum + minimum);

        if (maximum == red) {
            hue =
                (green - blue) / delta +
                (green < blue ? 6.0f : 0.0f);
        }
        else if (maximum == green) {
            hue =
                (blue - red) / delta +
                2.0f;
        }
        else {
            hue =
                (red - green) / delta +
                4.0f;
        }

        hue /= 6.0f;
    }

    const auto packedHue =
        static_cast<std::uint16_t>(
            std::clamp(
                static_cast<int>(
                    std::floor(hue * 64.0f)
                ),
                0,
                63
            )
        );

    const auto packedSaturation =
        static_cast<std::uint16_t>(
            std::clamp(
                static_cast<int>(
                    std::lround(
                        saturation * 7.0f
                    )
                ),
                0,
                7
            )
        );

    const auto packedLightness =
        static_cast<std::uint16_t>(
            std::clamp(
                static_cast<int>(
                    std::lround(
                        lightness * 127.0f
                    )
                ),
                0,
                127
            )
        );

    return static_cast<std::uint16_t>(
        (packedHue << 10) |
        (packedSaturation << 7) |
        packedLightness
    );
}


// ------------------------------------------------------------
// MTL
// ------------------------------------------------------------

inline std::unordered_map<
    std::string,
    std::uint16_t
>
loadMtlColors(
    const std::filesystem::path& path
) {
    std::unordered_map<
        std::string,
        std::uint16_t
    > colors;

    std::ifstream file(path);

    if (!file) {
        throw std::runtime_error(
            "Could not open MTL: " +
            path.string()
        );
    }

    std::string currentMaterial;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream stream(line);

        std::string command;
        stream >> command;

        if (command == "newmtl") {
            stream >> currentMaterial;
        }
        else if (
            command == "Kd" &&
            !currentMaterial.empty()
        ) {
            float red = 1.0f;
            float green = 1.0f;
            float blue = 1.0f;

            stream
                >> red
                >> green
                >> blue;

            colors[currentMaterial] =
                rgbToRsColor(
                    red,
                    green,
                    blue
                );
        }
    }

    return colors;
}


// ------------------------------------------------------------
// OBJ
// ------------------------------------------------------------

inline std::uint32_t parseObjIndex(
    const std::string& token,
    std::size_t vertexCount
) {
    const std::size_t slash =
        token.find('/');

    const std::string indexText =
        token.substr(0, slash);

    const int rawIndex =
        std::stoi(indexText);

    if (rawIndex == 0) {
        throw std::runtime_error(
            "OBJ index 0 is invalid"
        );
    }

    if (rawIndex > 0) {
        return static_cast<std::uint32_t>(
            rawIndex - 1
        );
    }

    const long long index =
        static_cast<long long>(
            vertexCount
        ) + rawIndex;

    if (index < 0) {
        throw std::runtime_error(
            "OBJ negative index is invalid"
        );
    }

    return static_cast<std::uint32_t>(
        index
    );
}


inline eld::model::ModelData loadObj(
    const std::filesystem::path& path
) {
    std::ifstream file(path);

    if (!file) {
        throw std::runtime_error(
            "Could not open OBJ: " +
            path.string()
        );
    }

    eld::model::ModelData model;

    model.id = 65000;

    std::unordered_map<
        std::string,
        std::uint16_t
    > materialColors;

    const std::uint16_t defaultColor =
        rgbToRsColor(
            0.55f,
            0.55f,
            0.55f
        );

    std::uint16_t currentColor =
        defaultColor;


    // OBJ smoothing state -> RuneScape render type.
    //
    // OBJ:
    //
    //     s off / s 0
    //         flat shading
    //
    //     s 1 / s 2 / s on / ...
    //         smooth shading
    //
    // Eldoria:
    //
    //     renderType 1 = face normal / flat
    //     renderType 0 = accumulated vertex normals / smooth
    //
    // Keep the historic loader behavior as the default:
    // OBJ files without an "s" directive remain flat shaded.
    std::uint8_t currentRenderType = 1;

    const auto baseDirectory =
        path.parent_path();

    std::string line;

    while (std::getline(file, line)) {
        if (
            line.empty() ||
            line.front() == '#'
        ) {
            continue;
        }

        std::istringstream stream(line);

        std::string command;
        stream >> command;

        if (command == "mtllib") {
            std::string fileName;
            stream >> fileName;

            materialColors =
                loadMtlColors(
                    baseDirectory /
                    fileName
                );
        }
        else if (command == "usemtl") {
            std::string materialName;
            stream >> materialName;

            const auto found =
                materialColors.find(
                    materialName
                );

            currentColor =
                found != materialColors.end()
                    ? found->second
                    : defaultColor;
        }
        else if (command == "s") {
            std::string smoothing;
            stream >> smoothing;

            // "off" and "0" explicitly disable smoothing.
            //
            // Any named/numeric smoothing group enables
            // smooth vertex-normal shading. We do not need to
            // preserve the actual group ID yet because ModelData
            // currently stores shading per face.
            if (
                smoothing == "off" ||
                smoothing == "0"
            ) {
                currentRenderType = 1;
            }
            else {
                currentRenderType = 0;
            }
        }
        else if (command == "v") {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;

            stream >> x >> y >> z;

            eld::model::Vertex vertex;

            vertex.x = x;

            // OBJ Y-up -> Eldoria source representation.
            // ModelBuilder later turns source Y into render -Y.
            vertex.y = -y;

            vertex.z = z;

            model.vertices.push_back(
                vertex
            );
        }
        else if (command == "f") {
            std::vector<std::uint32_t>
                polygon;

            std::string token;

            while (stream >> token) {
                polygon.push_back(
                    parseObjIndex(
                        token,
                        model.vertices.size()
                    )
                );
            }

            if (polygon.size() < 3) {
                continue;
            }

            // Fan triangulation.
            for (
                std::size_t i = 1;
                i + 1 < polygon.size();
                ++i
            ) {
                eld::model::Face face;

                face.a = polygon[0];
                face.b = polygon[i];
                face.c = polygon[i + 1];

                face.color =
                    currentColor;

                // Preserve the active OBJ smoothing mode.
                //
                // renderType 0 = smooth vertex normals
                // renderType 1 = flat face normal
                face.renderType =
                    currentRenderType;

                model.faces.push_back(
                    face
                );
            }
        }
    }

    if (model.vertices.empty()) {
        throw std::runtime_error(
            "OBJ contains no vertices"
        );
    }

    if (model.faces.empty()) {
        throw std::runtime_error(
            "OBJ contains no faces"
        );
    }

    return model;
}


// ------------------------------------------------------------
// Bounds
// ------------------------------------------------------------

struct Bounds {
    float minX;
    float maxX;

    float minY;
    float maxY;

    float minZ;
    float maxZ;
};


inline Bounds boundsOf(
    const eld::model::ModelData& model
) {
    if (model.vertices.empty()) {
        throw std::runtime_error(
            "Cannot calculate empty model bounds"
        );
    }

    Bounds bounds{
        model.vertices.front().x,
        model.vertices.front().x,

        model.vertices.front().y,
        model.vertices.front().y,

        model.vertices.front().z,
        model.vertices.front().z
    };

    for (
        const auto& vertex :
        model.vertices
    ) {
        bounds.minX =
            std::min(bounds.minX, vertex.x);

        bounds.maxX =
            std::max(bounds.maxX, vertex.x);

        bounds.minY =
            std::min(bounds.minY, vertex.y);

        bounds.maxY =
            std::max(bounds.maxY, vertex.y);

        bounds.minZ =
            std::min(bounds.minZ, vertex.z);

        bounds.maxZ =
            std::max(bounds.maxZ, vertex.z);
    }

    return bounds;
}


// ------------------------------------------------------------
// Fit generated sword into the authored coordinate space of
// cache worn-model 518.
//
// Generated OBJ:
//     long axis = Y
//
// RuneScape worn sword:
//     long axis = Z
//
// We use the reference sword's Z span for UNIFORM scaling.
// That preserves the crystal sword's wider guard instead of
// crushing X to the bronze sword's very thin width.
// ------------------------------------------------------------

inline void fitToWornReference(
    eld::model::ModelData& generated,
    const eld::model::ModelData& reference
) {
    const Bounds source =
        boundsOf(generated);

    const Bounds target =
        boundsOf(reference);

    const float sourceCenterX =
        (source.minX + source.maxX) *
        0.5f;

    const float sourceCenterY =
        (source.minY + source.maxY) *
        0.5f;

    const float sourceCenterZ =
        (source.minZ + source.maxZ) *
        0.5f;


    const float targetCenterX =
        (target.minX + target.maxX) *
        0.5f;

    const float targetCenterY =
        (target.minY + target.maxY) *
        0.5f;

    const float targetCenterZ =
        (target.minZ + target.maxZ) *
        0.5f;


    const float sourceLength =
        source.maxY -
        source.minY;

    const float targetLength =
        target.maxZ -
        target.minZ;

    if (sourceLength <= 0.000001f) {
        throw std::runtime_error(
            "Generated sword has zero length"
        );
    }

    const float scale =
        targetLength /
        sourceLength;


    const auto weaponVertexSkin =
        reference.vertices.empty()
            ? std::optional<std::uint8_t>{}
            : reference.vertices.front().skin;

    const auto weaponFaceSkin =
        reference.faces.empty()
            ? std::optional<std::uint8_t>{}
            : reference.faces.front().skin;


    for (
        auto& vertex :
        generated.vertices
    ) {
        const float localX =
            vertex.x -
            sourceCenterX;

        const float localY =
            vertex.y -
            sourceCenterY;

        const float localZ =
            vertex.z -
            sourceCenterZ;


        // Roll the sword 90 degrees around its long axis:
        //
        // generated depth Z -> RuneScape X
        // generated width X -> RuneScape Y
        // generated length Y -> RuneScape Z
        //
        // This makes the broad crystal/shard face vertical
        // instead of lying flat sideways.

        vertex.x =
            targetCenterX +
            localZ * scale;

        vertex.y =
            targetCenterY -
            localX * scale;

        vertex.z =
            targetCenterZ +
            localY * scale;


        // Model 518 uses the right-hand weapon animation group.
        vertex.skin =
            weaponVertexSkin;
    }


    for (
        auto& face :
        generated.faces
    ) {
        face.skin =
            weaponFaceSkin;
    }
}


// ------------------------------------------------------------
// Merge another ModelData into the player appearance.
// ------------------------------------------------------------

inline void appendModel(
    eld::model::ModelData& destination,
    eld::model::ModelData source
) {
    const auto vertexOffset =
        static_cast<std::uint32_t>(
            destination.vertices.size()
        );

    const auto mappingOffset =
        static_cast<std::uint32_t>(
            destination.textureMappings.size()
        );


    for (
        auto& face :
        source.faces
    ) {
        face.a += vertexOffset;
        face.b += vertexOffset;
        face.c += vertexOffset;

        if (
            face.textureMappingIndex
                .has_value()
        ) {
            *face.textureMappingIndex +=
                mappingOffset;
        }
    }


    for (
        auto& mapping :
        source.textureMappings
    ) {
        mapping.originVertex +=
            vertexOffset;

        mapping.uVertex +=
            vertexOffset;

        mapping.vVertex +=
            vertexOffset;
    }


    destination.vertices.insert(
        destination.vertices.end(),
        source.vertices.begin(),
        source.vertices.end()
    );

    destination.faces.insert(
        destination.faces.end(),
        source.faces.begin(),
        source.faces.end()
    );

    destination.textureMappings.insert(
        destination.textureMappings.end(),
        source.textureMappings.begin(),
        source.textureMappings.end()
    );
}

} // namespace eld::elclient::debug
