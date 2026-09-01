#pragma once

#include <cstddef>
#include <limits>
#include <string>
#include <vector>

namespace eld::elforge {

struct AnimationViewState {
    std::size_t selectedSequenceIndex = 0;
    static constexpr std::size_t NoActivePreview =
        std::numeric_limits<std::size_t>::max();

    // Indices into AnimationInspection::uses that currently
    // have a meaningful standalone 3D preview.
    std::vector<std::size_t> previewUseIndices;

    // Index inside previewUseIndices, not AnimationInspection::uses.
    std::size_t selectedPreviewUseIndex = 0;
    std::size_t activePreviewUseIndex = NoActivePreview;

    // Workspace chrome.
    bool detailsVisible = true;

    // Raw archive-frame inspection state.

    std::string previewStatus;
};

}
