#include "assets/model/ModelCodec.h"

#include <stdexcept>
#include <string>
#include <utility>

ModelCodec::Header ModelCodec::readHeader(Buffer& buf, int dataSize, int id) {
    if (dataSize < 18)
        throw std::runtime_error("Model " + std::to_string(id) + ": data too small");

    buf.seek(dataSize - 18);

    Header h;
    h.vertexCount        = buf.readUShort();
    h.triangleCount      = buf.readUShort();
    h.texTriCount        = buf.readByte();
    h.hasFaceRenderTypes = buf.readByte() == 1;
    h.priorityFlag       = buf.readByte();
    h.hasFaceAlpha       = buf.readByte() == 1;
    h.hasFaceSkins       = buf.readByte() == 1;
    h.hasVertexSkins     = buf.readByte() == 1;
    h.vertexXLen         = buf.readUShort();
    h.vertexYLen         = buf.readUShort();
    h.vertexZLen         = buf.readUShort();
    h.triIndexLen        = buf.readUShort();

    int expected = h.vertexCount + h.triangleCount
        + (h.priorityFlag == 255 ? h.triangleCount : 0)
        + (h.hasFaceSkins ? h.triangleCount : 0)
        + (h.hasFaceRenderTypes ? h.triangleCount : 0)
        + (h.hasVertexSkins ? h.vertexCount : 0)
        + (h.hasFaceAlpha ? h.triangleCount : 0)
        + h.triIndexLen
        + h.triangleCount * 2
        + h.texTriCount * 6
        + h.vertexXLen + h.vertexYLen + h.vertexZLen
        + 18;

    if (dataSize < expected)
        throw std::runtime_error("Model " + std::to_string(id) + ": data too small for declared sizes");

    return h;
}

ModelCodec::Offsets ModelCodec::computeOffsets(const Header& h) {
    int p = 0;
    auto take = [&](int size) { int start = p; p += size; return start; };

    Offsets o;
    o.vertexFlags    = take(h.vertexCount);
    o.triOpcodes     = take(h.triangleCount);
    o.facePriority   = take(h.priorityFlag == 255 ? h.triangleCount : 0);
    o.faceSkin       = take(h.hasFaceSkins ? h.triangleCount : 0);
    o.faceRenderType = take(h.hasFaceRenderTypes ? h.triangleCount : 0);
    o.vertexSkin     = take(h.hasVertexSkins ? h.vertexCount : 0);
    o.faceAlpha      = take(h.hasFaceAlpha ? h.triangleCount : 0);
    o.triIndex       = take(h.triIndexLen);
    o.faceColor      = take(h.triangleCount * 2);
    o.texTri         = take(h.texTriCount * 6);
    o.vertexX        = take(h.vertexXLen);
    o.vertexY        = take(h.vertexYLen);
    o.vertexZ        = take(h.vertexZLen);
    return o;
}

void ModelCodec::decodeVertices(Buffer& buf, const Header& h, const Offsets& o, Model& m) {
    m.vertexX.resize(h.vertexCount);
    m.vertexY.resize(h.vertexCount);
    m.vertexZ.resize(h.vertexCount);

    buf.seek(o.vertexFlags);
    std::vector<uint8_t> flags(h.vertexCount);
    for (int i = 0; i < h.vertexCount; i++) flags[i] = buf.readByte();

    Buffer xBuf(buf.slice(o.vertexX, o.vertexX + h.vertexXLen));
    Buffer yBuf(buf.slice(o.vertexY, o.vertexY + h.vertexYLen));
    Buffer zBuf(buf.slice(o.vertexZ, o.vertexZ + h.vertexZLen));

    int x = 0, y = 0, z = 0;
    for (int i = 0; i < h.vertexCount; i++) {
        int f = flags[i];
        if (f & 1) x += xBuf.readSignedSmart();
        if (f & 2) y += yBuf.readSignedSmart();
        if (f & 4) z += zBuf.readSignedSmart();
        m.vertexX[i] = x;
        m.vertexY[i] = y;
        m.vertexZ[i] = z;
    }

    if (h.hasVertexSkins) {
        buf.seek(o.vertexSkin);
        m.vertexSkin.resize(h.vertexCount);
        for (int i = 0; i < h.vertexCount; i++) m.vertexSkin[i] = buf.readByte();
    }
}

void ModelCodec::decodeTriangles(Buffer& buf, const Header& h, const Offsets& o, Model& m, int id) {
    buf.seek(o.triOpcodes);
    std::vector<uint8_t> opcodes(h.triangleCount);
    for (int i = 0; i < h.triangleCount; i++) opcodes[i] = buf.readByte();

    buf.seek(o.triIndex);
    m.triA.resize(h.triangleCount);
    m.triB.resize(h.triangleCount);
    m.triC.resize(h.triangleCount);

    int a = 0, b = 0, c = 0, last = 0;
    for (int i = 0; i < h.triangleCount; i++) {
        int op = opcodes[i];
        if (op == 1) {
            a = buf.readSignedSmart() + last; last = a;
            b = buf.readSignedSmart() + last; last = b;
            c = buf.readSignedSmart() + last; last = c;
        } else if (op == 2) {
            b = c;
            c = buf.readSignedSmart() + last; last = c;
        } else if (op == 3) {
            a = c;
            c = buf.readSignedSmart() + last; last = c;
        } else if (op == 4) {
            std::swap(a, b);
            c = buf.readSignedSmart() + last; last = c;
        } else {
            throw std::runtime_error("Model " + std::to_string(id)
                + ": unknown triangle opcode " + std::to_string(op));
        }
        m.triA[i] = static_cast<uint16_t>(a);
        m.triB[i] = static_cast<uint16_t>(b);
        m.triC[i] = static_cast<uint16_t>(c);
    }
}

void ModelCodec::decodeFaceData(Buffer& buf, const Header& h, const Offsets& o, Model& m) {
    buf.seek(o.faceColor);
    m.triColor.resize(h.triangleCount);
    for (int i = 0; i < h.triangleCount; i++) m.triColor[i] = buf.readUShort();

    if (h.hasFaceRenderTypes) {
        buf.seek(o.faceRenderType);
        m.triRenderType.resize(h.triangleCount);
        for (int i = 0; i < h.triangleCount; i++) m.triRenderType[i] = buf.readByte();
    }

    if (h.priorityFlag == 255) {
        buf.seek(o.facePriority);
        m.triPriority.resize(h.triangleCount);
        for (int i = 0; i < h.triangleCount; i++) m.triPriority[i] = buf.readByte();
    } else {
        m.sharedPriority = h.priorityFlag;
    }

    if (h.hasFaceAlpha) {
        buf.seek(o.faceAlpha);
        m.triAlpha.resize(h.triangleCount);
        for (int i = 0; i < h.triangleCount; i++) m.triAlpha[i] = buf.readByte();
    }

    if (h.hasFaceSkins) {
        buf.seek(o.faceSkin);
        m.triSkin.resize(h.triangleCount);
        for (int i = 0; i < h.triangleCount; i++) m.triSkin[i] = buf.readByte();
    }
}

void ModelCodec::decodeTextureTris(Buffer& buf, const Header& h, const Offsets& o, Model& m) {
    if (h.texTriCount <= 0) return;

    buf.seek(o.texTri);
    m.texP.resize(h.texTriCount);
    m.texQ.resize(h.texTriCount);
    m.texR.resize(h.texTriCount);
    for (int i = 0; i < h.texTriCount; i++) {
        m.texP[i] = buf.readUShort();
        m.texQ[i] = buf.readUShort();
        m.texR[i] = buf.readUShort();
    }
}

Model ModelCodec::decode(int id, Buffer& buf) {
    int dataSize = static_cast<int>(buf.size());

    Header  h = readHeader(buf, dataSize, id);
    Offsets o = computeOffsets(h);

    Model m;
    m.id = id;

    decodeVertices(buf, h, o, m);
    decodeTriangles(buf, h, o, m, id);
    decodeFaceData(buf, h, o, m);
    decodeTextureTris(buf, h, o, m);

    return m;
}
