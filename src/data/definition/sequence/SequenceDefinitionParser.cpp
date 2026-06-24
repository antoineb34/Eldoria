#include "SequenceDefinitionParser.h"

#include <exception>
#include <vector>

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

std::optional<SequenceDefinition>
SequenceDefinitionParser::parse(
    const DefinitionRecord& record
) const {
    try {
        eld::binary::ByteReader reader(
            record.bytes
        );

        SequenceDefinition definition;
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

                    std::vector<std::uint16_t>
                        primaryFrameIds(count);

                    std::vector<
                        std::optional<std::uint16_t>
                    > secondaryFrameIds(count);

                    std::vector<std::uint16_t>
                        durations(count);

                    for (
                        std::uint8_t index = 0;
                        index < count;
                        ++index
                    ) {
                        primaryFrameIds[index] =
                            reader.readU16();

                        secondaryFrameIds[index] =
                            readOptionalId(reader);

                        durations[index] =
                            reader.readU16();
                    }

                    definition.frames.reserve(count);

                    for (
                        std::uint8_t index = 0;
                        index < count;
                        ++index
                    ) {
                        definition.frames.push_back({
                            primaryFrameIds[index],
                            secondaryFrameIds[index],
                            durations[index]
                        });
                    }

                    break;
                }

                case 2:
                    definition.frameStep =
                        reader.readU16();
                    break;

                case 3: {
                    const std::uint8_t count =
                        reader.readU8();

                    definition.interleaveOrder.reserve(
                        count
                    );

                    for (
                        std::uint8_t index = 0;
                        index < count;
                        ++index
                    ) {
                        definition.interleaveOrder.push_back(
                            reader.readU8()
                        );
                    }

                    break;
                }

                case 4:
                    definition.stretches = true;
                    break;

                case 5:
                    definition.priority =
                        reader.readU8();
                    break;

                case 6:
                    definition.shieldItemId =
                        readOptionalId(reader);
                    break;

                case 7:
                    definition.weaponItemId =
                        readOptionalId(reader);
                    break;

                case 8:
                    definition.maximumLoops =
                        reader.readU8();
                    break;

                case 9:
                    definition.animatingPrecedence =
                        reader.readU8();
                    break;

                case 10:
                    definition.walkingPrecedence =
                        reader.readU8();
                    break;

                case 11:
                    definition.replayMode =
                        reader.readU8();
                    break;

                case 12:
                    definition.packedData =
                        reader.readU32();
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
