#include "CacheTreePanel.h"

#include <imgui.h>

#include <string>

#include "CacheState.h"
#include "CacheTreeNode.h"

namespace eldoria::apps::elforge {

namespace {

const char* getNodeIcon(
    bool hasChildren,
    bool expanded
) {
    if (!hasChildren) {
        return "•";
    }

    return expanded
        ? "◆"
        : "◇";
}

}

void CacheTreePanel::render(
    CacheState& state,
    float width,
    float height
) {
    ImGui::BeginChild(
        "CacheTreePanel",
        ImVec2(width, height),
        true
    );

    ImGui::TextUnformatted("CACHE");
    ImGui::Separator();

    ImGui::PushStyleVar(
        ImGuiStyleVar_ItemSpacing,
        ImVec2(0.0f, 4.0f)
    );

    for (const CacheTreeNode& child : state.rootNode.children) {
        renderNode(
            state,
            child,
            0
        );
    }

    ImGui::PopStyleVar();

    ImGui::EndChild();
}

void CacheTreePanel::renderNode(
    CacheState& state,
    const CacheTreeNode& node,
    int depth
) {
    const bool hasChildren =
        !node.children.empty();

    bool& expanded =
        state.expandedNodes[node.label];

    const bool selected =
        state.selection.label == node.label;

    const char* icon =
        getNodeIcon(
            hasChildren,
            expanded
        );

    std::string rowText =
        std::string(icon) +
        " " +
        node.label;

    ImGui::SetCursorPosX(
        ImGui::GetCursorPosX() + depth * 18.0f
    );

    bool clicked =
        ImGui::Selectable(
            rowText.c_str(),
            selected
        );

    if (clicked) {
        state.selection.type =
            node.type;

        state.selection.label =
            node.label;

        state.selection.indexId =
            node.indexId;

        state.selection.archiveId =
            node.archiveId;

        state.selection.fileId =
            node.fileId;

        if (hasChildren) {
            expanded =
                !expanded;
        }
    }

    if (
        hasChildren &&
        expanded
    ) {
        for (const CacheTreeNode& child : node.children) {
            renderNode(
                state,
                child,
                depth + 1
            );
        }
    }
}

}
