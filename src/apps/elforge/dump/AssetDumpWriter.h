#pragma once

#include <cstddef>
#include <filesystem>
#include <functional>
#include <iosfwd>
#include <string>

namespace eld::elforge {

bool writeAssetDump(
    const std::filesystem::path& path,
    const std::function<void(std::ostream&)>& writer,
    std::string& error
);

void writeHexDump(
    std::ostream& output,
    const void* data,
    std::size_t size
);

}
