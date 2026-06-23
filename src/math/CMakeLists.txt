add_library(eldoria_math INTERFACE)

add_library(Eldoria::Math ALIAS eldoria_math)

target_include_directories(eldoria_math INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
