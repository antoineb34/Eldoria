#pragma once

#include <cstdint>
#include <vector>

// Decoded model geometry. Plain data, no logic, no I/O.
// Parsed by ModelCodec::decode. Built into a Mesh for rendering.
struct Model {
    int id = -1;

    // vertices (parallel arrays, index-aligned)
    std::vector<int> vertexX;
    std::vector<int> vertexY;
    std::vector<int> vertexZ;
    std::vector<int> vertexSkin;       // empty if not present

    // triangles (parallel arrays, index-aligned)
    std::vector<uint16_t> triA;
    std::vector<uint16_t> triB;
    std::vector<uint16_t> triC;
    std::vector<uint16_t> triColor;    // packed HSL: bits 15-10=hue, 9-7=sat, 6-0=light
    std::vector<uint8_t>  triRenderType; // empty if not present
    std::vector<uint8_t>  triPriority;   // empty → use sharedPriority
    std::vector<uint8_t>  triAlpha;      // empty → all faces fully opaque
    std::vector<int>      triSkin;       // empty if not present

    uint8_t sharedPriority = 0;

    // texture triangles
    std::vector<uint16_t> texP;
    std::vector<uint16_t> texQ;
    std::vector<uint16_t> texR;

    // pure const queries over the struct's own fields — fine to keep here
    bool isFaceTextured(int i)    const { return !triRenderType.empty() && (triRenderType[i] & 2); }
    bool isFaceTransparent(int i) const { return !triRenderType.empty() && (triRenderType[i] & 1); }
    int  faceTexTriIndex(int i)   const { return triRenderType.empty() ? 0 : (triRenderType[i] >> 2); }
};
