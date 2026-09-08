#pragma once

#include <cstdint>
#include <span>

#include "message/MessageData.h"

namespace eld::message {

class MessageDecoder {
public:
  MessageData decode(std::span<const std::uint8_t> payload) const;
};

} // namespace eld::message
