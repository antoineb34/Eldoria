#pragma once

#include <cstdint>
#include <span>

#include "Message.h"

namespace eld::message {

class MessageDecoder {
public:
    Message decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
