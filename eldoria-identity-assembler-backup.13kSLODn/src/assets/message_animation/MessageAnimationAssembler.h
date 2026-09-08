#pragma once

#include "message_animation/MessageAnimationData.h"
#include "message_animation/MessageAnimationResource.h"

namespace eld::message_animation {

class MessageAnimationAssembler {
public:
  MessageAnimationResource assemble(MessageAnimationData data) const;
};

} // namespace eld::message_animation
