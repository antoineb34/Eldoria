#pragma once

#include <cstdint>
#include <span>

#include "message_animation/MessageAnimationData.h"

namespace eld::message_animation {

class MessageAnimationDecoder {
public:
  MessageAnimationData decode(std::span<const std::uint8_t> payload) const;
};

} // namespace eld::message_animation
