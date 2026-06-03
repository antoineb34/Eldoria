#pragma once

#include "CacheExplorer.h"

namespace rf::explorer {

class AppShell {
public:
    int run();

private:
    CacheExplorer explorer_;
};

}
