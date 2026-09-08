#include "item/ItemAssembler.h"

#include <utility>

namespace eld::item {

ItemResource ItemAssembler::assemble(ItemData data) const {
  return ItemResource{
      .data = std::move(data),
  };
}

} // namespace eld::item
