add_library(eldoria_audio STATIC
    midi/MidiPlayer.cpp
    midi/MidiTimeline.cpp
)

add_library(Eldoria::Audio ALIAS eldoria_audio)

target_include_directories(eldoria_audio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eldoria_audio PUBLIC
    Eldoria::Assets
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
    target_link_libraries(eldoria_audio PRIVATE
        PkgConfig::FLUIDSYNTH
    )

    target_compile_definitions(eldoria_audio PRIVATE
        ELDORIA_HAS_FLUIDSYNTH=1
    )
else()
    target_compile_definitions(eldoria_audio PRIVATE
        ELDORIA_HAS_FLUIDSYNTH=0
    )
endif()
