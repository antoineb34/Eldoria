#pragma once

#include <cstdint>


namespace eld::protocol
{

    enum class ClientOpcode :
        std::uint8_t
    {
        Hello = 0x01,
        Walk = 0x02
    };


    enum class ServerOpcode :
        std::uint8_t
    {
        Hello = 0x01,
        PlayerPosition = 0x02
    };

}
