#pragma once
#include "assets/model/Model.h"
#include "io/Buffer.h"

namespace ModelCodec {
    Model decode(Buffer& buf);
}
