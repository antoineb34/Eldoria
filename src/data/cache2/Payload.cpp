#include "Payload.h"

#include <utility>

namespace eld::cache {

Payload::Payload(std::vector<Sector> sectors)
    : sectors_(std::move(sectors)) {
}

const std::vector<Sector>& Payload::getSectors() const {
    return sectors_;
}

std::size_t Payload::getSectorCount() const {
    return sectors_.size();
}

}
