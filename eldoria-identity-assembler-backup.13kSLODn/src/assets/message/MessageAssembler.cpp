#include "message/MessageAssembler.h"

#include <utility>

namespace eld::message {

MessageResource MessageAssembler::assemble(MessageData data) const {
  return MessageResource{
      .data = std::move(data),
  };
}

} // namespace eld::message
