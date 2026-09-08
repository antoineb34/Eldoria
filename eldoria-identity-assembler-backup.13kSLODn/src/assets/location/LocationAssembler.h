#pragma once

#include "location/LocationData.h"
#include "location/LocationResource.h"

namespace eld::location {

class LocationAssembler {
public:
  LocationResource assemble(LocationData data) const;
};

} // namespace eld::location
