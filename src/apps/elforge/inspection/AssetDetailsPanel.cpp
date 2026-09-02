#include "dump/AnimationDumper.h"
#include "inspection/AssetDetailsPanel.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>

#include <imgui.h>

#include "explorer/CacheExplorerState.h"
#include "export/AnimationExporter.h"
#include "export/MidiExporter.h"

namespace eld::elforge {

namespace {

std::optional<std::pair<std::string, std::uint16_t>>
parseSpriteReference(
    const std::string& reference
) {
    const std::size_t comma =
        reference.rfind(',');

    if (comma == std::string::npos) {
        return std::nullopt;
    }

    std::string name =
        reference.substr(0, comma);

    if (
        name.size() < 4 ||
        name.substr(name.size() - 4) != ".dat"
    ) {
        name += ".dat";
    }

    std::uint16_t frameId = 0;

    const std::string frameText =
        reference.substr(comma + 1);

    const char* begin =
        frameText.data();

    const char* end =
        frameText.data() +
        frameText.size();

    const std::from_chars_result result =
        std::from_chars(
            begin,
            end,
            frameId
        );

    if (result.ec != std::errc{} || result.ptr != end) {
        return std::nullopt;
    }

    return std::pair<std::string, std::uint16_t>{
        name,
        frameId
    };
}

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

        case CacheTreeNodeType::Animation:
            return "Animation";

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

        case CacheTreeNodeType::InterfaceWidget:
            return "Interface";

        case CacheTreeNodeType::Midi:
            return "MIDI";

        case CacheTreeNodeType::MapRegion:
            return "Map Region";

        case CacheTreeNodeType::Font:
            return "Font";

        case CacheTreeNodeType::Image:
            return "Image";

        case CacheTreeNodeType::SpriteFrame:
            return "Sprite Frame";
    }

    return "Unknown";
}

const char* mapLocKindName(
    eld::render::map::SceneLocationKind kind
) {
    switch (kind) {
        case eld::render::map::SceneLocationKind::Wall:
            return "Wall";

        case eld::render::map::SceneLocationKind::WallDecoration:
            return "Wall Decoration";

        case eld::render::map::SceneLocationKind::GroundDecoration:
            return "Ground Decoration";

        case eld::render::map::SceneLocationKind::Location:
            return "Location";

        case eld::render::map::SceneLocationKind::Roof:
            return "Roof";
    }

    return "Unknown";
}

const char* animationTransformTypeName(
    std::uint8_t type
) {
    switch (type) {
        case 0: return "Pivot";
        case 1: return "Translate";
        case 2: return "Rotate";
        case 3: return "Scale";
        case 4: return "Unknown 4";
        case 5: return "Alpha";
        default: return "Unknown";
    }
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

void AssetDetailsPanel::render(
    CacheExplorerState& state,
    float width,
    float height
) {
    ImGui::BeginChild(
        "AssetDetailsPanel",
        ImVec2(width, height),
        true
    );

    ImGui::TextUnformatted("DETAILS");
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

    if (!state.assetDumpStatus.empty()) {
        ImGui::Spacing();

        ImGui::TextWrapped(
            "%s",
            state.assetDumpStatus.c_str()
        );
    }

    if (state.selection.regionId >= 0) {
        ImGui::Text(
            "Region: %d",
            state.selection.regionId
        );

        ImGui::Text(
            "Terrain file: %d",
            state.selection.terrainFileId
        );

        ImGui::Text(
            "Object file: %d",
            state.selection.locationFileId
        );
    }

    if (state.activeAnimation.has_value()) {
        const AnimationInspection& info =
            *state.activeAnimation;

        const eld::animation::Animation& animation =
            info.animation;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("ANIMATION");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(animation.id)
        );

        ImGui::Text(
            "Frames: %zu",
            animation.asset.frames.size()
        );

        ImGui::Text(
            "Skeleton slots: %zu",
            animation.asset.skeleton.slots.size()
        );

        ImGui::Text(
            "Raw file bytes: %zu",
            animation.file.bytes.size()
        );

        if (!animation.asset.frames.empty()) {
            std::uint16_t minimumFrame =
                animation.asset.frames.front().id;

            std::uint16_t maximumFrame =
                animation.asset.frames.front().id;

            for (
                const eld::animation::AnimationFrame& frame :
                animation.asset.frames
            ) {
                minimumFrame =
                    std::min(
                        minimumFrame,
                        frame.id
                    );

                maximumFrame =
                    std::max(
                        maximumFrame,
                        frame.id
                    );
            }

            ImGui::Text(
                "Global frame IDs: %u - %u",
                static_cast<unsigned int>(
                    minimumFrame
                ),
                static_cast<unsigned int>(
                    maximumFrame
                )
            );
        }

        ImGui::Spacing();

        const std::filesystem::path dumpPath =
            defaultAnimationDumpPath(
                animation.id
            );

        if (ImGui::Button("Dump Full Asset")) {
            std::string error;

            if (
                dumpAnimation(
                    info,
                    dumpPath,
                    error
                )
            ) {
                state.animationDumpStatus =
                    "Dumped: " +
                    dumpPath.string();
            }
            else {
                state.animationDumpStatus =
                    "Dump failed: " +
                    error;
            }
        }

        if (!state.animationDumpStatus.empty()) {
            ImGui::TextWrapped(
                "%s",
                state.animationDumpStatus.c_str()
            );
        }
    }

    if (state.activeMidi.has_value()) {
        const eld::midi::MidiFile& midi =
            *state.activeMidi;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("MIDI");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(midi.id)
        );

        ImGui::Text(
            "Source container: %s",
            midi.data.sourceContainer ==
                    eld::midi::MidiContainer::RiffMidi
                ? "RIFF/RMID"
                : "Standard MIDI"
        );

        ImGui::Text(
            "Format: %u",
            static_cast<unsigned int>(
                midi.data.header.format
            )
        );

        ImGui::Text(
            "Tracks: %u",
            static_cast<unsigned int>(
                midi.data.header.trackCount
            )
        );

        const std::uint16_t division =
            midi.data.header.division;

        if ((division & 0x8000u) == 0) {
            ImGui::Text(
                "Division: %u ticks / quarter note",
                static_cast<unsigned int>(division)
            );
        }
        else {
            ImGui::Text(
                "Division: SMPTE 0x%04X",
                static_cast<unsigned int>(division)
            );
        }

        ImGui::Text(
            "Normalized size: %zu bytes",
            midi.data.bytes.size()
        );

        ImGui::TextUnformatted(
            "Normalized stream: Standard MIDI (MThd at byte 0)"
        );

        const std::filesystem::path exportPath =
            defaultMidiExportPath(midi);

        if (ImGui::Button("Export normalized .mid")) {
            std::string error;

            if (exportMidi(midi, exportPath, error)) {
                state.midiExportStatus =
                    "Exported: " +
                    exportPath.string();
            }
            else {
                state.midiExportStatus =
                    "Export failed: " +
                    error;
            }
        }

        ImGui::TextWrapped(
            "Export path: %s",
            exportPath.string().c_str()
        );

        if (!state.midiExportStatus.empty()) {
            ImGui::TextWrapped(
                "%s",
                state.midiExportStatus.c_str()
            );
        }

        if (ImGui::CollapsingHeader("Track chunks")) {
            for (
                std::size_t index = 0;
                index < midi.data.tracks.size();
                ++index
            ) {
                const eld::midi::MidiTrackInfo& track =
                    midi.data.tracks[index];

                ImGui::Text(
                    "Track %zu: chunk @ %zu, data @ %zu, %u bytes",
                    index,
                    track.chunkOffset,
                    track.dataOffset,
                    static_cast<unsigned int>(track.dataSize)
                );
            }
        }
    }

    if (state.activeMap.has_value()) {
        const MapViewState& map =
            *state.activeMap;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("MAP REGION");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(
                map.indexEntry.regionId
            )
        );

        ImGui::Text(
            "Region coordinates: %d, %d",
            map.indexEntry.regionX(),
            map.indexEntry.regionY()
        );

        ImGui::Text(
            "World base: %d, %d",
            map.centerRegion.worldBaseX(),
            map.centerRegion.worldBaseY()
        );

        ImGui::Text(
            "Preload: %s",
            map.indexEntry.shouldPreload
                ? "yes"
                : "no"
        );

        ImGui::Text(
            "Decoded object spawns: %zu",
            map.centerRegion.locations.size()
        );

        ImGui::Text(
            "Neighborhood terrain: %zu / 9",
            map.stats.neighborhoodRegions
        );

        ImGui::Text(
            "Build time: %.2f ms",
            map.stats.buildMilliseconds
        );

        ImGui::Text(
            "Terrain triangles: %zu (%zu buckets)",
            map.stats.terrainTriangles,
            map.stats.terrainDrawBuckets
        );

        ImGui::Text(
            "Object triangles: %zu (%zu static buckets)",
            map.stats.locTriangles,
            map.stats.locDrawBuckets
        );

        ImGui::Text(
            "Loc models: %zu instances, %zu parts, %zu variants",
            map.stats.locModelInstances,
            map.stats.locModelParts,
            map.stats.locModelVariants
        );

        ImGui::Text(
            "Camera-dependent parts: %zu",
            map.stats.cameraDependentParts
        );

        if (!map.missingNeighborRegionIds.empty()) {
            ImGui::TextUnformatted(
                "Missing neighboring regions:"
            );

            for (
                const std::uint16_t regionId :
                map.missingNeighborRegionIds
            ) {
                ImGui::BulletText(
                    "%u",
                    static_cast<unsigned int>(regionId)
                );
            }
        }

        if (state.selectedMapTile.has_value()) {
            const MapTileSelection& selection =
                *state.selectedMapTile;

            if (
                selection.plane < eld::map::PlaneCount &&
                selection.x >= 0 &&
                selection.x < static_cast<int>(eld::map::RegionSize) &&
                selection.y >= 0 &&
                selection.y < static_cast<int>(eld::map::RegionSize)
            ) {
                const eld::map::MapTile& tile =
                    map.centerRegion.tile(
                        selection.plane,
                        static_cast<std::size_t>(selection.x),
                        static_cast<std::size_t>(selection.y)
                    );

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextUnformatted("SELECTED MAP TILE");

                ImGui::Text(
                    "Plane: %zu",
                    selection.plane
                );

                ImGui::Text(
                    "Local tile: %d, %d",
                    selection.x,
                    selection.y
                );

                ImGui::Text(
                    "World tile: %d, %d",
                    map.centerRegion.worldBaseX() + selection.x,
                    map.centerRegion.worldBaseY() + selection.y
                );

                ImGui::Text(
                    "Height: %d",
                    tile.height
                );

                if (tile.underlayId == 0) {
                    ImGui::TextUnformatted("Underlay raw ID: 0");
                }
                else {
                    ImGui::Text(
                        "Underlay raw ID: %u (floor definition %d)",
                        static_cast<unsigned int>(tile.underlayId),
                        static_cast<int>(tile.underlayId) - 1
                    );
                }

                if (tile.overlayId == 0) {
                    ImGui::TextUnformatted("Overlay raw ID: 0");
                }
                else {
                    ImGui::Text(
                        "Overlay raw ID: %u (floor definition %d)",
                        static_cast<unsigned int>(tile.overlayId),
                        static_cast<int>(tile.overlayId) - 1
                    );
                }

                ImGui::Text(
                    "Overlay shape / rotation: %u / %u",
                    static_cast<unsigned int>(tile.overlayShape),
                    static_cast<unsigned int>(tile.overlayRotation)
                );

                ImGui::Text(
                    "Settings: 0x%02X",
                    static_cast<unsigned int>(tile.settings)
                );

                if (tile.settings == 0) {
                    ImGui::TextUnformatted("Flags: none");
                }
                else {
                    ImGui::TextUnformatted("Flags:");

                    if (tile.hasFlag(eld::map::TileFlag::Solid)) {
                        ImGui::BulletText("0x01 solid / floor clipped");
                    }

                    if (tile.hasFlag(eld::map::TileFlag::Bridge)) {
                        ImGui::BulletText("0x02 bridge");
                    }

                    if (tile.hasFlag(eld::map::TileFlag::Roof)) {
                        ImGui::BulletText("0x04 roof marker");
                    }

                    if (tile.hasFlag(eld::map::TileFlag::ForceLevelZero)) {
                        ImGui::BulletText("0x08 force draw level 0");
                    }

                    if (tile.hasFlag(eld::map::TileFlag::LowMemoryHidden)) {
                        ImGui::BulletText("0x10 low-memory hidden");
                    }

                    if (tile.hasFlag(eld::map::TileFlag::Unknown20)) {
                        ImGui::BulletText("0x20 unknown / preserved");
                    }
                }
            }
        }

        if (
            state.selectedMapLocIndex.has_value() &&
            *state.selectedMapLocIndex < map.sceneLocs.size()
        ) {
            const eld::render::map::SceneLocationPlacement& loc =
                map.sceneLocs[*state.selectedMapLocIndex];

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextUnformatted("SELECTED MAP OBJECT");

            ImGui::Text(
                "Loc ID: %u",
                static_cast<unsigned int>(loc.id)
            );

            ImGui::Text(
                "Kind: %s",
                mapLocKindName(loc.kind)
            );

            ImGui::Text(
                "Shape / model type / rotation: %u / %u / %u",
                static_cast<unsigned int>(loc.shape),
                static_cast<unsigned int>(loc.modelType),
                static_cast<unsigned int>(loc.rotation)
            );

            ImGui::Text(
                "Source plane -> scene plane: %u -> %u",
                static_cast<unsigned int>(loc.sourcePlane),
                static_cast<unsigned int>(loc.scenePlane)
            );

            ImGui::Text(
                "Bridge attachment: %s",
                loc.bridgeAttachment ? "yes" : "no"
            );

            ImGui::Text(
                "Local tile: %d, %d",
                loc.tileX,
                loc.tileZ
            );

            ImGui::Text(
                "World tile: %d, %d",
                map.centerRegion.worldBaseX() + loc.tileX,
                map.centerRegion.worldBaseY() + loc.tileZ
            );

            ImGui::Text(
                "Footprint: %d x %d",
                loc.footprintWidth,
                loc.footprintLength
            );

            ImGui::Text(
                "Scene XYZ: %d, %d, %d",
                loc.sceneX,
                loc.sceneY,
                loc.sceneZ
            );

            ImGui::Text(
                "Primary model rotation: %u",
                static_cast<unsigned int>(
                    loc.primaryModelRotation
                )
            );

            if (loc.hasSecondaryModel) {
                ImGui::Text(
                    "Secondary model rotation: %u",
                    static_cast<unsigned int>(
                        loc.secondaryModelRotation
                    )
                );
            }

            ImGui::Text(
                "Extra scene yaw: %d",
                loc.sceneYaw
            );

            ImGui::Text(
                "Corner heights: %d, %d, %d, %d",
                loc.cornerHeights[0],
                loc.cornerHeights[1],
                loc.cornerHeights[2],
                loc.cornerHeights[3]
            );

            if (loc.kind == eld::render::map::SceneLocationKind::Wall) {
                ImGui::Text(
                    "Wall types: %d, %d",
                    loc.wallTypeA,
                    loc.wallTypeB
                );
            }

            if (
                loc.kind ==
                    eld::render::map::SceneLocationKind::WallDecoration
            ) {
                ImGui::Text(
                    "Decoration type / angle: %d / %d",
                    loc.decorationType,
                    loc.decorationAngle
                );

                ImGui::Text(
                    "Decoration offset X/Z: %d, %d",
                    loc.decorationOffsetX,
                    loc.decorationOffsetZ
                );
            }
        }
    }

    if (state.activeInterface.has_value()) {
        const auto& widget =
            *state.activeInterface;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("INTERFACE");

        ImGui::Text(
            "ID: %u",
            static_cast<unsigned int>(widget.id)
        );

        if (!state.activeInterfaceDump.empty()) {
            if (ImGui::Button("Copy interface dump")) {
                ImGui::SetClipboardText(
                    state.activeInterfaceDump.c_str()
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Print interface dump")) {
                ImGui::LogToTTY();
                ImGui::LogText(
                    "%s",
                    state.activeInterfaceDump.c_str()
                );
                ImGui::LogFinish();
            }

            if (ImGui::CollapsingHeader("Interface Dump")) {
                ImGui::TextUnformatted(
                    state.activeInterfaceDump.c_str()
                );
            }
        }


        if (widget.parentId.has_value()) {
            ImGui::Text(
                "Parent: %u",
                static_cast<unsigned int>(
                    *widget.parentId
                )
            );
        }
        else {
            ImGui::TextUnformatted("Parent: (root)");
        }

        ImGui::Text(
            "Type: %u",
            static_cast<unsigned int>(widget.type)
        );

        ImGui::Text(
            "Action type: %u",
            static_cast<unsigned int>(
                widget.actionType
            )
        );

        ImGui::Text(
            "Content type: %u",
            static_cast<unsigned int>(
                widget.contentType
            )
        );

        ImGui::Text(
            "Size: %u x %u",
            static_cast<unsigned int>(widget.width),
            static_cast<unsigned int>(widget.height)
        );

        ImGui::Text(
            "Children: %zu",
            widget.children.size()
        );

        ImGui::Text(
            "Conditions: %zu",
            widget.conditions.size()
        );

        ImGui::Text(
            "Scripts: %zu",
            widget.scripts.size()
        );

        if (!widget.text.empty()) {
            ImGui::TextWrapped(
                "Text: %s",
                widget.text.c_str()
            );
        }

        if (!widget.secondaryText.empty()) {
            ImGui::TextWrapped(
                "Secondary text: %s",
                widget.secondaryText.c_str()
            );
        }

        if (!widget.sprite.empty()) {
            ImGui::Text(
                "Sprite: %s",
                widget.sprite.c_str()
            );

            const auto parsedSprite =
                parseSpriteReference(widget.sprite);

            if (parsedSprite.has_value()) {
                ImGui::Text(
                    "Sprite Parsed: %s frame %u",
                    parsedSprite->first.c_str(),
                    static_cast<unsigned int>(
                        parsedSprite->second
                    )
                );
            }
            else {
                ImGui::TextUnformatted(
                    "Sprite Parsed: failed"
                );
            }
        }

        if (!widget.secondarySprite.empty()) {
            ImGui::Text(
                "Secondary Sprite: %s",
                widget.secondarySprite.c_str()
            );

            const auto parsedSprite =
                parseSpriteReference(widget.secondarySprite);

            if (parsedSprite.has_value()) {
                ImGui::Text(
                    "Secondary Sprite Parsed: %s frame %u",
                    parsedSprite->first.c_str(),
                    static_cast<unsigned int>(
                        parsedSprite->second
                    )
                );
            }
            else {
                ImGui::TextUnformatted(
                    "Secondary Sprite Parsed: failed"
                );
            }
        }

        if (widget.modelId.has_value()) {
            ImGui::Text(
                "Model: %u",
                static_cast<unsigned int>(
                    *widget.modelId
                )
            );
        }

        if (widget.animationId.has_value()) {
            ImGui::Text(
                "Animation: %u",
                static_cast<unsigned int>(
                    *widget.animationId
                )
            );
        }

        if (!widget.tooltip.empty()) {
            ImGui::Text(
                "Tooltip: %s",
                widget.tooltip.c_str()
            );
        }
    }

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

    if (state.activeTexture.has_value()) {
        const auto& texture =
            *state.activeTexture;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("TEXTURE");

        ImGui::Text(
            "Size: %u x %u",
            static_cast<unsigned int>(
                texture.image.width
            ),
            static_cast<unsigned int>(
                texture.image.height
            )
        );

        ImGui::Text(
            "Decoded pixels: %zu",
            texture.image.pixels.size()
        );
    }

    if (state.activeSprite.has_value()) {
        const auto& sprite =
            *state.activeSprite;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("SPRITE");

        ImGui::Text(
            "Size: %u x %u",
            static_cast<unsigned int>(
                sprite.image.width
            ),
            static_cast<unsigned int>(
                sprite.image.height
            )
        );

        ImGui::Text(
            "Frame: %d",
            std::max(
                state.selection.frameId,
                0
            )
        );

        ImGui::Text(
            "Decoded pixels: %zu",
            sprite.image.pixels.size()
        );
    }

    if (state.activeImage.has_value()) {
        const auto& image =
            *state.activeImage;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("IMAGE");

        ImGui::Text(
            "Size: %u x %u",
            static_cast<unsigned int>(
                image.width
            ),
            static_cast<unsigned int>(
                image.height
            )
        );

        ImGui::Text(
            "Decoded pixels: %zu",
            image.pixels.size()
        );
    }

    if (state.activeFont.has_value()) {
        const auto& font =
            *state.activeFont;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("FONT");

        ImGui::Text(
            "Glyphs: %zu",
            font.glyphs.size()
        );

        ImGui::Text(
            "Line height: %u",
            static_cast<unsigned int>(
                font.lineHeight
            )
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
