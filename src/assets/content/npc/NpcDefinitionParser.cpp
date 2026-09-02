#include "NpcDefinitionParser.h"

#include <exception>
#include <utility>

#include "binary/ByteReader.h"

namespace eld::definition {

namespace {

std::optional<std::uint16_t> readOptionalId(
    eld::binary::ByteReader& reader
) {
    const std::uint16_t id =
        reader.readU16();

    return id == 65535
        ? std::nullopt
        : std::optional{id};
}

void readMorphs(
    eld::binary::ByteReader& reader,
    NpcDefinition& definition,
    bool hasFallback
) {
    definition.morphVarbitId =
        readOptionalId(reader);

    definition.morphVarpId =
        readOptionalId(reader);

    std::optional<std::uint16_t> fallback;

    if (hasFallback) {
        fallback =
            readOptionalId(reader);
    }

    const std::uint8_t count =
        reader.readU8();

    definition.morphIds.reserve(
        static_cast<std::size_t>(count) + 2
    );

    for (
        std::uint16_t index = 0;
        index <= count;
        ++index
    ) {
        definition.morphIds.push_back(
            readOptionalId(reader)
        );
    }

    if (hasFallback) {
        definition.morphIds.push_back(
            fallback
        );
    }
}

}

std::optional<NpcDefinition>
NpcDefinitionParser::parse(
    const DefinitionRecord& record
) const {
    try {
        eld::binary::ByteReader reader(
            record.bytes
        );

        NpcDefinition definition;
        definition.id = record.id;

        while (!reader.atEnd()) {
            const std::uint8_t opcode =
                reader.readU8();

            switch (opcode) {
                case 0:
                    return reader.atEnd()
                        ? std::optional{definition}
                        : std::nullopt;

                case 1: {
                    const std::uint8_t count =
                        reader.readU8();

                    definition.modelIds.reserve(count);

                    for (
                        std::uint8_t index = 0;
                        index < count;
                        ++index
                    ) {
                        definition.modelIds.push_back(
                            reader.readU16()
                        );
                    }

                    break;
                }

                case 2:
                    definition.name =
                        reader.readTerminatedString(10);
                    break;

                case 3:
                    definition.description =
                        reader.readTerminatedString(10);
                    break;

                case 12:
                    definition.size =
                        reader.readI8();
                    break;

                case 13:
                    definition.idleAnimationId =
                        readOptionalId(reader);
                    break;

                case 14:
                    definition.walkAnimationId =
                        readOptionalId(reader);
                    break;

                case 17:
                    definition.walkAnimationId =
                        readOptionalId(reader);

                    definition.turnAroundAnimationId =
                        readOptionalId(reader);

                    definition.turnRightAnimationId =
                        readOptionalId(reader);

                    definition.turnLeftAnimationId =
                        readOptionalId(reader);
                    break;

                case 40: {
                    const std::uint8_t count =
                        reader.readU8();

                    definition.recolors.reserve(count);

                    for (
                        std::uint8_t index = 0;
                        index < count;
                        ++index
                    ) {
                        definition.recolors.push_back({
                            reader.readU16(),
                            reader.readU16()
                        });
                    }

                    break;
                }

                case 60: {
                    const std::uint8_t count =
                        reader.readU8();

                    definition.headModelIds.reserve(count);

                    for (
                        std::uint8_t index = 0;
                        index < count;
                        ++index
                    ) {
                        definition.headModelIds.push_back(
                            reader.readU16()
                        );
                    }

                    break;
                }

                case 90:
                case 91:
                case 92:
                    definition.additionalModelIds[
                        opcode - 90
                    ] = readOptionalId(reader);
                    break;

                case 93:
                    definition.visibleOnMinimap = false;
                    break;

                case 95:
                    definition.combatLevel =
                        reader.readU16();
                    break;

                case 97:
                    definition.scaleX =
                        reader.readU16();
                    break;

                case 98:
                    definition.scaleY =
                        reader.readU16();
                    break;

                case 99:
                    definition.renderPriority = true;
                    break;

                case 100:
                    definition.ambient =
                        reader.readI8();
                    break;

                case 101:
                    definition.contrast =
                        reader.readI8();
                    break;

                case 102:
                    definition.headIconId =
                        readOptionalId(reader);
                    break;

                case 103:
                    definition.rotationSpeed =
                        reader.readU16();
                    break;

                case 106:
                    readMorphs(
                        reader,
                        definition,
                        false
                    );
                    break;

                case 107:
                    definition.clickable = false;
                    break;

                case 118:
                    readMorphs(
                        reader,
                        definition,
                        true
                    );
                    break;

                default:
                    if (
                        opcode >= 30 &&
                        opcode < 35
                    ) {
                        std::string action =
                            reader.readTerminatedString(10);

                        if (action != "hidden") {
                            definition.actions[opcode - 30] =
                                std::move(action);
                        }

                        break;
                    }

                    return std::nullopt;
            }
        }

        return std::nullopt;
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

}
