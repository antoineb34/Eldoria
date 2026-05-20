#pragma once

#include "io/Buffer.h"

struct MesDef {
    int id = -1;

    static MesDef parse(Buffer& buf);
};
