#include "decoders/NpcDecoder.h"

#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>

#include "binary/ByteReader.h"

namespace eld::npc {

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
    Npc& npc,
    bool hasFallback
) {
    npc.morphVarbitId =
        readOptionalId(reader);

    npc.morphVarpId =
        readOptionalId(reader);

    std::optional<std::uint16_t> fallback;

    if (hasFallback) {
        fallback =
            readOptionalId(reader);
    }

    const std::uint8_t count =
        reader.readU8();

    npc.morphIds.reserve(
        static_cast<std::size_t>(count) + 2
    );

    for (
        std::uint16_t index = 0;
        index <= count;
        ++index
    ) {
        npc.morphIds.push_back(
            readOptionalId(reader)
        );
    }

    if (hasFallback) {
        npc.morphIds.push_back(
            fallback
        );
    }
}

}

Npc NpcDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    eld::binary::ByteReader reader(payload);

    Npc npc;

    while (!reader.atEnd()) {
            const std::uint8_t opcode =
                reader.readU8();

            switch (opcode) {
                case 0:
                    if (!reader.atEnd()) {
                        throw std::runtime_error(
                            "NPC payload contains trailing data"
                        );
                    }

                    return npc;

                case 1: {
                    const std::uint8_t count =
                        reader.readU8();

                    npc.modelIds.reserve(count);

                    for (
                        std::uint8_t index = 0;
                        index < count;
                        ++index
                    ) {
                        npc.modelIds.push_back(
                            reader.readU16()
                        );
                    }

                    break;
                }

                case 2:
                    npc.name =
                        reader.readTerminatedString(10);
                    break;

                case 3:
                    npc.description =
                        reader.readTerminatedString(10);
                    break;

                case 12:
                    npc.size =
                        reader.readI8();
                    break;

                case 13:
                    npc.idleAnimationId =
                        readOptionalId(reader);
                    break;

                case 14:
                    npc.walkAnimationId =
                        readOptionalId(reader);
                    break;

                case 17:
                    npc.walkAnimationId =
                        readOptionalId(reader);

                    npc.turnAroundAnimationId =
                        readOptionalId(reader);

                    npc.turnRightAnimationId =
                        readOptionalId(reader);

                    npc.turnLeftAnimationId =
                        readOptionalId(reader);
                    break;

                case 40: {
                    const std::uint8_t count =
                        reader.readU8();

                    npc.recolors.reserve(count);

                    for (
                        std::uint8_t index = 0;
                        index < count;
                        ++index
                    ) {
                        npc.recolors.push_back({
                            reader.readU16(),
                            reader.readU16()
                        });
                    }

                    break;
                }

                case 60: {
                    const std::uint8_t count =
                        reader.readU8();

                    npc.headModelIds.reserve(count);

                    for (
                        std::uint8_t index = 0;
                        index < count;
                        ++index
                    ) {
                        npc.headModelIds.push_back(
                            reader.readU16()
                        );
                    }

                    break;
                }

                case 90:
                case 91:
                case 92:
                    npc.additionalModelIds[
                        opcode - 90
                    ] = readOptionalId(reader);
                    break;

                case 93:
                    npc.visibleOnMinimap = false;
                    break;

                case 95:
                    npc.combatLevel =
                        reader.readU16();
                    break;

                case 97:
                    npc.scaleX =
                        reader.readU16();
                    break;

                case 98:
                    npc.scaleY =
                        reader.readU16();
                    break;

                case 99:
                    npc.renderPriority = true;
                    break;

                case 100:
                    npc.ambient =
                        reader.readI8();
                    break;

                case 101:
                    npc.contrast =
                        reader.readI8();
                    break;

                case 102:
                    npc.headIconId =
                        readOptionalId(reader);
                    break;

                case 103:
                    npc.rotationSpeed =
                        reader.readU16();
                    break;

                case 106:
                    readMorphs(
                        reader,
                        npc,
                        false
                    );
                    break;

                case 107:
                    npc.clickable = false;
                    break;

                case 118:
                    readMorphs(
                        reader,
                        npc,
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
                            npc.actions[opcode - 30] =
                                std::move(action);
                        }

                        break;
                    }

                    throw std::runtime_error(
                        "Unknown NPC opcode"
                    );
            }
        }

    throw std::runtime_error(
        "NPC payload is missing terminator"
    );
}

}
