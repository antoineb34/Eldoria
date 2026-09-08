#pragma once

#include <filesystem>
#include <string>

#include "inspection/AnimationInspector.h"

namespace eld::elforge {

std::filesystem::path defaultAnimationExportPath(
    std::uint16_t animationId
);

std::filesystem::path defaultAnimationRelationsExportPath();

bool exportAnimationInspection(
    const AnimationInspection& info,
    const std::filesystem::path& path,
    std::string& error
);

bool exportAllAnimationInspections(
    const AnimationInspector& relations,
    const std::filesystem::path& path,
    std::string& error
);

}
