#pragma once

#include "CacheExplorer.h"

namespace eld::explorer {

class AppShell {
public:
    int run();

private:
    CacheExplorer explorer_;
};

}
