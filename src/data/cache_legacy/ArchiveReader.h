#pragma once

#include <optional>
#include <vector>

#include "Archive.h"

namespace eld::cache_legacy {

class ArchiveReader {
public:
    static std::optional<Archive> read(
        const std::vector<std::uint8_t>& cacheFilePayload
    );
};

}
