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
find_package(JPEG REQUIRED)

# Shared modules.
eldoria_add_module(data src/data)
eldoria_add_module(world src/world)
eldoria_add_module(game src/game)
eldoria_add_module(network src/net)
eldoria_add_module(platform src/platform)
eldoria_add_module(render src/render)
eldoria_add_module(graphics src/graphics)
eldoria_add_module(math src/math)

# Target Eldoria applications.
eldoria_add_app(elforge src/apps/elforge)
eldoria_add_app(elclient src/apps/elclient)
eldoria_add_app(elserver src/apps/elserver)
eldoria_add_app(interface_probe src/apps/interface_probe)

eldoria_add_app(animation_probe src/apps/animation_probe)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
