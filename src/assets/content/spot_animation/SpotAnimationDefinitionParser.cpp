#include "SpotAnimationDefinitionParser.h"

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

}

std::optional<SpotAnimationDefinition>
SpotAnimationDefinitionParser::parse(
    const DefinitionRecord& record
) const {
    try {
        eld::binary::ByteReader reader(
            record.bytes
        );

        SpotAnimationDefinition definition;
        definition.id = record.id;

        while (!reader.atEnd()) {
            const std::uint8_t opcode =
                reader.readU8();

            switch (opcode) {
                case 0:
                    return reader.atEnd()
                        ? std::optional{definition}
                        : std::nullopt;

                case 1:
                    definition.modelId =
                        readOptionalId(reader);
                    break;

                case 2:
                    definition.sequenceId =
                        readOptionalId(reader);
                    break;

                case 4:
                    definition.scaleX =
                        reader.readU16();
                    break;

                case 5:
                    definition.scaleY =
                        reader.readU16();
                    break;

                case 6:
                    definition.rotation =
                        reader.readU16();
                    break;

                case 7:
                    definition.ambient =
                        reader.readU8();
                    break;

                case 8:
                    definition.contrast =
                        reader.readU8();
                    break;

                default:
                    if (
                        opcode >= 40 &&
                        opcode < 50
                    ) {
                        definition.recolorSources[
                            opcode - 40
                        ] = reader.readU16();

                        break;
                    }

                    if (
                        opcode >= 50 &&
                        opcode < 60
                    ) {
                        definition.recolorDestinations[
                            opcode - 50
                        ] = reader.readU16();

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
