add_library(
    eldoria_protocol
    STATIC
    PacketDecoder.cpp
    PacketFraming.cpp
    HelloPackets.cpp
    WalkPackets.cpp
    PlayerPackets.cpp
)

add_library(
    Eldoria::Protocol
    ALIAS eldoria_protocol
)

target_include_directories(
    eldoria_protocol
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    eldoria_protocol
    PUBLIC
        Eldoria::Binary
)
