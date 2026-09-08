#include "message_animation/MessageAnimationDecoder.h"

#include <cstdint>
#include <span>
#include <stdexcept>

#include "binary/ByteReader.h"

namespace eld::message_animation {

MessageAnimationData
MessageAnimationDecoder::decode(std::span<const std::uint8_t> payload) const {
  eld::binary::ByteReader reader(payload);

  if (reader.readU8() != 0 || !reader.atEnd()) {
    throw std::runtime_error("Invalid message-animation payload");
  }

  return {};
}

} // namespace eld::message_animation
