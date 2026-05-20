#pragma once
#include <string>
#include <vector>

struct Item {
    std::string name;
    int value = 0;
    int equipSlot = -1;
    std::vector<int> modelIds;
    // …extend as you decode more fields
};
