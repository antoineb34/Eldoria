#pragma once

#include "item/ItemData.h"
#include "item/ItemResource.h"

namespace eld::item {

class ItemAssembler {
public:
  ItemResource assemble(ItemData data) const;
};

} // namespace eld::item
