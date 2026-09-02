#pragma once

#include "explorer/CacheExplorer.h"

namespace eld::elforge {

class ElForgeApp {
public:
    int run();

private:
    CacheExplorer explorer_;
};

}
