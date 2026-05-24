#include "ModelDecoder.h"

#include "ModelFooter.h"
#include "ModelLayout.h"
#include "VertexDecoder.h"
#include "FaceDecoder.h"
#include "TextureTriangleDecoder.h"

namespace rf::model {

ModelDef decodeModel(
    const std::vector<char>& payload
) {
    ModelFooter footer =
        readModelFooter(payload);

    ModelLayout layout =
        calculateModelLayout(footer);

    std::vector<Vertex> vertices =
        decodeVertices(payload, footer, layout);

    std::vector<TextureTriangle> textureTriangles =
        decodeTextureTriangles(payload, footer, layout);

    std::vector<Face> faces =
        decodeFaces(payload, footer, layout);

    return ModelDef{
        footer,
        layout,
        std::move(vertices),
        std::move(faces),
        std::move(textureTriangles)
    };
}

}
