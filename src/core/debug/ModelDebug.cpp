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
    dumpChunk(
        payload,
        "vertex flags",
        layout.vertexFlagsOffset,
        footer.vertexCount
    );

    dumpChunk(
        payload,
        "triangle types",
        layout.triangleTypesOffset,
        footer.triangleCount
    );

    dumpChunk(
        payload,
        "triangle data",
        layout.triangleDataOffset,
        footer.triangleDataLength
    );

    dumpChunk(
        payload,
        "triangle colors",
        layout.triangleColorsOffset,
        footer.triangleCount * 2
    );

    dumpChunk(
        payload,
        "x data",
        layout.xDataOffset,
        footer.xDataLength
    );

    dumpChunk(
        payload,
        "y data",
        layout.yDataOffset,
        footer.yDataLength
    );

    dumpChunk(
        payload,
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
            << "\n";
    }
}

}
