#include "CacheInspectorPanel.h"

#include <array>
#include <string>

#include <imgui.h>

#include "../CacheExplorerState.h"

namespace eld::elforge {

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

        case CacheTreeNodeType::Archive:
            return "Archive";

        case CacheTreeNodeType::ArchiveFile:
            return "Archive File";

        case CacheTreeNodeType::Sprite:
            return "Sprite";

        case CacheTreeNodeType::DefinitionGroup:
            return "Definition Group";

        case CacheTreeNodeType::FloorDefinition:
            return "Floor Definition";

        case CacheTreeNodeType::IdentityKitDefinition:
            return "Identity Kit";

        case CacheTreeNodeType::Font:
            return "Font";

        case CacheTreeNodeType::Image:
            return "Image";

        case CacheTreeNodeType::SpriteFrame:
            return "Sprite Frame";
    }

    return "Unknown";
}

std::string buildModelDebugText(
    const CacheExplorerState& state,
    const eld::model::Model& model
) {
    const eld::model::ModelMesh& asset =
        model.mesh;

    std::array<int, 12> priorityCounts {};
    std::array<int, 4> renderTypeCounts {};

    int alphaFaces = 0;
    int texturedFaces = 0;

    for (const eld::model::Face& face : asset.faces) {
        if (face.priority < priorityCounts.size()) {
            priorityCounts[face.priority]++;
        }

        if (face.renderType < renderTypeCounts.size()) {
            renderTypeCounts[face.renderType]++;
        }

        if (face.alpha > 0) {
            alphaFaces++;
        }

        if (face.textureId.has_value()) {
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
        std::to_string(asset.vertices.size()) +
        "\n";

    debug += "Faces: " +
        std::to_string(asset.faces.size()) +
        "\n";

    debug += "Texture mappings: " +
        std::to_string(asset.textureMappings.size()) +
        "\n";

    debug += "Source vertices: " +
        std::to_string(model.sourceMap.vertices.size()) +
        "\n";

    debug += "Source faces: " +
        std::to_string(model.sourceMap.faces.size()) +
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

    if (state.activeIdentityKit.has_value()) {
        const eld::definition::IdentityKitDefinition& kit =
            *state.activeIdentityKit;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("IDENTITY KIT");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(kit.id)
        );

        if (kit.bodyPartId.has_value()) {
            ImGui::Text(
                "Body part: %u",
                static_cast<unsigned int>(*kit.bodyPartId)
            );
        }
        else {
            ImGui::TextUnformatted("Body part: (none)");
        }

        ImGui::Text(
            "Selectable: %s",
            kit.selectable ? "yes" : "no"
        );

        ImGui::Text(
            "Body models: %zu",
            kit.modelIds.size()
        );

        for (const std::uint16_t modelId : kit.modelIds) {
            ImGui::BulletText(
                "Model %u",
                static_cast<unsigned int>(modelId)
            );
        }

        ImGui::TextUnformatted("Recolors:");

        bool hasRecolors = false;

        for (
            std::size_t index = 0;
            index < kit.recolorSources.size();
            ++index
        ) {
            if (
                kit.recolorSources[index].has_value() &&
                kit.recolorDestinations[index].has_value()
            ) {
                hasRecolors = true;

                ImGui::BulletText(
                    "%u -> %u",
                    static_cast<unsigned int>(
                        *kit.recolorSources[index]
                    ),
                    static_cast<unsigned int>(
                        *kit.recolorDestinations[index]
                    )
                );
            }
        }

        if (!hasRecolors) {
            ImGui::TextUnformatted("(none)");
        }

        ImGui::TextUnformatted("Head models:");

        bool hasHeadModels = false;

        for (
            const std::optional<std::uint16_t>& modelId :
            kit.headModelIds
        ) {
            if (modelId.has_value()) {
                hasHeadModels = true;

                ImGui::BulletText(
                    "Model %u",
                    static_cast<unsigned int>(*modelId)
                );
            }
        }

        if (!hasHeadModels) {
            ImGui::TextUnformatted("(none)");
        }
    }

    if (state.activeFloor.has_value()) {
        const eld::definition::FloorDefinition& floor =
            *state.activeFloor;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("FLOOR");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(
                floor.id
            )
        );

        ImGui::Text(
            "Name: %s",
            floor.name.empty()
                ? "(none)"
                : floor.name.c_str()
        );

        if (floor.rgb.has_value()) {
            const std::uint32_t rgb =
                *floor.rgb;

            ImGui::Text(
                "Primary RGB: #%06X",
                static_cast<unsigned int>(
                    rgb
                )
            );

            ImGui::ColorButton(
                "##FloorPrimaryColor",
                ImVec4(
                    static_cast<float>(
                        (rgb >> 16) & 0xFF
                    ) / 255.0f,
                    static_cast<float>(
                        (rgb >> 8) & 0xFF
                    ) / 255.0f,
                    static_cast<float>(
                        rgb & 0xFF
                    ) / 255.0f,
                    1.0f
                ),
                ImGuiColorEditFlags_NoTooltip,
                ImVec2(80.0f, 32.0f)
            );
        }
        else {
            ImGui::TextUnformatted(
                "Primary RGB: (none)"
            );
        }

        if (floor.textureId.has_value()) {
            ImGui::Text(
                "Texture ID: %u",
                static_cast<unsigned int>(
                    *floor.textureId
                )
            );
        }
        else {
            ImGui::TextUnformatted(
                "Texture ID: (none)"
            );
        }

        if (floor.secondaryRgb.has_value()) {
            const std::uint32_t rgb =
                *floor.secondaryRgb;

            ImGui::Text(
                "Secondary RGB: #%06X",
                static_cast<unsigned int>(
                    rgb
                )
            );

            ImGui::ColorButton(
                "##FloorSecondaryColor",
                ImVec4(
                    static_cast<float>(
                        (rgb >> 16) & 0xFF
                    ) / 255.0f,
                    static_cast<float>(
                        (rgb >> 8) & 0xFF
                    ) / 255.0f,
                    static_cast<float>(
                        rgb & 0xFF
                    ) / 255.0f,
                    1.0f
                ),
                ImGuiColorEditFlags_NoTooltip,
                ImVec2(80.0f, 32.0f)
            );
        }
        else {
            ImGui::TextUnformatted(
                "Secondary RGB: (none)"
            );
        }

        ImGui::Text(
            "Occlude: %s",
            floor.occlude
                ? "true"
                : "false"
        );
    }

    if (state.activeModel) {
        const eld::model::Model& model =
            *state.activeModel;

        const eld::model::ModelMesh& asset =
            model.mesh;

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
            asset.vertices.size()
        );

        ImGui::Text(
            "Faces: %zu",
            asset.faces.size()
        );

        ImGui::Text(
            "Texture mappings: %zu",
            asset.textureMappings.size()
        );

        ImGui::Text(
            "Source vertices: %zu",
            model.sourceMap.vertices.size()
        );

        ImGui::Text(
            "Source faces: %zu",
            model.sourceMap.faces.size()
        );

        std::array<int, 12> priorityCounts {};

        for (const eld::model::Face& face : asset.faces) {
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
