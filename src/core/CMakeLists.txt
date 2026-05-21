add_library(core STATIC
    filestore/FileStore.cpp
    filestore/Archive.cpp
    filestore/VersionList.cpp

    io/Buffer.cpp
    io/ByteBuffer.cpp
    io/Compression.cpp

    cache/CacheStore.cpp
    cache/ArchiveDecoder.cpp
    cache/ArchiveFileTable.cpp
    cache/NameHash.cpp
    cache/ArchiveFileExtractor.cpp
    cache/ConfigArchiveLoader.cpp

    model/ModelFooter.cpp
    model/ModelLayout.cpp
    model/VertexDecoder.cpp
    model/FaceDecoder.cpp

    platform/SdlContext.cpp

    render/Projection.cpp
    render/WireframeRenderer.cpp
    render/Color.cpp
    render/TriangleRasterizer.cpp
    render/DepthBuffer.cpp
    render/Mesh.cpp

    debug/ModelDebug.cpp

    assets/model/ModelDef.cpp

    math/Mat4.cpp

    legacy/DefinitionsLoader.cpp
    legacy/FloDef.cpp
    legacy/IdkDef.cpp
    legacy/ItemDef.cpp
    legacy/LocDef.cpp
    legacy/MapObjects.cpp
    legacy/MapRegion.cpp
    legacy/MapTerrain.cpp
    legacy/MesAnimDef.cpp
    legacy/MesDef.cpp
    legacy/NpcDef.cpp
    legacy/ParamDef.cpp
    legacy/RegionRenderer2D.cpp
    legacy/SeqDef.cpp
    legacy/SpotAnimDef.cpp
    legacy/TextureDef.cpp
    legacy/VarbitDef.cpp
    legacy/VarpDef.cpp

    texture/TextureDecoder.cpp
    texture/TextureIndex.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core PUBLIC
    BZip2::BZip2
    ZLIB::ZLIB
    SDL3::SDL3
)
