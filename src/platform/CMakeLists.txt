add_library(eldoria_platform STATIC
    audio/MidiPlayer.cpp
    sdl/SdlContext.cpp
    sdl/SdlOpenGLContext.cpp
)

add_library(Eldoria::Platform ALIAS eldoria_platform)

target_include_directories(eldoria_platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_platform PUBLIC
    SDL3::SDL3
)

find_package(PkgConfig QUIET)

if(PkgConfig_FOUND)
    pkg_check_modules(
        FLUIDSYNTH
        QUIET
        IMPORTED_TARGET
        fluidsynth
    )
endif()

if(TARGET PkgConfig::FLUIDSYNTH)
    target_link_libraries(eldoria_platform PRIVATE
        PkgConfig::FLUIDSYNTH
    )

    target_compile_definitions(eldoria_platform PRIVATE
        ELDORIA_HAS_FLUIDSYNTH=1
    )
else()
    target_compile_definitions(eldoria_platform PRIVATE
        ELDORIA_HAS_FLUIDSYNTH=0
    )
endif()

add_subdirectory(imgui)
