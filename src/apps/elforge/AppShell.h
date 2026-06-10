#pragma once

#include "CacheExplorer.h"

namespace eld::elforge {

class AppShell {
public:
    int run();

private:
    CacheExplorer explorer_;
};

}
