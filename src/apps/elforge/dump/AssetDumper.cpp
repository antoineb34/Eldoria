#include "dump/AssetDumper.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include "dump/AnimationDumper.h"
#include "explorer/CacheExplorerState.h"

namespace eld::elforge {

namespace {

const char* nodeTypeName(
    CacheTreeNodeType type
) {
    switch (type) {
        case CacheTreeNodeType::Root: return "Root";
        case CacheTreeNodeType::Index: return "Index";
        case CacheTreeNodeType::File: return "File";
        case CacheTreeNodeType::Model: return "Model";
        case CacheTreeNodeType::Animation: return "Animation";
        case CacheTreeNodeType::Texture: return "Texture";
        case CacheTreeNodeType::Archive: return "Archive";
        case CacheTreeNodeType::ArchiveFile: return "Archive File";
        case CacheTreeNodeType::Sprite: return "Sprite";
        case CacheTreeNodeType::DefinitionGroup: return "Definition Group";
        case CacheTreeNodeType::Floor: return "Floor Definition";
        case CacheTreeNodeType::IdentityKit: return "Identity Kit";
        case CacheTreeNodeType::Location: return "Location";
        case CacheTreeNodeType::Npc: return "NPC";
        case CacheTreeNodeType::Item: return "Item";
        case CacheTreeNodeType::Sequence: return "Sequence";
        case CacheTreeNodeType::SpotAnimation: return "Spot Animation";
        case CacheTreeNodeType::Varp: return "Varp";
        case CacheTreeNodeType::Varbit: return "Varbit";
        case CacheTreeNodeType::Parameter: return "Parameter";
        case CacheTreeNodeType::Message: return "Message";
        case CacheTreeNodeType::MessageAnimation: return "Message Animation";
        case CacheTreeNodeType::Widget: return "Interface";
        case CacheTreeNodeType::Midi: return "MIDI";
        case CacheTreeNodeType::MapRegion: return "Map Region";
        case CacheTreeNodeType::Font: return "Font";
        case CacheTreeNodeType::Image: return "Image";
        case CacheTreeNodeType::SpriteFrame: return "Sprite Frame";
    }

    return "Unknown";
}


std::string slug(
    std::string value
) {
    for (char& ch : value) {
        const unsigned char byte =
            static_cast<unsigned char>(ch);

        if (std::isalnum(byte)) {
            ch = static_cast<char>(
                std::tolower(byte)
            );
        }
        else {
            ch = '_';
        }
    }

    while (
        value.find("__") !=
        std::string::npos
    ) {
        value.replace(
            value.find("__"),
            2,
            "_"
        );
    }

    if (value.empty()) {
        value = "asset";
    }

    return value;
}


template <typename TBytes>
void writeHexBytes(
    std::ostream& out,
    const TBytes& bytes
) {
    constexpr std::size_t Width = 16;

    for (
        std::size_t offset = 0;
        offset < bytes.size();
        offset += Width
    ) {
        out << std::hex
            << std::setw(8)
            << std::setfill('0')
            << offset
            << "  ";

        for (
            std::size_t index = 0;
            index < Width;
            ++index
        ) {
            if (offset + index < bytes.size()) {
                out << std::setw(2)
                    << static_cast<unsigned int>(
                        bytes[offset + index]
                    )
                    << ' ';
            }
            else {
                out << "   ";
            }
        }

        out << " |";

        for (
            std::size_t index = 0;
            index < Width &&
                offset + index < bytes.size();
            ++index
        ) {
            const unsigned char value =
                static_cast<unsigned char>(
                    bytes[offset + index]
                );

            out << (
                std::isprint(value)
                    ? static_cast<char>(value)
                    : '.'
            );
        }

        out << "|\n";
    }

    out << std::dec
        << std::setfill(' ');
}


void writePixels(
    std::ostream& out,
    const eld::image::Image& image
) {
    out << "Width: " << image.width << "\n";
    out << "Height: " << image.height << "\n";
    out << "Pixels: " << image.pixels.size() << "\n\n";
    out << "[Decoded Pixels]\n";

    for (
        std::size_t index = 0;
        index < image.pixels.size();
        ++index
    ) {
        const auto& pixel =
            image.pixels[index];

        out << index
            << ": "
            << static_cast<unsigned int>(pixel.red)
            << ' '
            << static_cast<unsigned int>(pixel.green)
            << ' '
            << static_cast<unsigned int>(pixel.blue)
            << ' '
            << static_cast<unsigned int>(pixel.alpha)
            << '\n';
    }
}


void writeSelection(
    std::ostream& out,
    const CacheExplorerState& state
) {
    out << "ELDORIA ASSET DUMP\n";
    out << "Type: "
        << nodeTypeName(state.selection.type)
        << "\n";
    out << "Label: "
        << state.selection.label
        << "\n";
    out << "Key: "
        << state.selection.key
        << "\n";
    out << "Index: "
        << state.selection.indexId
        << "\n";
    out << "Archive: "
        << state.selection.archiveId
        << "\n";
    out << "File: "
        << state.selection.fileId
        << "\n";
    out << "Definition: "
        << state.selection.definitionId
        << "\n";
    out << "Region: "
        << state.selection.regionId
        << "\n";
    out << "Frame: "
        << state.selection.frameId
        << "\n\n";
}


void writeModel(
    std::ostream& out,
    const eld::model::Model& model
) {
    out << "[Model]\n";
    out << "ID: " << model.id << "\n";
    out << "Vertices: "
        << model.vertices.size()
        << "\n";
    out << "Faces: "
        << model.faces.size()
        << "\n";
    out << "Texture mappings: "
        << model.textureMappings.size()
        << "\n";
        
    out << "Vertices\n";
    for (
        std::size_t index = 0;
        index < model.vertices.size();
        ++index
    ) {
        const auto& vertex =
            model.vertices[index];

        out << index
            << ": "
            << vertex.x << ' '
            << vertex.y << ' '
            << vertex.z;

        if (vertex.skin.has_value()) {
            out << " skin="
                << static_cast<unsigned int>(
                    *vertex.skin
                );
        }

        out << '\n';
    }

    out << "\nFaces\n";
    for (
        std::size_t index = 0;
        index < model.faces.size();
        ++index
    ) {
        const auto& face =
            model.faces[index];

        out << index
            << ": "
            << face.a << ' '
            << face.b << ' '
            << face.c
            << " color=" << face.color
            << " priority="
            << static_cast<unsigned int>(face.priority)
            << " alpha="
            << static_cast<unsigned int>(face.alpha)
            << " renderType="
            << static_cast<unsigned int>(face.renderType);

        if (face.skin.has_value()) {
            out << " skin="
                << static_cast<unsigned int>(
                    *face.skin
                );
        }

        if (face.textureId.has_value()) {
            out << " texture="
                << *face.textureId;
        }

        if (face.textureMappingIndex.has_value()) {
            out << " mapping="
                << *face.textureMappingIndex;
        }

        out << '\n';
    }

    out << "\nTexture Mappings\n";
    for (
        std::size_t index = 0;
        index < model.textureMappings.size();
        ++index
    ) {
        const auto& mapping =
            model.textureMappings[index];

        out << index
            << ": origin="
            << mapping.originVertex
            << " u="
            << mapping.uVertex
            << " v="
            << mapping.vVertex
            << '\n';
    }

    out << "\n[Raw Source]\n";
        out << "[Raw Bytes]\n";
}

}


std::filesystem::path defaultAssetDumpPath(
    const CacheExplorerState& state
) {
    std::string name =
        slug(
            nodeTypeName(
                state.selection.type
            )
        );

    int id =
        state.selection.definitionId >= 0
            ? state.selection.definitionId
            : state.selection.regionId >= 0
                ? state.selection.regionId
                : state.selection.fileId >= 0
                    ? state.selection.fileId
                    : state.selection.archiveId;

    if (!state.selection.name.empty()) {
        name += "_" +
            slug(
                state.selection.name
            );
    }

    if (id >= 0) {
        name += "_" +
            std::to_string(id);
    }

    if (state.selection.frameId >= 0) {
        name += "_frame_" +
            std::to_string(
                state.selection.frameId
            );
    }

    return
        std::filesystem::path("dumps") /
        "assets" /
        (name + ".txt");
}


bool dumpActiveAsset(
    const CacheExplorerState& state,
    const std::filesystem::path& path,
    std::string& error
) {
    error.clear();

    // Raw animation selection already has a dedicated exhaustive
    // dumper. Reuse it instead of maintaining two representations.
    if (
        state.selection.type ==
            CacheTreeNodeType::Animation &&
        state.activeAnimation.has_value()
    ) {
        return dumpAnimation(
            *state.activeAnimation,
            path,
            error
        );
    }

    try {
        if (!path.parent_path().empty()) {
            std::filesystem::create_directories(
                path.parent_path()
            );
        }

        std::ofstream out(
            path,
            std::ios::binary
        );

        if (!out) {
            error = "Could not open dump file";
            return false;
        }

        writeSelection(
            out,
            state
        );

        out << "[Decoded Data]\n";

        if (state.activeMidi.has_value()) {
            const auto& midi =
                *state.activeMidi;

            out << "\n[MIDI]\n";
            out << "ID: " << midi.id << "\n";
            out << "Tracks: "
                << midi.tracks.size()
                << "\n";
            out << "Division: "
                << midi.division
                << "\n";
            out << "Total ticks: "
                << midi.totalTicks
                << "\n";
            out << "Normalized bytes: "
                << midi.bytes.size()
                << "\n";

            for (
                std::size_t index = 0;
                index < midi.tracks.size();
                ++index
            ) {
                out << "Track " << index
                    << ": events="
                    << midi.tracks[index].events.size()
                    << "\n";
            }

            out << "\n[Raw Source]\n";
            out << "Normalized Standard MIDI stream\n\n";
            out << "[Raw Bytes]\n";
            writeHexBytes(
                out,
                midi.bytes
            );
        }

        if (state.activeMap.has_value()) {
            const auto& map =
                *state.activeMap;

            out << "\n[Map Region]\n";
            out << "ID: "
                << map.indexEntry.regionId
                << "\n";
            out << "Region: "
                << map.indexEntry.regionX()
                << ", "
                << map.indexEntry.regionY()
                << "\n";
            out << "World base: "
                << map.centerRegion.worldBaseX()
                << ", "
                << map.centerRegion.worldBaseY()
                << "\n";
            out << "Objects: "
                << map.centerRegion.locations.size()
                << "\n";
            out << "Scene locs: "
                << map.sceneLocs.size()
                << "\n";
            out << "Terrain triangles: "
                << map.stats.terrainTriangles
                << "\n";
            out << "Object triangles: "
                << map.stats.locTriangles
                << "\n\n";

            out << "Tiles\n";
            for (
                std::size_t plane = 0;
                plane < eld::map::PlaneCount;
                ++plane
            ) {
                for (
                    std::size_t x = 0;
                    x < eld::map::RegionSize;
                    ++x
                ) {
                    for (
                        std::size_t y = 0;
                        y < eld::map::RegionSize;
                        ++y
                    ) {
                        const auto& tile =
                            map.centerRegion.tile(
                                plane,
                                x,
                                y
                            );

                        out << plane << ','
                            << x << ','
                            << y
                            << " height="
                            << tile.height
                            << " underlay="
                            << static_cast<unsigned int>(
                                tile.underlayId
                            )
                            << " overlay="
                            << static_cast<unsigned int>(
                                tile.overlayId
                            )
                            << " shape="
                            << static_cast<unsigned int>(
                                tile.overlayShape
                            )
                            << " rotation="
                            << static_cast<unsigned int>(
                                tile.overlayRotation
                            )
                            << " settings=0x"
                            << std::hex
                            << static_cast<unsigned int>(
                                tile.settings
                            )
                            << std::dec
                            << '\n';
                    }
                }
            }

            out << "\nScene Locations\n";
            for (
                std::size_t index = 0;
                index < map.sceneLocs.size();
                ++index
            ) {
                const auto& loc =
                    map.sceneLocs[index];

                out << index
                    << ": id=" << loc.id
                    << " kind="
                    << static_cast<int>(loc.kind)
                    << " shape="
                    << static_cast<unsigned int>(loc.shape)
                    << " modelType="
                    << static_cast<unsigned int>(loc.modelType)
                    << " rotation="
                    << static_cast<unsigned int>(loc.rotation)
                    << " sourcePlane="
                    << static_cast<unsigned int>(loc.sourcePlane)
                    << " scenePlane="
                    << static_cast<unsigned int>(loc.scenePlane)
                    << " tile="
                    << loc.tileX << ',' << loc.tileZ
                    << " footprint="
                    << loc.footprintWidth << 'x'
                    << loc.footprintLength
                    << " scene="
                    << loc.sceneX << ','
                    << loc.sceneY << ','
                    << loc.sceneZ
                    << " yaw="
                    << loc.sceneYaw
                    << '\n';
            }

            out << "\n[Raw Source]\n";
            out << "Map source bytes are not retained by MapViewState after decode.\n";
        }

        if (state.activeInterface.has_value()) {
            const auto& widget =
                *state.activeInterface;

            out << "\n[Interface]\n";
            out << "ID: " << widget.id << "\n";
            out << "Type: "
                << static_cast<unsigned int>(widget.type)
                << "\n";
            out << "Size: "
                << widget.width << " x "
                << widget.height << "\n";
            out << "Children: "
                << widget.children.size()
                << "\n";
            out << "Conditions: "
                << widget.conditions.size()
                << "\n";
            out << "Scripts: "
                << widget.scripts.size()
                << "\n\n";

            if (!state.activeInterfaceDump.empty()) {
                out << state.activeInterfaceDump
                    << "\n";
            }

            out << "\n[Raw Source]\n";
            out << "Packed interface source bytes are not retained by CacheExplorerState.\n";
        }

        if (state.activeMessage.has_value()) {
            out << "\n[Message]\nID: "
                << state.activeMessage->id
                << "\n";
        }

        if (state.activeMessageAnimation.has_value()) {
            out << "\n[Message Animation]\nID: "
                << state.activeMessageAnimation->id
                << "\n";
        }

        if (state.activeParameter.has_value()) {
            const auto& value =
                *state.activeParameter;

            out << "\n[Parameter]\n";
            out << "ID: " << value.id << "\n";
            out << "Type: "
                << (
                    value.type.has_value()
                        ? std::string(1, *value.type)
                        : std::string("(none)")
                )
                << "\n";
            out << "Auto disable: "
                << (value.autoDisable ? "yes" : "no")
                << "\n";
            out << "Default: ";
            if (value.isString()) {
                out << value.defaultString;
            }
            else {
                out << value.defaultInteger;
            }
            out << "\n";
        }

        if (state.activeVarbit.has_value()) {
            const auto& value =
                *state.activeVarbit;

            out << "\n[Varbit]\n";
            out << "ID: " << value.id << "\n";
            out << "Name: " << value.name << "\n";
            if (value.varpId.has_value()) {
                out << "Varp: " << *value.varpId << "\n";
            }
            out << "LSB: "
                << static_cast<unsigned int>(
                    value.leastSignificantBit
                )
                << "\n";
            out << "MSB: "
                << static_cast<unsigned int>(
                    value.mostSignificantBit
                )
                << "\n";
            out << "Tracked: "
                << (value.tracked ? "yes" : "no")
                << "\n";
        }

        if (state.activeVarp.has_value()) {
            const auto& value =
                *state.activeVarp;

            out << "\n[Varp]\n";
            out << "ID: " << value.id << "\n";
            out << "Name: " << value.name << "\n";
            if (value.clientCode.has_value()) {
                out << "Client code: "
                    << *value.clientCode
                    << "\n";
            }
            out << "Tracked: "
                << (value.tracked ? "yes" : "no")
                << "\n";
            out << "Persistent: "
                << (value.persistent ? "yes" : "no")
                << "\n";
            out << "Active: "
                << (value.active ? "yes" : "no")
                << "\n";
            out << "Mode: "
                << static_cast<int>(value.mode)
                << "\n";
        }

        if (state.activeSpotAnimation.has_value()) {
            const auto& value =
                *state.activeSpotAnimation;

            out << "\n[Spot Animation]\n";
            out << "ID: " << value.id << "\n";
            if (value.modelId.has_value()) {
                out << "Model: " << *value.modelId << "\n";
            }
            if (value.sequenceId.has_value()) {
                out << "Sequence: " << *value.sequenceId << "\n";
            }
            out << "Scale: "
                << value.scaleX << ", "
                << value.scaleY << "\n";
            out << "Rotation: "
                << value.rotation << "\n";
            out << "Ambient: "
                << value.ambient << "\n";
            out << "Contrast: "
                << value.contrast << "\n";
            out << "Recolors:\n";
            for (
                std::size_t index = 0;
                index < value.recolorSources.size();
                ++index
            ) {
                if (
                    value.recolorSources[index].has_value() &&
                    value.recolorDestinations[index].has_value()
                ) {
                    out << "  "
                        << *value.recolorSources[index]
                        << " -> "
                        << *value.recolorDestinations[index]
                        << "\n";
                }
            }
        }

        if (state.activeSequence.has_value()) {
            const auto& value =
                *state.activeSequence;

            out << "\n[Sequence]\n";
            out << "ID: " << value.id << "\n";
            out << "Frames: "
                << value.frames.size() << "\n";
            if (value.frameStep.has_value()) {
                out << "Frame step: "
                    << *value.frameStep << "\n";
            }
            out << "Priority: "
                << static_cast<unsigned int>(value.priority)
                << "\n";
            out << "Maximum loops: "
                << static_cast<unsigned int>(value.maximumLoops)
                << "\n";
            out << "Stretches: "
                << (value.stretches ? "yes" : "no")
                << "\n";

            for (
                std::size_t index = 0;
                index < value.frames.size();
                ++index
            ) {
                const auto& frame =
                    value.frames[index];

                out << index
                    << ": primary="
                    << frame.primaryFrameId;

                if (frame.secondaryFrameId.has_value()) {
                    out << " secondary="
                        << *frame.secondaryFrameId;
                }

                out << " duration="
                    << frame.duration
                    << "\n";
            }
        }

        if (state.activeItem.has_value()) {
            const auto& value =
                *state.activeItem;

            out << "\n[Item]\n";
            out << "ID: " << value.id << "\n";
            out << "Name: " << value.name << "\n";
            if (value.inventoryModelId.has_value()) {
                out << "Inventory model: "
                    << *value.inventoryModelId
                    << "\n";
            }
            out << "Value: " << value.value << "\n";
            out << "Stackable: "
                << (value.stackable ? "yes" : "no")
                << "\n";
            out << "Members only: "
                << (value.membersOnly ? "yes" : "no")
                << "\n";
            out << "Zoom: " << value.zoom << "\n";
            out << "Rotation: "
                << value.rotationX << ", "
                << value.rotationY << ", "
                << value.rotationZ << "\n";
            out << "Offset: "
                << value.offsetX << ", "
                << value.offsetY << "\n";
            out << "Inventory actions:\n";
            for (const auto& action : value.inventoryActions) {
                if (!action.empty()) {
                    out << "  " << action << "\n";
                }
            }
            out << "Ground actions:\n";
            for (const auto& action : value.groundActions) {
                if (!action.empty()) {
                    out << "  " << action << "\n";
                }
            }
        }

        if (state.activeNpc.has_value()) {
            const auto& value =
                *state.activeNpc;

            out << "\n[NPC]\n";
            out << "ID: " << value.id << "\n";
            out << "Name: " << value.name << "\n";
            if (value.combatLevel.has_value()) {
                out << "Combat level: "
                    << *value.combatLevel << "\n";
            }
            out << "Size: "
                << static_cast<int>(value.size)
                << "\n";
            out << "Models:\n";
            for (const auto modelId : value.modelIds) {
                out << "  " << modelId << "\n";
            }
            if (value.idleAnimationId.has_value()) {
                out << "Idle animation: "
                    << *value.idleAnimationId << "\n";
            }
            if (value.walkAnimationId.has_value()) {
                out << "Walk animation: "
                    << *value.walkAnimationId << "\n";
            }
            out << "Scale: "
                << value.scaleX << ", "
                << value.scaleY << "\n";
            out << "Minimap: "
                << (value.visibleOnMinimap ? "visible" : "hidden")
                << "\n";
            out << "Clickable: "
                << (value.clickable ? "yes" : "no")
                << "\n";
            out << "Actions:\n";
            for (const auto& action : value.actions) {
                if (!action.empty()) {
                    out << "  " << action << "\n";
                }
            }
        }

        if (state.activeLocation.has_value()) {
            const auto& value =
                *state.activeLocation;

            out << "\n[Location]\n";
            out << "ID: " << value.id << "\n";
            out << "Name: " << value.name << "\n";
            out << "Size: "
                << value.width << " x "
                << value.length << "\n";
            out << "Models:\n";
            for (const auto& model : value.models) {
                out << "  " << model.id;
                if (model.type.has_value()) {
                    out << " type=" << *model.type;
                }
                out << "\n";
            }
            out << "Solid: "
                << (value.solid ? "yes" : "no")
                << "\n";
            out << "Impenetrable: "
                << (value.impenetrable ? "yes" : "no")
                << "\n";
            if (value.animationId.has_value()) {
                out << "Animation: "
                    << *value.animationId << "\n";
            }
            out << "Scale: "
                << value.scaleX << ", "
                << value.scaleY << ", "
                << value.scaleZ << "\n";
            out << "Offset: "
                << value.offsetX << ", "
                << value.offsetY << ", "
                << value.offsetZ << "\n";
            out << "Actions:\n";
            for (const auto& action : value.actions) {
                if (!action.empty()) {
                    out << "  " << action << "\n";
                }
            }
        }

        if (state.activeIdentityKit.has_value()) {
            const auto& value =
                *state.activeIdentityKit;

            out << "\n[Identity Kit]\n";
            out << "ID: " << value.id << "\n";
            if (value.bodyPartId.has_value()) {
                out << "Body part: "
                    << *value.bodyPartId << "\n";
            }
            out << "Selectable: "
                << (value.selectable ? "yes" : "no")
                << "\n";
            out << "Body models:\n";
            for (const auto modelId : value.modelIds) {
                out << "  " << modelId << "\n";
            }
            out << "Recolors:\n";
            for (
                std::size_t index = 0;
                index < value.recolorSources.size();
                ++index
            ) {
                if (
                    value.recolorSources[index].has_value() &&
                    value.recolorDestinations[index].has_value()
                ) {
                    out << "  "
                        << *value.recolorSources[index]
                        << " -> "
                        << *value.recolorDestinations[index]
                        << "\n";
                }
            }
            out << "Head models:\n";
            for (const auto& modelId : value.headModelIds) {
                if (modelId.has_value()) {
                    out << "  " << *modelId << "\n";
                }
            }
        }

        if (state.activeFloor.has_value()) {
            const auto& value =
                *state.activeFloor;

            out << "\n[Floor]\n";
            out << "ID: " << value.id << "\n";
            out << "Name: " << value.name << "\n";
            if (value.rgb.has_value()) {
                out << "Primary RGB: 0x"
                    << std::hex << *value.rgb
                    << std::dec << "\n";
            }
            if (value.textureId.has_value()) {
                out << "Texture: "
                    << *value.textureId << "\n";
            }
            if (value.secondaryRgb.has_value()) {
                out << "Secondary RGB: 0x"
                    << std::hex << *value.secondaryRgb
                    << std::dec << "\n";
            }
            out << "Occlude: "
                << (value.occlude ? "true" : "false")
                << "\n";
        }

        if (state.activeFont.has_value()) {
            const auto& font =
                *state.activeFont;

            out << "\n[Font]\n";
            out << "Glyphs: "
                << font.glyphs.size()
                << "\n";
            out << "Line height: "
                << static_cast<unsigned int>(
                    font.lineHeight
                )
                << "\n";

            for (
                std::size_t index = 0;
                index < font.glyphs.size();
                ++index
            ) {
                const auto& glyph =
                    font.glyphs[index];

                out << "Glyph " << index
                    << ": size="
                    << glyph.width << 'x'
                    << glyph.height
                    << " offset="
                    << glyph.offsetX << ','
                    << glyph.offsetY
                    << " alphaBytes="
                    << glyph.alpha.size()
                    << "\n";
            }
        }

        if (state.activeTexture.has_value()) {
            out << "\n[Texture]\n";
            writePixels(
                out,
                state.activeTexture->image
            );
        }

        if (state.activeSprite.has_value()) {
            out << "\n[Sprite]\n";
            out << "Frame: "
                << state.selection.frameId
                << "\n";
            writePixels(
                out,
                state.activeSprite->image
            );
        }

        if (state.activeImage.has_value()) {
            out << "\n[Image]\n";
            writePixels(
                out,
                *state.activeImage
            );
        }

        if (state.activeModel.has_value()) {
            out << "\n";
            writeModel(
                out,
                *state.activeModel
            );
        }

        if (
            !state.activeAnimation.has_value() &&
            !state.activeMidi.has_value() &&
            !state.activeMap.has_value() &&
            !state.activeInterface.has_value() &&
            !state.activeMessage.has_value() &&
            !state.activeMessageAnimation.has_value() &&
            !state.activeParameter.has_value() &&
            !state.activeVarbit.has_value() &&
            !state.activeVarp.has_value() &&
            !state.activeSpotAnimation.has_value() &&
            !state.activeSequence.has_value() &&
            !state.activeItem.has_value() &&
            !state.activeNpc.has_value() &&
            !state.activeLocation.has_value() &&
            !state.activeIdentityKit.has_value() &&
            !state.activeFloor.has_value() &&
            !state.activeFont.has_value() &&
            !state.activeTexture.has_value() &&
            !state.activeSprite.has_value() &&
            !state.activeImage.has_value() &&
            !state.activeModel.has_value()
        ) {
            out << "No decoded asset object is active for this selection.\n";
        }

        out << "\n[Raw Source]\n";
        out << "If raw bytes are not listed above, the current decoded state does not retain them.\n";

        if (!out.good()) {
            error = "Failed while writing dump file";
            return false;
        }

        return true;
    }
    catch (const std::exception& exception) {
        error = exception.what();
        return false;
    }
}

}
