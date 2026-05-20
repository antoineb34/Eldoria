cmake_minimum_required(VERSION 3.20)

project(RuneForgeTool)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(ZLIB REQUIRED)

# SDL3 via pkg-config
pkg_check_modules(SDL3 REQUIRED sdl3)

add_executable(tool
    main.cpp

    ../core/cache/CacheStore.cpp
    ../core/io/ByteBuffer.cpp
    ../core/io/Compression.cpp
)

target_include_directories(tool PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL3_INCLUDE_DIRS}
)

target_link_directories(tool PRIVATE
    ${SDL3_LIBRARY_DIRS}
)

target_link_libraries(tool PRIVATE
    ZLIB::ZLIB
    ${SDL3_LIBRARIES}
)
