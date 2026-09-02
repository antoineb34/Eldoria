#include "MapFileReader.h"

#include "cache/File.h"

namespace eld::map {

MapFile MapFileReader::read(
    const eld::cache::Store& store,
    std::uint16_t fileId
) const {
    const eld::cache::File file = store.get(fileId);

    return MapFile{
        fileId,
        file.getEntry(),
        file.getCompressionType(),
        file.getBytes(eld::cache::CompressionState::Decompressed)
    };
}

}
