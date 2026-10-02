cmake_minimum_required(VERSION 3.20)

project(Eldoria VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(EldoriaTargets)

include(FetchContent)


# ------------------------------------------------------------
# Standalone Asio
# ------------------------------------------------------------

find_package(Threads REQUIRED)

FetchContent_Declare(
    asio
    GIT_REPOSITORY
        https://github.com/chriskohlhoff/asio.git
    GIT_TAG
        asio-1-38-2
)

FetchContent_GetProperties(
    asio
)

if(NOT asio_POPULATED)
    FetchContent_Populate(
        asio
    )
endif()

add_library(
    eldoria_asio
    INTERFACE
)

add_library(
    Eldoria::Asio
    ALIAS eldoria_asio
)

target_include_directories(
    eldoria_asio
    SYSTEM
    INTERFACE
        ${asio_SOURCE_DIR}/asio/include
)

target_compile_definitions(
    eldoria_asio
    INTERFACE
        ASIO_STANDALONE
        ASIO_NO_DEPRECATED
)

target_link_libraries(
    eldoria_asio
    INTERFACE
        Threads::Threads
)


# OpenGL 3.3 Core function loader.
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
    SOURCE_SUBDIR cmake
)

FetchContent_MakeAvailable(glad)

glad_add_library(eldoria_glad STATIC
    REPRODUCIBLE
    API gl:core=3.3
)


set(
    BUILD_MIDILIBRARY_ONLY
    ON
    CACHE BOOL ""
    FORCE
)

FetchContent_Declare(
    midifile
    GIT_REPOSITORY
        https://github.com/craigsapp/midifile.git
    GIT_TAG
        98917df5b1bf0d6e8d4c0e5fff86d6b05343e793
)

set(
    CMAKE_POLICY_VERSION_MINIMUM
    3.5
)

FetchContent_MakeAvailable(midifile)


find_package(ZLIB REQUIRED)
find_package(SDL3 REQUIRED CONFIG)
find_package(BZip2 REQUIRED)
find_package(OpenGL REQUIRED)

# Shared modules.
eldoria_add_module(binary src/binary)
eldoria_add_module(storage src/storage)
eldoria_add_module(assets src/assets)
eldoria_add_module(host src/host)
eldoria_add_module(runtime src/runtime)
eldoria_add_module(graphics src/graphics)

# Eldoria applications.
eldoria_add_app(elforge src/apps/elforge)
eldoria_add_app(elclient src/apps/elclient)
eldoria_add_app(elserver src/apps/elserver)

add_subdirectory(tools/dataset_builder)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
