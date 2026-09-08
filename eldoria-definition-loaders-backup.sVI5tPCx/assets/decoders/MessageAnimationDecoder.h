#pragma once

#include <cstdint>
#include <span>

#include "MessageAnimation.h"

namespace eld::message_animation {

class MessageAnimationDecoder {
public:
    MessageAnimation decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
