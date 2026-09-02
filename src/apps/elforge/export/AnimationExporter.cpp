#include "export/AnimationExporter.h"

#include <fstream>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>

namespace eld::elforge {

namespace {

std::string jsonEscape(
    std::string_view value
) {
    std::ostringstream output;

    for (const unsigned char character : value) {
        switch (character) {
            case '"': output << "\\\""; break;
            case '\\': output << "\\\\"; break;
            case '\b': output << "\\b"; break;
            case '\f': output << "\\f"; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default:
                if (character < 0x20u) {
                    output
                        << "\\u"
                        << std::hex
                        << std::setw(4)
                        << std::setfill('0')
                        << static_cast<unsigned int>(character)
                        << std::dec
                        << std::setfill(' ');
                }
                else {
                    output << static_cast<char>(character);
                }
                break;
        }
    }

    return output.str();
}

const char* transformTypeName(
    std::uint8_t type
) {
    switch (type) {
        case 0: return "pivot";
        case 1: return "translate";
        case 2: return "rotate";
        case 3: return "scale";
        case 4: return "unknown4";
        case 5: return "alpha";
        default: return "unknown";
    }
}

void writeIndent(
    std::ostream& output,
    int indent
) {
    for (int index = 0; index < indent; ++index) {
        output.put(' ');
    }
}

void writeInfo(
    std::ostream& output,
    const AnimationInspection& info,
    int indent
) {
    const eld::animation::Animation& animation = info.animation;

    writeIndent(output, indent);
    output << "{\n";

    writeIndent(output, indent + 2);
    output << "\"animationId\": " << animation.id << ",\n";

    writeIndent(output, indent + 2);
    output << "\"storedDecodedBytes\": " << animation.bytes.size() << ",\n";

    writeIndent(output, indent + 2);
    output << "\"frameCount\": " << animation.asset.frames.size() << ",\n";

    writeIndent(output, indent + 2);
    output << "\"skeleton\": {\n";
    writeIndent(output, indent + 4);
    output << "\"slotCount\": " << animation.asset.skeleton.slots.size() << ",\n";
    writeIndent(output, indent + 4);
    output << "\"slots\": [\n";

    for (std::size_t index = 0; index < animation.asset.skeleton.slots.size(); ++index) {
        const eld::animation::SkeletonSlot& slot =
            animation.asset.skeleton.slots[index];

        writeIndent(output, indent + 6);
        output
            << "{\"index\": " << index
            << ", \"type\": " << static_cast<unsigned int>(slot.type)
            << ", \"typeName\": \"" << transformTypeName(slot.type)
            << "\", \"groups\": [";

        for (std::size_t group = 0; group < slot.groups.size(); ++group) {
            if (group != 0) {
                output << ", ";
            }
            output << static_cast<unsigned int>(slot.groups[group]);
        }

        output << "]}";
        output << (index + 1 == animation.asset.skeleton.slots.size() ? "\n" : ",\n");
    }

    writeIndent(output, indent + 4);
    output << "]\n";
    writeIndent(output, indent + 2);
    output << "},\n";

    writeIndent(output, indent + 2);
    output << "\"frames\": [\n";

    for (std::size_t index = 0; index < animation.asset.frames.size(); ++index) {
        const eld::animation::AnimationFrame& frame =
            animation.asset.frames[index];

        writeIndent(output, indent + 4);
        output << "{\n";
        writeIndent(output, indent + 6);
        output << "\"id\": " << frame.id << ",\n";
        writeIndent(output, indent + 6);
        output << "\"slotCount\": " << static_cast<unsigned int>(frame.slotCount) << ",\n";
        writeIndent(output, indent + 6);
        output << "\"delay\": " << static_cast<unsigned int>(frame.delay) << ",\n";
        writeIndent(output, indent + 6);
        output << "\"transforms\": [\n";

        for (std::size_t transformIndex = 0; transformIndex < frame.transforms.size(); ++transformIndex) {
            const eld::animation::FrameTransform& transform =
                frame.transforms[transformIndex];

            writeIndent(output, indent + 8);
            output
                << "{\"slot\": " << transform.slot
                << ", \"flags\": " << static_cast<unsigned int>(transform.flags)
                << ", \"x\": " << transform.x
                << ", \"y\": " << transform.y
                << ", \"z\": " << transform.z
                << "}";
            output << (transformIndex + 1 == frame.transforms.size() ? "\n" : ",\n");
        }

        writeIndent(output, indent + 6);
        output << "]\n";
        writeIndent(output, indent + 4);
        output << "}";
        output << (index + 1 == animation.asset.frames.size() ? "\n" : ",\n");
    }

    writeIndent(output, indent + 2);
    output << "],\n";

    writeIndent(output, indent + 2);
    output << "\"sequences\": [\n";

    for (std::size_t index = 0; index < info.sequences.size(); ++index) {
        const AnimationSequenceReference& sequence =
            info.sequences[index];

        writeIndent(output, indent + 4);
        output
            << "{\"id\": " << sequence.sequenceId
            << ", \"matchingPrimaryFrames\": " << sequence.matchingPrimaryFrames
            << ", \"matchingSecondaryFrames\": " << sequence.matchingSecondaryFrames
            << ", \"totalFrameReferences\": " << sequence.totalFrameReferences
            << "}";
        output << (index + 1 == info.sequences.size() ? "\n" : ",\n");
    }

    writeIndent(output, indent + 2);
    output << "],\n";

    writeIndent(output, indent + 2);
    output << "\"uses\": [\n";

    for (std::size_t index = 0; index < info.uses.size(); ++index) {
        const AnimationUse& use = info.uses[index];

        writeIndent(output, indent + 4);
        output
            << "{\"source\": \"" << jsonEscape(use.source)
            << "\", \"id\": " << use.sourceId
            << ", \"name\": \"" << jsonEscape(use.sourceName)
            << "\", \"role\": \"" << jsonEscape(use.role)
            << "\", \"sequenceId\": " << use.sequenceId;

        if (use.viaSpotAnimationId.has_value()) {
            output
                << ", \"viaSpotAnimationId\": "
                << *use.viaSpotAnimationId;
        }

        output
            << ", \"provenance\": \"" << jsonEscape(use.provenance)
            << "\"}";
        output << (index + 1 == info.uses.size() ? "\n" : ",\n");
    }

    writeIndent(output, indent + 2);
    output << "]\n";

    writeIndent(output, indent);
    output << "}";
}

bool openOutput(
    const std::filesystem::path& path,
    std::ofstream& output,
    std::string& error
) {
    try {
        const std::filesystem::path parent = path.parent_path();

        if (!parent.empty()) {
            std::filesystem::create_directories(parent);
        }

        output.open(
            path,
            std::ios::out | std::ios::trunc
        );

        if (!output.is_open()) {
            error = "could not open output file";
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

std::filesystem::path defaultAnimationExportPath(
    std::uint16_t animationId
) {
    return
        std::filesystem::path("exports") /
        "animation" /
        ("animation_" + std::to_string(animationId) + ".json");
}

std::filesystem::path defaultAnimationRelationsExportPath() {
    return
        std::filesystem::path("exports") /
        "animation" /
        "animation_relations.json";
}

bool exportAnimationInspection(
    const AnimationInspection& info,
    const std::filesystem::path& path,
    std::string& error
) {
    std::ofstream output;

    if (!openOutput(path, output, error)) {
        return false;
    }

    output << "{\n  \"schemaVersion\": 1,\n  \"animation\": ";
    writeInfo(output, info, 2);
    output << "\n}\n";

    if (!output.good()) {
        error = "failed while writing animation JSON";
        return false;
    }

    return true;
}

bool exportAllAnimationInspections(
    const AnimationInspector& relations,
    const std::filesystem::path& path,
    std::string& error
) {
    std::ofstream output;

    if (!openOutput(path, output, error)) {
        return false;
    }

    const std::vector<std::uint16_t> ids =
        relations.listIds();

    output
        << "{\n"
        << "  \"schemaVersion\": 1,\n"
        << "  \"animationCount\": " << ids.size() << ",\n"
        << "  \"animations\": [\n";

    try {
        for (std::size_t index = 0; index < ids.size(); ++index) {
            const AnimationInspection info =
                relations.inspect(ids[index]);

            writeInfo(output, info, 4);
            output << (index + 1 == ids.size() ? "\n" : ",\n");
        }
    }
    catch (const std::exception& exception) {
        error = exception.what();
        return false;
    }

    output << "  ]\n}\n";

    if (!output.good()) {
        error = "failed while writing animation relations JSON";
        return false;
    }

    return true;
}

}
