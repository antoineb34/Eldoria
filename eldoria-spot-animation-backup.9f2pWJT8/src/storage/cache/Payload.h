#pragma once

#include <cstddef>
#include <vector>

#include "Sector.h"

namespace eld::cache {

class Payload {
public:
    explicit Payload(std::vector<Sector> sectors);

    const std::vector<Sector>& getSectors() const;
    std::size_t getSectorCount() const;

private:
    std::vector<Sector> sectors_;
};

}
