#pragma once

#include <optional>
#include <vector>

#include "../Archive.h"

namespace rf::cache::versionlist {

struct VersionList {
    Archive archive;
};

class VersionListReader {
public:
    static std::optional<VersionList> read(
        const std::vector<unsigned char>& payload
    );
};

}
