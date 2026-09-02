#include "FloorDefinitionParser.h"

#include <exception>

#include "binary/ByteReader.h"

namespace eld::definition {

std::optional<FloorDefinition>
FloorDefinitionParser::parse(
    const DefinitionRecord& record
) const {
    try {
        eld::binary::ByteReader reader(
            record.bytes
        );

        FloorDefinition definition;

        definition.id =
            record.id;

        while (!reader.atEnd()) {
            const std::uint8_t opcode =
                reader.readU8();

            switch (opcode) {
                case 0:
                    return reader.atEnd()
                        ? std::optional{
                              definition
                          }
                        : std::nullopt;

                case 1:
                    definition.rgb =
                        reader.readU24();
                    break;

                case 2:
                    definition.textureId =
                        reader.readU8();
                    break;

                case 3:
                    break;

                case 5:
                    definition.occlude =
                        false;
                    break;

                case 6:
                    definition.name =
                        reader.readTerminatedString(10);
                    break;

                case 7:
                    definition.secondaryRgb =
                        reader.readU24();
                    break;

                default:
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
