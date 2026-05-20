add_library(core STATIC
    filestore/FileStore.cpp
    filestore/Archive.cpp
    filestore/VersionList.cpp

    io/Buffer.cpp

    assets/model/ModelDef.cpp     # real, compiles

    # assets/model/ModelCodec.cpp   # stub — uncomment when ported
    # assets/item/ItemCodec.cpp     # stub — uncomment when ported
    # assets/Assets.cpp             # stub — uncomment when ready

    math/Mat4.cpp
    render/Mesh.cpp

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
)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core PUBLIC
    BZip2::BZip2
    ZLIB::ZLIB
)
