#include "location/LocationAssembler.h"

#include <utility>

namespace eld::location {

LocationResource LocationAssembler::assemble(LocationData data) const {
  return LocationResource{
      .data = std::move(data),
  };
}

} // namespace eld::location
