#pragma once

#include "IndexedImageFile.h"
#include "Image.h"
#include "IndexedImageSourceMap.h"

namespace eld::image {

class IndexedImageDecoder {
public:
    Image decode(
        const IndexedImageFile& file
    ) const;

    Image decode(
        const IndexedImageFile& file,
        IndexedImageSourceMap& sourceMap
    ) const;

private:
    Image decodeImage(
        const IndexedImageFile& file,
        IndexedImageSourceMap* sourceMap
    ) const;
};

}
