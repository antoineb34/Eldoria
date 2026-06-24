#include "ParameterDefinitionParser.h"

#include <exception>

#include "binary/ByteReader.h"

namespace eld::definition {

std::optional<ParameterDefinition>
ParameterDefinitionParser::parse(
    const DefinitionRecord& record
) const {
    try {
        eld::binary::ByteReader reader(
            record.bytes
        );

        ParameterDefinition definition;
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
                    definition.type =
                        static_cast<char>(
                            reader.readU8()
                        );
                    break;

                case 2:
                    definition.defaultInteger =
                        reader.readI32();
                    break;

                case 4:
                    definition.autoDisable = false;
                    break;

                case 5:
                    definition.defaultString =
                        reader.readTerminatedString(10);
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
