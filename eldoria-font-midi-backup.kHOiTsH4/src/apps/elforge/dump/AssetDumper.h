#pragma once

#include <filesystem>
#include <string>

namespace eld::elforge {

struct CacheExplorerState;

std::filesystem::path defaultAssetDumpPath(
    const CacheExplorerState& state
);

bool dumpActiveAsset(
    const CacheExplorerState& state,
    const std::filesystem::path& path,
    std::string& error
);

}
