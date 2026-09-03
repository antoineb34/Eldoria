cmake_minimum_required(VERSION 3.20)

project(Eldoria VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(EldoriaTargets)

find_package(ZLIB REQUIRED)
find_package(SDL3 REQUIRED CONFIG)
find_package(BZip2 REQUIRED)

# Shared modules.
eldoria_add_module(binary src/binary)
eldoria_add_module(storage src/storage)
eldoria_add_module(assets src/assets)
eldoria_add_module(host src/host)
eldoria_add_module(runtime src/runtime)

# Eldoria applications.
eldoria_add_app(elforge src/apps/elforge)
eldoria_add_app(elclient src/apps/elclient)
eldoria_add_app(elserver src/apps/elserver)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
