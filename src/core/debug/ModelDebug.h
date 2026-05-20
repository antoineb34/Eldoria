#pragma once

#include <vector>

#include "../model/FaceDecoder.h"
#include "../model/ModelFooter.h"
#include "../model/ModelLayout.h"
#include "../model/VertexDecoder.h"

namespace rf::debug {

void dumpBytes(
    const std::vector<char>& data,
    const char* title
);

void dumpChunk(
    const std::vector<char>& data,
    const char* name,
    int start,
    int length
);

void dumpModelFooter(
    const rf::model::ModelFooter& footer
);

void dumpModelChunks(
    const std::vector<char>& payload,
    const rf::model::ModelFooter& footer,
    const rf::model::ModelLayout& layout
);

void dumpDecodedVertices(
    const std::vector<rf::model::Vertex>& vertices
);

void dumpDecodedFaces(
    const std::vector<rf::model::Face>& faces
);

}
