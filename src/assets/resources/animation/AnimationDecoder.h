#pragma once

#include "AnimationAsset.h"
#include "AnimationFile.h"

namespace eld::animation {

class AnimationDecoder {
public:
    AnimationAsset decode(
        const AnimationFile& file
    ) const;
};

}
