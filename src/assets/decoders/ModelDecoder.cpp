#include "decoders/ModelDecoder.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>

#include "binary/ByteReader.h"

namespace eld::model {

Model ModelDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    constexpr std::size_t FooterSize = 18;

    if (payload.size() < FooterSize) {
        throw std::runtime_error(
            "Model payload is too small"
        );
    }

    std::span<const std::uint8_t> body =
        payload.first(payload.size() - FooterSize);

    eld::binary::ByteReader footer(
        payload.last(FooterSize)
    );

    const auto vertexCount = footer.readU16();
    const auto faceCount = footer.readU16();
    const auto textureMappingCount = footer.readU8();

    const auto texture = footer.readU8();
    const auto priority = footer.readU8();
    const auto alpha = footer.readU8();
    const auto faceSkin = footer.readU8();
    const auto vertexSkin = footer.readU8();

    const auto xLength = footer.readU16();
    const auto yLength = footer.readU16();
    const auto zLength = footer.readU16();
    const auto faceDataLength = footer.readU16();


    // Body streams

    std::size_t offset = 0;

    auto section = [&](std::size_t size) {
        if (
            offset > body.size() ||
            size > body.size() - offset
        ) {
            throw std::runtime_error(
                "Model section exceeds body"
            );
        }

        eld::binary::ByteReader reader(
            body.subspan(offset)
        );

        offset += size;
        return reader;
    };

    auto vertexFlags = section(vertexCount);
    auto faceTypes = section(faceCount);

    auto facePriorities =
        section(priority == 255 ? faceCount : 0);

    auto faceSkins =
        section(faceSkin == 1 ? faceCount : 0);

    auto faceTextures =
        section(texture == 1 ? faceCount : 0);

    auto vertexSkins =
        section(vertexSkin == 1 ? vertexCount : 0);

    auto faceAlphas =
        section(alpha == 1 ? faceCount : 0);

    auto faceData = section(faceDataLength);
    auto faceColors = section(faceCount * 2);
    auto textureTriangles = section(textureMappingCount * 6);

    auto xData = section(xLength);
    auto yData = section(yLength);
    auto zData = section(zLength);

    if (offset != body.size()) {
        throw std::runtime_error(
            "Invalid model layout"
        );
    }


    // Model

    Model model;

    model.vertices.resize(vertexCount);
    model.faces.resize(faceCount);
    model.textureMappings.resize(textureMappingCount);


    // Vertices

    int x = 0;
    int y = 0;
    int z = 0;

    for (Vertex& vertex : model.vertices) {
        const auto flags = vertexFlags.readU8();

        if (flags & 1) {
            x += xData.readSignedSmart();
        }

        if (flags & 2) {
            y += yData.readSignedSmart();
        }

        if (flags & 4) {
            z += zData.readSignedSmart();
        }

        vertex.x = static_cast<float>(x);
        vertex.y = static_cast<float>(y);
        vertex.z = static_cast<float>(z);

        if (vertexSkin == 1) {
            vertex.skin = vertexSkins.readU8();
        }
    }


    // Faces

    std::uint32_t a = 0;
    std::uint32_t b = 0;
    std::uint32_t c = 0;

    int last = 0;

    auto index = [&] {
        last += faceData.readSignedSmart();

        return static_cast<std::uint32_t>(
            last
        );
    };

    for (Face& face : model.faces) {
        switch (faceTypes.readU8()) {
            case 1:
                a = index();
                b = index();
                c = index();
                break;

            case 2:
                b = c;
                c = index();
                break;

            case 3:
                a = c;
                c = index();
                break;

            case 4:
                std::swap(a, b);
                c = index();
                break;

            default:
                break;
        }

        face.a = a;
        face.b = b;
        face.c = c;
        face.color = faceColors.readU16();

        face.priority =
            priority == 255
                ? facePriorities.readU8()
                : priority;

        if (alpha == 1) {
            face.alpha = faceAlphas.readU8();
        }

        if (faceSkin == 1) {
            face.skin = faceSkins.readU8();
        }

        if (texture == 1) {
            const auto info =
                faceTextures.readU8();

            face.renderType = info & 1;

            if (info & 2) {
                face.textureId = face.color;
                face.textureMappingIndex =
                    static_cast<std::uint32_t>(
                        info >> 2
                    );
            }
        }
    }


    // Texture mappings

    for (TextureMapping& mapping : model.textureMappings) {
        mapping.originVertex = textureTriangles.readU16();
        mapping.uVertex = textureTriangles.readU16();
        mapping.vVertex = textureTriangles.readU16();
    }

    return model;
}

}
