#pragma once

#include "assets/model/Model.h"
#include "io/Buffer.h"

class ModelCodec {
public:
    static Model decode(int id, Buffer& buf);

private:
    struct Header {
        int vertexCount;
        int triangleCount;
        int texTriCount;
        bool hasFaceRenderTypes;
        bool hasFaceAlpha;
        bool hasFaceSkins;
        bool hasVertexSkins;
        uint8_t priorityFlag;
        int vertexXLen;
        int vertexYLen;
        int vertexZLen;
        int triIndexLen;
    };

    struct Offsets {
        int vertexFlags;
        int triOpcodes;
        int facePriority;
        int faceSkin;
        int faceRenderType;
        int vertexSkin;
        int faceAlpha;
        int triIndex;
        int faceColor;
        int texTri;
        int vertexX;
        int vertexY;
        int vertexZ;
    };

    static Header  readHeader(Buffer& buf, int dataSize, int id);
    static Offsets  computeOffsets(const Header& h);
    static void     decodeVertices(Buffer& buf, const Header& h, const Offsets& o, Model& m);
    static void     decodeTriangles(Buffer& buf, const Header& h, const Offsets& o, Model& m, int id);
    static void     decodeFaceData(Buffer& buf, const Header& h, const Offsets& o, Model& m);
    static void     decodeTextureTris(Buffer& buf, const Header& h, const Offsets& o, Model& m);
};
