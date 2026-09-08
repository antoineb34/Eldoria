#include "explorer/tree/CacheTreePanel.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include <imgui.h>

#include "explorer/CacheExplorerState.h"
#include "explorer/tree/CacheTreeNode.h"
#include "ui/ElForgeTheme.h"
#include "ui/PanelUi.h"

namespace eld::elforge {

namespace {

struct VisibleNode {
    const CacheTreeNode* node = nullptr;
    int depth = 0;
};


void appendVisibleNodes(
    CacheExplorerState& state,
    const CacheTreeNode& node,
    int depth,
    std::vector<VisibleNode>& output
) {
    output.push_back({
        &node,
        depth
    });

    if (node.children.empty()) {
        return;
    }

    const auto expanded =
        state.expandedNodes.find(
            node.key
        );

    if (
        expanded == state.expandedNodes.end() ||
        !expanded->second
    ) {
        return;
    }

    for (
        const CacheTreeNode& child :
        node.children
    ) {
        appendVisibleNodes(
            state,
            child,
            depth + 1,
            output
        );
    }
}


void selectNode(
    CacheExplorerState& state,
    const CacheTreeNode& node
) {
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

    state.selection.locationFileId =
        node.locationFileId;
}


std::size_t findSelectedIndex(
    const CacheExplorerState& state,
    const std::vector<VisibleNode>& visible
) {
    const auto found =
        std::find_if(
            visible.begin(),
            visible.end(),
            [&state](
                const VisibleNode& entry
            ) {
                return
                    entry.node != nullptr &&
                    entry.node->key ==
                        state.selection.key;
            }
        );

    if (found == visible.end()) {
        return visible.size();
    }

    return static_cast<std::size_t>(
        found -
        visible.begin()
    );
}


void moveSelection(
    CacheExplorerState& state,
    const std::vector<VisibleNode>& visible,
    int direction
) {
    if (visible.empty()) {
        return;
    }

    std::size_t index =
        findSelectedIndex(
            state,
            visible
        );

    if (index >= visible.size()) {
        index =
            direction >= 0
                ? 0
                : visible.size() - 1;
    }
    else if (direction > 0) {
        index =
            std::min(
                index + 1,
                visible.size() - 1
            );
    }
    else if (direction < 0) {
        index =
            index > 0
                ? index - 1
                : 0;
    }

    selectNode(
        state,
        *visible[index].node
    );
}


const char* nodeMarker(
    bool hasChildren,
    bool expanded
) {
    if (!hasChildren) {
        return "·";
    }

    return expanded
        ? "▾"
        : "▸";
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
        false
    );

    ui::panelTitle(
        "EXPLORER"
    );

    ImGui::SameLine();

    ImGui::TextDisabled(
        "  ↑↓ browse  ←→ tree"
    );

    ImGui::Separator();

    // --------------------------------------------------------
    // Build exactly what the user can currently see.
    //
    // Keyboard navigation traverses this list, not hidden
    // descendants.
    // --------------------------------------------------------

    std::vector<VisibleNode> visible;

    for (
        const CacheTreeNode& child :
        state.rootNode.children
    ) {
        appendVisibleNodes(
            state,
            child,
            0,
            visible
        );
    }

    bool keyboardMoved =
        false;

    // Only consume navigation keys while Explorer owns focus.
    const bool focused =
        ImGui::IsWindowFocused(
            ImGuiFocusedFlags_RootAndChildWindows
        );

    if (focused && !visible.empty()) {
        // `true` enables ImGui key repeat, so holding ↓ or ↑
        // continuously browses the cache.
        if (
            ImGui::IsKeyPressed(
                ImGuiKey_DownArrow,
                true
            )
        ) {
            moveSelection(
                state,
                visible,
                +1
            );

            keyboardMoved =
                true;
        }

        if (
            ImGui::IsKeyPressed(
                ImGuiKey_UpArrow,
                true
            )
        ) {
            moveSelection(
                state,
                visible,
                -1
            );

            keyboardMoved =
                true;
        }

        std::size_t selectedIndex =
            findSelectedIndex(
                state,
                visible
            );

        if (selectedIndex < visible.size()) {
            const CacheTreeNode& selected =
                *visible[
                    selectedIndex
                ].node;

            const bool hasChildren =
                !selected.children.empty();

            bool& expanded =
                state.expandedNodes[
                    selected.key
                ];

            if (
                ImGui::IsKeyPressed(
                    ImGuiKey_RightArrow
                ) &&
                hasChildren
            ) {
                expanded = true;
            }

            if (
                ImGui::IsKeyPressed(
                    ImGuiKey_LeftArrow
                ) &&
                hasChildren &&
                expanded
            ) {
                expanded = false;
            }

            if (
                ImGui::IsKeyPressed(
                    ImGuiKey_Enter
                ) &&
                hasChildren
            ) {
                expanded =
                    !expanded;
            }
        }
    }

    // Expansion could have changed above.
    // Rebuild so the rendered list always matches state.
    visible.clear();

    for (
        const CacheTreeNode& child :
        state.rootNode.children
    ) {
        appendVisibleNodes(
            state,
            child,
            0,
            visible
        );
    }

    ImGui::PushStyleVar(
        ImGuiStyleVar_ItemSpacing,
        ImVec2(
            0.0f,
            2.0f
        )
    );

    for (
        const VisibleNode& entry :
        visible
    ) {
        if (entry.node == nullptr) {
            continue;
        }

        const CacheTreeNode& node =
            *entry.node;

        const bool hasChildren =
            !node.children.empty();

        bool& expanded =
            state.expandedNodes[
                node.key
            ];

        const bool selected =
            state.selection.key ==
            node.key;

        const std::string rowText =
            std::string(
                nodeMarker(
                    hasChildren,
                    expanded
                )
            ) +
            " " +
            node.label;

        ImGui::SetCursorPosX(
            ImGui::GetCursorPosX() +
            static_cast<float>(
                entry.depth
            ) *
            15.0f
        );

        ImGui::PushID(
            node.key.c_str()
        );

        const bool clicked =
            ImGui::Selectable(
                rowText.c_str(),
                selected,
                ImGuiSelectableFlags_None,
                ImVec2(
                    0.0f,
                    24.0f
                )
            );

        if (selected) {
            const ImVec2 selectedMin =
                ImGui::GetItemRectMin();

            const ImVec2 selectedMax =
                ImGui::GetItemRectMax();

            ImGui::GetWindowDrawList()->
                AddRectFilled(
                    selectedMin,
                    ImVec2(
                        selectedMin.x + 2.0f,
                        selectedMax.y
                    ),
                    ImGui::ColorConvertFloat4ToU32(
                        ui::themePalette().primary
                    )
                );
        }

        if (
            selected &&
            keyboardMoved
        ) {
            ImGui::SetScrollHereY(
                0.5f
            );
        }

        if (
            ImGui::IsItemHovered() &&
            !node.label.empty()
        ) {
            ImGui::SetTooltip(
                "%s",
                node.label.c_str()
            );
        }

        ImGui::PopID();

        if (clicked) {
            selectNode(
                state,
                node
            );

            if (hasChildren) {
                expanded =
                    !expanded;
            }
        }
    }

    ImGui::PopStyleVar();

    ImGui::EndChild();
}

}
