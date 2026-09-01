#pragma once

#include <filesystem>
#include <string>

#include "AnimationRelations.h"

namespace eld::elforge {

std::filesystem::path defaultAnimationDumpPath(
    std::uint16_t animationId
);

std::filesystem::path defaultAnimationRelationsDumpPath();

bool dumpAnimationRelationsInfo(
    const AnimationRelationsInfo& info,
    const std::filesystem::path& path,
    std::string& error
);

bool dumpAllAnimationRelations(
    const AnimationRelations& relations,
    const std::filesystem::path& path,
    std::string& error
);

}
