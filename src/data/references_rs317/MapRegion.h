#pragma once

#include <cstddef>

#include "filestore/FileStore.h"
#include "MapObjects.h"
#include "MapTerrain.h"
#include "filestore/VersionList.h"

class MapRegion {
public:
    static MapRegion load(FileStore& reader, const VersionList& versionList, int regionId);

    int regionId() const;
    int terrainFileId() const;
    int objectFileId() const;
    std::size_t terrainDataSize() const;
    std::size_t objectDataSize() const;

    const MapTerrain& terrain() const;
    const MapObjects& objects() const;

private:
    int regionId_ = -1;
    int terrainFileId_ = -1;
    int objectFileId_ = -1;
    std::size_t terrainDataSize_ = 0;
    std::size_t objectDataSize_ = 0;
    MapTerrain terrain_;
    MapObjects objects_;
};
