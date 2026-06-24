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

        case CacheTreeNodeType::LocationDefinition:
            return "Location";

        case CacheTreeNodeType::NpcDefinition:
            return "NPC";

        case CacheTreeNodeType::ItemDefinition:
            return "Item";

        case CacheTreeNodeType::SequenceDefinition:
            return "Sequence";

        case CacheTreeNodeType::SpotAnimationDefinition:
            return "Spot Animation";

        case CacheTreeNodeType::VarpDefinition:
            return "Varp";

        case CacheTreeNodeType::VarbitDefinition:
            return "Varbit";

        case CacheTreeNodeType::ParameterDefinition:
            return "Parameter";

        case CacheTreeNodeType::MessageDefinition:
            return "Message";

        case CacheTreeNodeType::MessageAnimationDefinition:
            return "Message Animation";

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

    if (state.activeMessage.has_value()) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("MESSAGE");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(
                state.activeMessage->id
            )
        );

        ImGui::TextUnformatted(
            "This cache contains no message fields."
        );
    }

    if (state.activeMessageAnimation.has_value()) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("MESSAGE ANIMATION");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(
                state.activeMessageAnimation->id
            )
        );

        ImGui::TextUnformatted(
            "This cache contains no animation fields."
        );
    }

    if (state.activeParameter.has_value()) {
        const auto& parameter =
            *state.activeParameter;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("PARAMETER");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(parameter.id)
        );

        if (parameter.type.has_value()) {
            ImGui::Text(
                "Type: %c",
                *parameter.type
            );
        }
        else {
            ImGui::TextUnformatted("Type: (none)");
        }

        if (parameter.isString()) {
            ImGui::Text(
                "Default: %s",
                parameter.defaultString.c_str()
            );
        }
        else {
            ImGui::Text(
                "Default: %d",
                parameter.defaultInteger
            );
        }

        ImGui::Text(
            "Auto disable: %s",
            parameter.autoDisable ? "yes" : "no"
        );
    }

    if (state.activeVarbit.has_value()) {
        const auto& varbit =
            *state.activeVarbit;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("VARBIT");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(varbit.id)
        );

        ImGui::Text(
            "Name: %s",
            varbit.name.empty()
                ? "(none)"
                : varbit.name.c_str()
        );

        if (varbit.varpId.has_value()) {
            ImGui::Text(
                "Varp: %u",
                static_cast<unsigned int>(
                    *varbit.varpId
                )
            );
        }

        ImGui::Text(
            "Bit range: %u to %u",
            static_cast<unsigned int>(
                varbit.leastSignificantBit
            ),
            static_cast<unsigned int>(
                varbit.mostSignificantBit
            )
        );

        const unsigned int bitCount =
            varbit.mostSignificantBit >=
                varbit.leastSignificantBit
                ? varbit.mostSignificantBit -
                    varbit.leastSignificantBit
                : 0;

        ImGui::Text("Bit count: %u", bitCount);

        ImGui::Text(
            "Maximum value: %u",
            bitCount < 32
                ? ((1u << bitCount) - 1u)
                : 0xFFFFFFFFu
        );

        ImGui::Text(
            "Tracked: %s",
            varbit.tracked ? "yes" : "no"
        );
    }

    if (state.activeVarp.has_value()) {
        const auto& varp =
            *state.activeVarp;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("VARP");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(varp.id)
        );

        ImGui::Text(
            "Name: %s",
            varp.name.empty()
                ? "(none)"
                : varp.name.c_str()
        );

        if (varp.clientCode.has_value()) {
            ImGui::Text(
                "Client code: %u",
                static_cast<unsigned int>(
                    *varp.clientCode
                )
            );
        }

        ImGui::Text(
            "Tracked: %s",
            varp.tracked ? "yes" : "no"
        );

        ImGui::Text(
            "Persistent: %s",
            varp.persistent ? "yes" : "no"
        );

        ImGui::Text(
            "Active: %s",
            varp.active ? "yes" : "no"
        );

        ImGui::Text(
            "Mode: %d",
            static_cast<int>(varp.mode)
        );
    }

    if (state.activeSpotAnimation.has_value()) {
        const eld::definition::SpotAnimationDefinition& effect =
            *state.activeSpotAnimation;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("SPOT ANIMATION");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(effect.id)
        );

        if (effect.modelId.has_value()) {
            ImGui::Text(
                "Model: %u",
                static_cast<unsigned int>(
                    *effect.modelId
                )
            );
        }
        else {
            ImGui::TextUnformatted("Model: (none)");
        }

        if (effect.sequenceId.has_value()) {
            ImGui::Text(
                "Sequence: %u",
                static_cast<unsigned int>(
                    *effect.sequenceId
                )
            );
        }
        else {
            ImGui::TextUnformatted("Sequence: (none)");
        }

        ImGui::Text(
            "Scale: %u, %u",
            static_cast<unsigned int>(effect.scaleX),
            static_cast<unsigned int>(effect.scaleY)
        );

        ImGui::Text(
            "Rotation: %u",
            static_cast<unsigned int>(effect.rotation)
        );

        ImGui::Text(
            "Ambient: %u",
            static_cast<unsigned int>(effect.ambient)
        );

        ImGui::Text(
            "Contrast: %u",
            static_cast<unsigned int>(effect.contrast)
        );

        ImGui::TextUnformatted("Recolors:");

        bool hasRecolors = false;

        for (
            std::size_t index = 0;
            index < effect.recolorSources.size();
            ++index
        ) {
            if (
                effect.recolorSources[index].has_value() &&
                effect.recolorDestinations[index].has_value()
            ) {
                hasRecolors = true;

                ImGui::BulletText(
                    "%u -> %u",
                    static_cast<unsigned int>(
                        *effect.recolorSources[index]
                    ),
                    static_cast<unsigned int>(
                        *effect.recolorDestinations[index]
                    )
                );
            }
        }

        if (!hasRecolors) {
            ImGui::TextUnformatted("(none)");
        }
    }

    if (state.activeSequence.has_value()) {
        const eld::definition::SequenceDefinition& sequence =
            *state.activeSequence;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("SEQUENCE");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(sequence.id)
        );

        ImGui::Text(
            "Frames: %zu",
            sequence.frames.size()
        );

        if (sequence.frameStep.has_value()) {
            ImGui::Text(
                "Frame step: %u",
                static_cast<unsigned int>(
                    *sequence.frameStep
                )
            );
        }
        else {
            ImGui::TextUnformatted(
                "Frame step: (none)"
            );
        }

        ImGui::Text(
            "Priority: %u",
            static_cast<unsigned int>(
                sequence.priority
            )
        );

        ImGui::Text(
            "Maximum loops: %u",
            static_cast<unsigned int>(
                sequence.maximumLoops
            )
        );

        ImGui::Text(
            "Stretches: %s",
            sequence.stretches ? "yes" : "no"
        );

        for (
            std::size_t index = 0;
            index < sequence.frames.size();
            ++index
        ) {
            const eld::definition::SequenceFrame& frame =
                sequence.frames[index];

            if (frame.secondaryFrameId.has_value()) {
                ImGui::BulletText(
                    "%zu: primary %u, secondary %u, duration %u",
                    index,
                    static_cast<unsigned int>(
                        frame.primaryFrameId
                    ),
                    static_cast<unsigned int>(
                        *frame.secondaryFrameId
                    ),
                    static_cast<unsigned int>(
                        frame.duration
                    )
                );
            }
            else {
                ImGui::BulletText(
                    "%zu: frame %u, duration %u",
                    index,
                    static_cast<unsigned int>(
                        frame.primaryFrameId
                    ),
                    static_cast<unsigned int>(
                        frame.duration
                    )
                );
            }
        }
    }

    if (state.activeItem.has_value()) {
        const eld::definition::ItemDefinition& item =
            *state.activeItem;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("ITEM");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(item.id)
        );

        ImGui::Text(
            "Name: %s",
            item.name.c_str()
        );

        if (item.inventoryModelId.has_value()) {
            ImGui::Text(
                "Inventory model: %u",
                static_cast<unsigned int>(
                    *item.inventoryModelId
                )
            );
        }
        else {
            ImGui::TextUnformatted(
                "Inventory model: (none)"
            );
        }

        ImGui::Text(
            "Value: %u",
            static_cast<unsigned int>(item.value)
        );

        ImGui::Text(
            "Stackable: %s",
            item.stackable ? "yes" : "no"
        );

        ImGui::Text(
            "Members only: %s",
            item.membersOnly ? "yes" : "no"
        );

        ImGui::Text(
            "Zoom: %u",
            static_cast<unsigned int>(item.zoom)
        );

        ImGui::Text(
            "Rotation: %u, %u, %u",
            static_cast<unsigned int>(item.rotationX),
            static_cast<unsigned int>(item.rotationY),
            static_cast<unsigned int>(item.rotationZ)
        );

        ImGui::Text(
            "Offset: %d, %d",
            static_cast<int>(item.offsetX),
            static_cast<int>(item.offsetY)
        );

        ImGui::TextUnformatted("Inventory actions:");

        bool hasInventoryActions = false;

        for (const std::string& action : item.inventoryActions) {
            if (!action.empty()) {
                hasInventoryActions = true;
                ImGui::BulletText("%s", action.c_str());
            }
        }

        if (!hasInventoryActions) {
            ImGui::TextUnformatted("(none)");
        }

        ImGui::TextUnformatted("Ground actions:");

        bool hasGroundActions = false;

        for (const std::string& action : item.groundActions) {
            if (!action.empty()) {
                hasGroundActions = true;
                ImGui::BulletText("%s", action.c_str());
            }
        }

        if (!hasGroundActions) {
            ImGui::TextUnformatted("(none)");
        }
    }

    if (state.activeNpc.has_value()) {
        const eld::definition::NpcDefinition& npc =
            *state.activeNpc;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("NPC");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(npc.id)
        );

        ImGui::Text(
            "Name: %s",
            npc.name.c_str()
        );

        ImGui::Text(
            "Combat level: %s",
            npc.combatLevel.has_value()
                ? std::to_string(*npc.combatLevel).c_str()
                : "(none)"
        );

        ImGui::Text(
            "Size: %d",
            static_cast<int>(npc.size)
        );

        ImGui::Text(
            "Models: %zu",
            npc.modelIds.size()
        );

        for (const std::uint16_t modelId : npc.modelIds) {
            ImGui::BulletText(
                "Model %u",
                static_cast<unsigned int>(modelId)
            );
        }

        ImGui::Text(
            "Idle animation: %s",
            npc.idleAnimationId.has_value()
                ? std::to_string(*npc.idleAnimationId).c_str()
                : "(none)"
        );

        ImGui::Text(
            "Walk animation: %s",
            npc.walkAnimationId.has_value()
                ? std::to_string(*npc.walkAnimationId).c_str()
                : "(none)"
        );

        ImGui::Text(
            "Scale: %u, %u",
            static_cast<unsigned int>(npc.scaleX),
            static_cast<unsigned int>(npc.scaleY)
        );

        ImGui::Text(
            "Minimap: %s",
            npc.visibleOnMinimap ? "visible" : "hidden"
        );

        ImGui::Text(
            "Clickable: %s",
            npc.clickable ? "yes" : "no"
        );

        ImGui::TextUnformatted("Actions:");

        bool hasActions = false;

        for (const std::string& action : npc.actions) {
            if (!action.empty()) {
                hasActions = true;
                ImGui::BulletText("%s", action.c_str());
            }
        }

        if (!hasActions) {
            ImGui::TextUnformatted("(none)");
        }
    }

    if (state.activeLocation.has_value()) {
        const eld::definition::LocationDefinition& location =
            *state.activeLocation;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("LOCATION");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(location.id)
        );

        ImGui::Text(
            "Name: %s",
            location.name.c_str()
        );

        ImGui::Text(
            "Size: %u x %u",
            static_cast<unsigned int>(location.width),
            static_cast<unsigned int>(location.length)
        );

        ImGui::Text(
            "Models: %zu",
            location.models.size()
        );

        for (
            const eld::definition::LocationModel& model :
            location.models
        ) {
            if (model.type.has_value()) {
                ImGui::BulletText(
                    "Model %u, type %u",
                    static_cast<unsigned int>(model.id),
                    static_cast<unsigned int>(*model.type)
                );
            }
            else {
                ImGui::BulletText(
                    "Model %u",
                    static_cast<unsigned int>(model.id)
                );
            }
        }

        ImGui::Text(
            "Solid: %s",
            location.solid ? "yes" : "no"
        );

        ImGui::Text(
            "Impenetrable: %s",
            location.impenetrable ? "yes" : "no"
        );

        if (location.animationId.has_value()) {
            ImGui::Text(
                "Animation: %u",
                static_cast<unsigned int>(
                    *location.animationId
                )
            );
        }
        else {
            ImGui::TextUnformatted(
                "Animation: (none)"
            );
        }

        ImGui::Text(
            "Scale: %u, %u, %u",
            static_cast<unsigned int>(location.scaleX),
            static_cast<unsigned int>(location.scaleY),
            static_cast<unsigned int>(location.scaleZ)
        );

        ImGui::Text(
            "Offset: %d, %d, %d",
            static_cast<int>(location.offsetX),
            static_cast<int>(location.offsetY),
            static_cast<int>(location.offsetZ)
        );

        ImGui::TextUnformatted("Actions:");

        bool hasActions = false;

        for (const std::string& action : location.actions) {
            if (!action.empty()) {
                hasActions = true;
                ImGui::BulletText("%s", action.c_str());
            }
        }

        if (!hasActions) {
            ImGui::TextUnformatted("(none)");
        }
    }

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
