#include "ui/IconButton.h"

#include <algorithm>

namespace eld::elforge::ui {

namespace {

void drawPlay(
    ImDrawList* d,
    ImVec2 c,
    float r,
    ImU32 color
) {
    d->AddTriangleFilled(
        ImVec2(c.x - r * 0.38f, c.y - r * 0.58f),
        ImVec2(c.x - r * 0.38f, c.y + r * 0.58f),
        ImVec2(c.x + r * 0.55f, c.y),
        color
    );
}


void drawPause(
    ImDrawList* d,
    ImVec2 c,
    float r,
    ImU32 color
) {
    const float w = r * 0.22f;

    d->AddRectFilled(
        ImVec2(c.x - r * 0.42f, c.y - r * 0.58f),
        ImVec2(c.x - r * 0.42f + w, c.y + r * 0.58f),
        color
    );

    d->AddRectFilled(
        ImVec2(c.x + r * 0.18f, c.y - r * 0.58f),
        ImVec2(c.x + r * 0.18f + w, c.y + r * 0.58f),
        color
    );
}


void drawChevron(
    ImDrawList* d,
    ImVec2 c,
    float r,
    ImU32 color,
    bool right
) {
    const float direction = right ? 1.0f : -1.0f;

    d->AddLine(
        ImVec2(c.x - direction * r * 0.30f, c.y - r * 0.48f),
        ImVec2(c.x + direction * r * 0.24f, c.y),
        color,
        2.0f
    );

    d->AddLine(
        ImVec2(c.x + direction * r * 0.24f, c.y),
        ImVec2(c.x - direction * r * 0.30f, c.y + r * 0.48f),
        color,
        2.0f
    );
}


void drawStep(
    ImDrawList* d,
    ImVec2 c,
    float r,
    ImU32 color,
    bool forward
) {
    const float direction = forward ? 1.0f : -1.0f;

    const float barX =
        c.x + direction * r * 0.52f;

    d->AddLine(
        ImVec2(barX, c.y - r * 0.55f),
        ImVec2(barX, c.y + r * 0.55f),
        color,
        2.0f
    );

    d->AddTriangleFilled(
        ImVec2(c.x + direction * r * 0.32f, c.y),
        ImVec2(c.x - direction * r * 0.38f, c.y - r * 0.52f),
        ImVec2(c.x - direction * r * 0.38f, c.y + r * 0.52f),
        color
    );
}


void drawRestart(
    ImDrawList* d,
    ImVec2 c,
    float r,
    ImU32 color
) {
    d->PathArcTo(
        c,
        r * 0.58f,
        -2.70f,
        2.15f,
        18
    );

    d->PathStroke(
        color,
        false,
        2.0f
    );

    d->AddTriangleFilled(
        ImVec2(c.x - r * 0.70f, c.y - r * 0.12f),
        ImVec2(c.x - r * 0.24f, c.y - r * 0.52f),
        ImVec2(c.x - r * 0.12f, c.y + r * 0.04f),
        color
    );
}


void drawInfo(
    ImDrawList* d,
    ImVec2 c,
    float r,
    ImU32 color
) {
    d->AddCircle(
        c,
        r * 0.70f,
        color,
        20,
        1.8f
    );

    d->AddCircleFilled(
        ImVec2(c.x, c.y - r * 0.30f),
        std::max(1.1f, r * 0.09f),
        color
    );

    d->AddLine(
        ImVec2(c.x, c.y - r * 0.02f),
        ImVec2(c.x, c.y + r * 0.40f),
        color,
        2.0f
    );
}


void drawDownload(
    ImDrawList* d,
    ImVec2 c,
    float r,
    ImU32 color
) {
    d->AddLine(
        ImVec2(c.x, c.y - r * 0.62f),
        ImVec2(c.x, c.y + r * 0.18f),
        color,
        2.0f
    );

    d->AddTriangleFilled(
        ImVec2(c.x, c.y + r * 0.55f),
        ImVec2(c.x - r * 0.36f, c.y + r * 0.12f),
        ImVec2(c.x + r * 0.36f, c.y + r * 0.12f),
        color
    );

    d->AddLine(
        ImVec2(c.x - r * 0.55f, c.y + r * 0.68f),
        ImVec2(c.x + r * 0.55f, c.y + r * 0.68f),
        color,
        2.0f
    );
}


void drawMove(
    ImDrawList* d,
    ImVec2 c,
    float r,
    ImU32 color
) {
    d->AddLine(
        ImVec2(c.x - r * 0.62f, c.y),
        ImVec2(c.x + r * 0.62f, c.y),
        color,
        1.8f
    );

    d->AddLine(
        ImVec2(c.x, c.y - r * 0.62f),
        ImVec2(c.x, c.y + r * 0.62f),
        color,
        1.8f
    );

    d->AddTriangleFilled(
        ImVec2(c.x + r * 0.72f, c.y),
        ImVec2(c.x + r * 0.40f, c.y - r * 0.20f),
        ImVec2(c.x + r * 0.40f, c.y + r * 0.20f),
        color
    );

    d->AddTriangleFilled(
        ImVec2(c.x, c.y - r * 0.72f),
        ImVec2(c.x - r * 0.20f, c.y - r * 0.40f),
        ImVec2(c.x + r * 0.20f, c.y - r * 0.40f),
        color
    );
}


void drawRotate(
    ImDrawList* d,
    ImVec2 c,
    float r,
    ImU32 color
) {
    d->PathArcTo(
        c,
        r * 0.58f,
        -2.50f,
        2.25f,
        18
    );

    d->PathStroke(
        color,
        false,
        2.0f
    );

    d->AddTriangleFilled(
        ImVec2(c.x - r * 0.66f, c.y - r * 0.08f),
        ImVec2(c.x - r * 0.28f, c.y - r * 0.48f),
        ImVec2(c.x - r * 0.14f, c.y + r * 0.02f),
        color
    );
}


void drawScale(
    ImDrawList* d,
    ImVec2 c,
    float r,
    ImU32 color
) {
    d->AddLine(
        ImVec2(c.x - r * 0.46f, c.y + r * 0.46f),
        ImVec2(c.x + r * 0.46f, c.y - r * 0.46f),
        color,
        2.0f
    );

    const float box = r * 0.20f;

    d->AddRect(
        ImVec2(
            c.x + r * 0.46f - box,
            c.y - r * 0.46f - box
        ),
        ImVec2(
            c.x + r * 0.46f + box,
            c.y - r * 0.46f + box
        ),
        color,
        1.0f,
        0,
        1.8f
    );

    d->AddRectFilled(
        ImVec2(
            c.x - r * 0.46f - box,
            c.y + r * 0.46f - box
        ),
        ImVec2(
            c.x - r * 0.46f + box,
            c.y + r * 0.46f + box
        ),
        color
    );
}


void drawGrid(
    ImDrawList* d,
    ImVec2 c,
    float r,
    ImU32 color
) {
    constexpr float positions[] = {
        -0.48f,
        0.0f,
        0.48f
    };

    for (float position : positions) {
        d->AddLine(
            ImVec2(c.x + r * position, c.y - r * 0.58f),
            ImVec2(c.x + r * position, c.y + r * 0.58f),
            color,
            1.3f
        );

        d->AddLine(
            ImVec2(c.x - r * 0.58f, c.y + r * position),
            ImVec2(c.x + r * 0.58f, c.y + r * position),
            color,
            1.3f
        );
    }
}


void drawFocus(
    ImDrawList* d,
    ImVec2 c,
    float r,
    ImU32 color
) {
    d->AddCircle(
        c,
        r * 0.44f,
        color,
        16,
        1.8f
    );

    d->AddCircleFilled(
        c,
        r * 0.10f,
        color
    );

    d->AddLine(
        ImVec2(c.x, c.y - r * 0.72f),
        ImVec2(c.x, c.y - r * 0.48f),
        color,
        1.8f
    );

    d->AddLine(
        ImVec2(c.x, c.y + r * 0.48f),
        ImVec2(c.x, c.y + r * 0.72f),
        color,
        1.8f
    );

    d->AddLine(
        ImVec2(c.x - r * 0.72f, c.y),
        ImVec2(c.x - r * 0.48f, c.y),
        color,
        1.8f
    );

    d->AddLine(
        ImVec2(c.x + r * 0.48f, c.y),
        ImVec2(c.x + r * 0.72f, c.y),
        color,
        1.8f
    );
}

}


bool iconButton(
    const char* id,
    Icon icon,
    const char* tooltip,
    ImVec2 size,
    bool selected
) {
    ImGui::PushID(
        id
    );

    if (selected) {
        ImGui::PushStyleColor(
            ImGuiCol_Button,
            ImGui::GetStyleColorVec4(
                ImGuiCol_Header
            )
        );
    }

    const bool pressed =
        ImGui::Button(
            "##IconButton",
            size
        );

    if (selected) {
        ImGui::PopStyleColor();
    }

    const ImVec2 minimum =
        ImGui::GetItemRectMin();

    const ImVec2 maximum =
        ImGui::GetItemRectMax();

    const ImVec2 center{
        (minimum.x + maximum.x) * 0.5f,
        (minimum.y + maximum.y) * 0.5f
    };

    const float radius =
        std::min(
            maximum.x - minimum.x,
            maximum.y - minimum.y
        ) *
        0.34f;

    ImDrawList* drawList =
        ImGui::GetWindowDrawList();

    const ImU32 color =
        ImGui::GetColorU32(
            ImGuiCol_Text
        );

    switch (icon) {
        case Icon::Play:
            drawPlay(drawList, center, radius, color);
            break;

        case Icon::Pause:
            drawPause(drawList, center, radius, color);
            break;

        case Icon::StepBackward:
            drawStep(drawList, center, radius, color, false);
            break;

        case Icon::StepForward:
            drawStep(drawList, center, radius, color, true);
            break;

        case Icon::ChevronLeft:
            drawChevron(drawList, center, radius, color, false);
            break;

        case Icon::ChevronRight:
            drawChevron(drawList, center, radius, color, true);
            break;

        case Icon::Move:
            drawMove(drawList, center, radius, color);
            break;

        case Icon::Rotate:
            drawRotate(drawList, center, radius, color);
            break;

        case Icon::Scale:
            drawScale(drawList, center, radius, color);
            break;

        case Icon::Grid:
            drawGrid(drawList, center, radius, color);
            break;

        case Icon::Focus:
            drawFocus(drawList, center, radius, color);
            break;

        case Icon::Info:
            drawInfo(drawList, center, radius, color);
            break;

        case Icon::Download:
            drawDownload(drawList, center, radius, color);
            break;

        case Icon::Restart:
            drawRestart(drawList, center, radius, color);
            break;
    }

    if (
        tooltip != nullptr &&
        tooltip[0] != '\0' &&
        ImGui::IsItemHovered(
            ImGuiHoveredFlags_DelayShort
        )
    ) {
        ImGui::SetTooltip(
            "%s",
            tooltip
        );
    }

    ImGui::PopID();

    return pressed;
}

}
