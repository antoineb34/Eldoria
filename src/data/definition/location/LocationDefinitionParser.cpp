#include "LocationDefinitionParser.h"

#include <exception>

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
    LocationDefinition& definition,
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

std::optional<LocationDefinition>
LocationDefinitionParser::parse(
    const DefinitionRecord& record
) const {
    try {
        eld::binary::ByteReader reader(
            record.bytes
        );

        LocationDefinition definition;
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

                    definition.models.clear();
                    definition.models.reserve(count);

                    for (
                        std::uint8_t index = 0;
                        index < count;
                        ++index
                    ) {
                        definition.models.push_back({
                            reader.readU16(),
                            reader.readU8()
                        });
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

                case 5: {
                    const std::uint8_t count =
                        reader.readU8();

                    definition.models.clear();
                    definition.models.reserve(count);

                    for (
                        std::uint8_t index = 0;
                        index < count;
                        ++index
                    ) {
                        definition.models.push_back({
                            reader.readU16(),
                            std::nullopt
                        });
                    }

                    break;
                }

                case 14:
                    definition.width =
                        reader.readU8();
                    break;

                case 15:
                    definition.length =
                        reader.readU8();
                    break;

                case 17:
                    definition.solid = false;
                    break;

                case 18:
                    definition.impenetrable = false;
                    break;

                case 19:
                    definition.interactionType =
                        reader.readU8();
                    break;

                case 21:
                    definition.contouredGround = true;
                    break;

                case 22:
                    definition.nonFlatShading = true;
                    break;

                case 23:
                    definition.modelClipped = true;
                    break;

                case 24:
                    definition.animationId =
                        readOptionalId(reader);
                    break;

                case 28:
                    definition.decorDisplacement =
                        reader.readU8();
                    break;

                case 29:
                    definition.ambient =
                        reader.readI8();
                    break;

                case 39:
                    definition.contrast =
                        reader.readI8();
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

                case 60:
                    definition.mapFunctionId =
                        readOptionalId(reader);
                    break;

                case 62:
                    definition.rotated = true;
                    break;

                case 64:
                    definition.castsShadow = false;
                    break;

                case 65:
                    definition.scaleX =
                        reader.readU16();
                    break;

                case 66:
                    definition.scaleY =
                        reader.readU16();
                    break;

                case 67:
                    definition.scaleZ =
                        reader.readU16();
                    break;

                case 68:
                    definition.mapSceneId =
                        readOptionalId(reader);
                    break;

                case 69:
                    definition.surroundings =
                        reader.readU8();
                    break;

                case 70:
                    definition.offsetX =
                        reader.readI16();
                    break;

                case 71:
                    definition.offsetY =
                        reader.readI16();
                    break;

                case 72:
                    definition.offsetZ =
                        reader.readI16();
                    break;

                case 73:
                    definition.obstructsGround = true;
                    break;

                case 74:
                    definition.hollow = true;
                    break;

                case 75:
                    definition.supportItems =
                        reader.readU8();
                    break;

                case 77:
                    readMorphs(
                        reader,
                        definition,
                        false
                    );
                    break;

                case 92:
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
