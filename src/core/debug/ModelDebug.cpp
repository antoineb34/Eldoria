#include "ModelDebug.h"

#include <iomanip>
#include <iostream>

namespace rf::debug {

void dumpBytes(
    const std::vector<char>& data,
    const char* title
) {
    std::cout << "\n" << title << ":\n";

    for (int i = 0; i < data.size(); i++) {
        if (i % 16 == 0) {
            std::cout << "\n";
        }

        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int)(unsigned char)data[i]
            << " ";
    }

    std::cout << std::dec << "\n";
}

void dumpChunk(
    const std::vector<char>& data,
    const char* name,
    int start,
    int length
) {
    std::cout
        << "\n\n=== "
        << name
        << " ==="
        << "\noffset: "
        << start
        << "\nlength: "
        << length
        << " bytes\n";

    for (int i = 0; i < length; i++) {
        if (i % 16 == 0) {
            std::cout << "\n";
        }

        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int)(unsigned char)data[start + i]
            << " ";
    }

    std::cout << std::dec << "\n";
}

void dumpModelFooter(
    const rf::model::ModelFooter& footer
) {
    std::cout
        << "\nmodel footer decoded:"
        << "\nvertex count: "
        << footer.vertexCount
        << " vertices"

        << "\ntriangle count: "
        << footer.triangleCount
        << " triangles"

        << "\ntexture triangle count: "
        << footer.textureTriangleCount
        << " textured triangles"

        << "\ntexture flag: "
        << footer.textureFlag

        << "\npriority flag: "
        << footer.priorityFlag

        << "\nalpha flag: "
        << footer.alphaFlag

        << "\ntriangle skin flag: "
        << footer.triangleSkinFlag

        << "\nvertex skin flag: "
        << footer.vertexSkinFlag

        << "\nx data length: "
        << footer.xDataLength
        << " bytes"

        << "\ny data length: "
        << footer.yDataLength
        << " bytes"

        << "\nz data length: "
        << footer.zDataLength
        << " bytes"

        << "\ntriangle data length: "
        << footer.triangleDataLength
        << " bytes"

        << "\n";
}

void dumpModelChunks(
    const std::vector<char>& payload,
    const rf::model::ModelFooter& footer,
    const rf::model::ModelLayout& layout
) {
    auto dumpMaybe =
        [&](const char* name, int start, int length) {

            if (length <= 0) {
                std::cout
                    << "\n\n=== "
                    << name
                    << " ==="
                    << "\noffset: "
                    << start
                    << "\nlength: 0 bytes"
                    << "\nempty / not present\n";

                return;
            }

            dumpChunk(
                payload,
                name,
                start,
                length
            );
        };

    dumpMaybe(
        "vertex flags",
        layout.vertexFlagsOffset,
        footer.vertexCount
    );

    dumpMaybe(
        "triangle types",
        layout.triangleTypesOffset,
        footer.triangleCount
    );

    dumpMaybe(
        "triangle priorities",
        layout.trianglePrioritiesOffset,
        footer.priorityFlag == 255
            ? footer.triangleCount
            : 0
    );

    dumpMaybe(
        "triangle skins",
        layout.triangleSkinsOffset,
        footer.triangleSkinFlag == 1
            ? footer.triangleCount
            : 0
    );

    dumpMaybe(
        "texture pointers",
        layout.texturePointersOffset,
        footer.textureFlag == 1
            ? footer.triangleCount
            : 0
    );

    dumpMaybe(
        "vertex skins",
        layout.vertexSkinsOffset,
        footer.vertexSkinFlag == 1
            ? footer.vertexCount
            : 0
    );

    dumpMaybe(
        "triangle alphas",
        layout.triangleAlphasOffset,
        footer.alphaFlag == 1
            ? footer.triangleCount
            : 0
    );

    dumpMaybe(
        "triangle data",
        layout.triangleDataOffset,
        footer.triangleDataLength
    );

    dumpMaybe(
        "triangle colors",
        layout.triangleColorsOffset,
        footer.triangleCount * 2
    );

    dumpMaybe(
        "texture data",
        layout.textureDataOffset,
        footer.textureTriangleCount * 6
    );

    dumpMaybe(
        "x data",
        layout.xDataOffset,
        footer.xDataLength
    );

    dumpMaybe(
        "y data",
        layout.yDataOffset,
        footer.yDataLength
    );

    dumpMaybe(
        "z data",
        layout.zDataOffset,
        footer.zDataLength
    );
}

void dumpDecodedVertices(
    const std::vector<rf::model::Vertex>& vertices
) {
    std::cout
        << "\n\n=== decoded vertices ==="
        << "\nvertex count: "
        << vertices.size()
        << "\n";

    for (int i = 0; i < vertices.size(); i++) {
        std::cout
            << "vertex "
            << i
            << ": x="
            << vertices[i].x
            << " y="
            << vertices[i].y
            << " z="
            << vertices[i].z
            << "\n";
    }
}

void dumpDecodedFaces(
    const std::vector<rf::model::Face>& faces
) {
    std::cout
        << "\n\n=== decoded faces ==="
        << "\nface count: "
        << faces.size()
        << "\n";

    for (int i = 0; i < faces.size(); i++) {
        std::cout
            << "face "
            << i
            << ": "
            << faces[i].a
            << ", "
            << faces[i].b
            << ", "
            << faces[i].c
            << " (color:"
            << faces[i].color
            << " priority:"
            << (int)faces[i].priority
            << " alpha:"
            << (int)faces[i].alpha
            << " texture:"
            << faces[i].textureFlag
            << ")\n";
    }
}

}
