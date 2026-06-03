#include "CacheInspectorPanel.h"

#include <array>
#include <string>

#include <imgui.h>

#include "../CacheExplorerState.h"

namespace rf::explorer {

namespace {

const char* getNodeTypeName(
    CacheTreeNodeType type
) {
    switch (type) {
        case CacheTreeNodeType::Root:
            return "Root";

        case CacheTreeNodeType::Index:
            return "Index";

        case CacheTreeNodeType::File:
            return "File";

        case CacheTreeNodeType::Model:
            return "Model";

        case CacheTreeNodeType::Texture:
            return "Texture";
    }

    return "Unknown";
}

std::string buildModelDebugText(
    const CacheExplorerState& state,
    const rf::model::ModelAsset& model
) {
    std::array<int, 12> priorityCounts {};
    std::array<int, 4> renderTypeCounts {};

    int alphaFaces = 0;
    int texturedFaces = 0;

    for (const rf::model::Face& face : model.faces) {
        if (face.priority < priorityCounts.size()) {
            priorityCounts[face.priority]++;
        }

        if (face.renderType < renderTypeCounts.size()) {
            renderTypeCounts[face.renderType]++;
        }

        if (face.alpha > 0) {
            alphaFaces++;
        }

        if (face.texturePointer >= 0) {
            texturedFaces++;
        }
    }

    std::string debug;

    debug += "Model Debug\n";
    debug += "===========\n";

    debug += "Selected: " +
        state.selection.label +
        "\n";

    debug += "Type: " +
        std::string(getNodeTypeName(state.selection.type)) +
        "\n";

    debug += "Index: " +
        std::to_string(state.selection.indexId) +
        "\n";

    debug += "Archive: " +
        std::to_string(state.selection.archiveId) +
        "\n";

    debug += "File: " +
        std::to_string(state.selection.fileId) +
        "\n\n";

    debug += "Vertices: " +
        std::to_string(model.vertices.size()) +
        "\n";

    debug += "Faces: " +
        std::to_string(model.faces.size()) +
        "\n";

    debug += "Texture UV mappings: " +
        std::to_string(model.textureUVMappings.size()) +
        "\n";

    debug += "Loaded textures: " +
        std::to_string(model.textures.size()) +
        "\n";

    debug += "Alpha faces: " +
        std::to_string(alphaFaces) +
        "\n";

    debug += "Textured faces: " +
        std::to_string(texturedFaces) +
        "\n\n";

    debug += "Priority Buckets\n";
    debug += "----------------\n";

    for (size_t i = 0; i < priorityCounts.size(); i++) {
        debug += "Priority " +
            std::to_string(i) +
            ": " +
            std::to_string(priorityCounts[i]) +
            "\n";
    }

    debug += "\nRender Type Buckets\n";
    debug += "-------------------\n";

    for (size_t i = 0; i < renderTypeCounts.size(); i++) {
        debug += "RenderType " +
            std::to_string(i) +
            ": " +
            std::to_string(renderTypeCounts[i]) +
            "\n";
    }

    return debug;
}

}

void CacheInspectorPanel::render(
    CacheExplorerState& state,
    float width,
    float height
) {
    ImGui::BeginChild(
        "CacheInspectorPanel",
        ImVec2(width, height),
        true
    );

    ImGui::TextUnformatted("INSPECTOR");
    ImGui::Separator();

    ImGui::Text(
        "Selected: %s",
        state.selection.label.c_str()
    );

    ImGui::Spacing();

    ImGui::Text(
        "Type: %s",
        getNodeTypeName(
            state.selection.type
        )
    );

    ImGui::Text(
        "Index: %d",
        state.selection.indexId
    );

    ImGui::Text(
        "Archive: %d",
        state.selection.archiveId
    );

    ImGui::Text(
        "File: %d",
        state.selection.fileId
    );

    if (state.activeModel) {
        const rf::model::ModelAsset& model =
            *state.activeModel;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("MODEL");

        if (ImGui::Button("Copy Model Debug")) {
            std::string debug =
                buildModelDebugText(
                    state,
                    model
                );

            ImGui::SetClipboardText(
                debug.c_str()
            );
        }

        ImGui::Text(
            "Vertices: %zu",
            model.vertices.size()
        );

        ImGui::Text(
            "Faces: %zu",
            model.faces.size()
        );

        ImGui::Text(
            "Texture UV mappings: %zu",
            model.textureUVMappings.size()
        );

        ImGui::Text(
            "Loaded textures: %zu",
            model.textures.size()
        );

        std::array<int, 12> priorityCounts {};

        for (const rf::model::Face& face : model.faces) {
            if (face.priority < priorityCounts.size()) {
                priorityCounts[face.priority]++;
            }
        }

        ImGui::Spacing();
        ImGui::TextUnformatted("Priority Buckets");

        for (size_t i = 0; i < priorityCounts.size(); i++) {
            ImGui::Text(
                "%zu: %d",
                i,
                priorityCounts[i]
            );
        }
    }

    ImGui::EndChild();
}

}
