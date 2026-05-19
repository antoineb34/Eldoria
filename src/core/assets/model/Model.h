#pragma once
#include <cstdint>
#include <vector>

struct Model {
    std::vector<int16_t> vertX, vertY, vertZ;
    std::vector<int>     triA, triB, triC;
    std::vector<int>     triColor;
    // …extend as you decode more fields
};
