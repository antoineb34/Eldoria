add_library(eldoria_math INTERFACE)

add_library(Eldoria::Math ALIAS eldoria_math)

target_include_directories(eldoria_math INTERFACE
    ${PROJECT_SOURCE_DIR}/src/runtime
)
