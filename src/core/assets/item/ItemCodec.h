#pragma once
#include "assets/item/Item.h"
#include "io/Buffer.h"

namespace ItemCodec {
    Item decode(Buffer& buf);
}
