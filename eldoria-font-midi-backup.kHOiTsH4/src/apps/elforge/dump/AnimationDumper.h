#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "inspection/AnimationInspector.h"

namespace eld::elforge {

std::filesystem::path defaultAnimationDumpPath(
    std::uint16_t animationId
);

bool dumpAnimation(
    const AnimationInspection& inspection,
    const std::filesystem::path& path,
    std::string& error
);

}
