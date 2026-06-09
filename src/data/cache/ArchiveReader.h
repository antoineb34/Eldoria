#pragma once

#include <optional>
#include <vector>

#include "Archive.h"

namespace rf::cache {

class ArchiveReader {
public:
    static std::optional<Archive> read(
        const std::vector<unsigned char>& payload
    );
};

}
