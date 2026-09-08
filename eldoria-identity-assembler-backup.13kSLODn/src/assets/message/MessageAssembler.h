#pragma once

#include "message/MessageData.h"
#include "message/MessageResource.h"

namespace eld::message {

class MessageAssembler {
public:
  MessageResource assemble(MessageData data) const;
};

} // namespace eld::message
