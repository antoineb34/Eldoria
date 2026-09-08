#include "message_animation/MessageAnimationAssembler.h"

#include <utility>

namespace eld::message_animation {

MessageAnimationResource
MessageAnimationAssembler::assemble(MessageAnimationData data) const {
  return MessageAnimationResource{
      .data = std::move(data),
  };
}

} // namespace eld::message_animation
