#pragma once

#include <cstdint>
#include <vector>

#include "Packet.h"


namespace eld::protocol
{

    struct ClientHello
    {
        std::uint16_t
            protocolVersion = 0;
    };


    struct ServerHello
    {
        bool
            accepted = false;


        std::uint16_t
            protocolVersion = 0;
    };


    std::vector<std::uint8_t> encode(
        const ClientHello& packet
    );


    std::vector<std::uint8_t> encode(
        const ServerHello& packet
    );


    ClientHello decodeClientHello(
        const Packet& packet
    );


    ServerHello decodeServerHello(
        const Packet& packet
    );

}
