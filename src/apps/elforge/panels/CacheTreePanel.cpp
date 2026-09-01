#include "CacheTreePanel.h"

#include <imgui.h>

#include <string>

#include "../CacheExplorerState.h"
#include "../CacheTreeNode.h"

namespace eld::elforge {

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
    CacheExplorerState& state,
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

    for (
        const CacheTreeNode& child :
        state.rootNode.children
    ) {
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
    CacheExplorerState& state,
    const CacheTreeNode& node,
    int depth
) {
    const bool hasChildren =
        !node.children.empty();

    bool& expanded =
        state.expandedNodes[node.key];

    const bool selected =
        state.selection.key == node.key;

    const char* icon =
        getNodeIcon(
            hasChildren,
            expanded
        );

    const std::string rowText =
        std::string(icon) +
        " " +
        node.label;

    ImGui::SetCursorPosX(
        ImGui::GetCursorPosX() +
        depth * 18.0f
    );

    ImGui::PushID(
        node.key.c_str()
    );

    const bool clicked =
        ImGui::Selectable(
            rowText.c_str(),
            selected
        );

    ImGui::PopID();

    if (clicked) {
        state.selection.type =
            node.type;

        state.selection.key =
            node.key;

        state.selection.label =
            node.label;

        state.selection.indexId =
            node.indexId;

        state.selection.archiveId =
            node.archiveId;

        state.selection.fileId =
            node.fileId;

        state.selection.name =
            node.name;

        state.selection.frameId =
            node.frameId;

        state.selection.definitionId =
            node.definitionId;

        state.selection.regionId =
            node.regionId;

        state.selection.terrainFileId =
            node.terrainFileId;

        state.selection.objectFileId =
            node.objectFileId;

        if (hasChildren) {
            expanded =
                !expanded;
        }
    }

    if (
        hasChildren &&
        expanded
    ) {
        for (
            const CacheTreeNode& child :
            node.children
        ) {
            renderNode(
                state,
                child,
                depth + 1
            );
        }
    }
}

}
