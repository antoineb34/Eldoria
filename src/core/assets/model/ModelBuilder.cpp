#include "ModelBuilder.h"

#include "decoder/FaceDecoder.h"
#include "decoder/TextureUVMappingDecoder.h"
#include "decoder/VertexDecoder.h"

namespace rf::model {

ModelAsset ModelBuilder::build(
    const ModelFile& file
) const {
    VertexDecoder vertexDecoder(file);
    FaceDecoder faceDecoder(file);
    TextureUVMappingDecoder textureUVMappingDecoder(file);

    return {
        vertexDecoder.decode(),
        faceDecoder.decode(),
        textureUVMappingDecoder.decode()
    };
}

}
