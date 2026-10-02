add_library(
    eldoria_network
    STATIC
    TcpServer.cpp
    TcpSession.cpp
    TcpClient.cpp
)

add_library(
    Eldoria::Network
    ALIAS eldoria_network
)

target_include_directories(
    eldoria_network
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    eldoria_network
    PUBLIC
        Eldoria::Asio
        Eldoria::Protocol
)
