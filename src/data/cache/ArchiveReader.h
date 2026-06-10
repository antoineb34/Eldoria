#pragma once

#include <optional>
#include <vector>

#include "Archive.h"

namespace eld::cache {

class ArchiveReader {
public:
    static std::optional<Archive> read(
        const std::vector<std::uint8_t>& cacheFilePayload
    );
};

}
