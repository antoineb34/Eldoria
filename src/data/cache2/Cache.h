#pragma once

#include <filesystem>
#include <vector>

#include "Index.h"
#include "Store.h"

namespace eld::cache {

class Cache {
public:
    explicit Cache(
        const std::filesystem::path& rootPath
    );

    Store open(
        IndexId indexId
    ) const;

private:
    std::filesystem::path dataPath_;
    std::vector<Index> indexes_;
};

}
