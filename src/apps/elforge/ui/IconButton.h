#pragma once

#include <imgui.h>

namespace eld::elforge::ui {

enum class Icon {
    Play,
    Pause,
    StepBackward,
    StepForward,
    ChevronLeft,
    ChevronRight,

    Move,
    Rotate,
    Scale,
    Grid,
    Focus,

    Info,
    Download,
    Restart
};

bool iconButton(
    const char* id,
    Icon icon,
    const char* tooltip,
    ImVec2 size = ImVec2(30.0f, 28.0f),
    bool selected = false
);

}
