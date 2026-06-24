#include "IdentityKitDefinitionParser.h"

#include "binary/ByteReader.h"

namespace eld::definition {

std::optional<IdentityKitDefinition>
IdentityKitDefinitionParser::parse(
    std::uint16_t id,
    const std::vector<std::uint8_t>& data
) const {
    try {
        eld::binary::ByteReader reader(data);

        IdentityKitDefinition definition{};
        definition.id = id;

        while (reader.remaining() > 0) {
            const std::uint8_t opcode = reader.readU8();

            if (opcode == 0) {
                return definition;
            }

            if (opcode == 1) {
                definition.bodyPartId = reader.readU8();
                continue;
            }

            if (opcode == 2) {
                const std::uint8_t count = reader.readU8();
                definition.modelIds.reserve(count);

                for (std::uint8_t index = 0; index < count; ++index) {
                    definition.modelIds.push_back(reader.readU16());
                }

                continue;
            }

            if (opcode == 3) {
                definition.selectable = false;
                continue;
            }

            if (opcode >= 40 && opcode < 50) {
                definition.recolorSources[opcode - 40] =
                    reader.readU16();
                continue;
            }

            if (opcode >= 50 && opcode < 60) {
                definition.recolorDestinations[opcode - 50] =
                    reader.readU16();
                continue;
            }

            if (opcode >= 60 && opcode < 65) {
                definition.headModelIds[opcode - 60] =
                    reader.readU16();
                continue;
            }

            return std::nullopt;
        }
    }
    catch (...) {
        return std::nullopt;
    }

    return std::nullopt;
}

}
