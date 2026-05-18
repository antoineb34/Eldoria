add_library(core STATIC)

target_sources(core PRIVATE
    data/Archive.cpp
    data/Buffer.cpp
    data/CacheReader.cpp
    data/VersionList.cpp

    assets/model/ModelDef.cpp

    render/Mat4.cpp
    render/Mesh.cpp

    legacy/DefinitionsLoader.cpp
    legacy/FloDef.cpp
    legacy/IdkDef.cpp
    legacy/ItemDef.cpp
    legacy/LocDef.cpp
    legacy/MesAnimDef.cpp
    legacy/MesDef.cpp
    legacy/NpcDef.cpp
    legacy/ParamDef.cpp
    legacy/SeqDef.cpp
    legacy/SpotAnimDef.cpp
    legacy/TextureDef.cpp
    legacy/VarbitDef.cpp
    legacy/VarpDef.cpp
    legacy/MapTerrain.cpp
    legacy/MapObjects.cpp
    legacy/MapRegion.cpp
    legacy/RegionRenderer2D.cpp
)

# Debug buffer bounds checking
option(DEBUG_BUFFER_CHECKS "Enable Buffer bounds checking" OFF)
if(DEBUG_BUFFER_CHECKS)
    target_compile_definitions(core PUBLIC DEBUG_BUFFER)
endif()

# Benchmark executables, optional
option(BUILD_BENCHMARKS "Build benchmark executables" OFF)
if(BUILD_BENCHMARKS)
    add_executable(buffer_benchmark src/BufferBenchmark.cpp)
    target_link_libraries(buffer_benchmark PRIVATE core)

    add_executable(cache_benchmark src/CacheBenchmark.cpp)
    target_link_libraries(cache_benchmark PRIVATE core)
endif()

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/model
    ${CMAKE_CURRENT_SOURCE_DIR}/render
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime
    ${CMAKE_CURRENT_SOURCE_DIR}/util
    ${CMAKE_CURRENT_SOURCE_DIR}/legacy
)

target_link_libraries(core PUBLIC
    BZip2::BZip2
    ZLIB::ZLIB
)
