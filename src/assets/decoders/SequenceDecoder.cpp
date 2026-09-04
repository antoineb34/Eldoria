#include "decoders/SequenceDecoder.h"

#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>

#include "binary/ByteReader.h"

namespace eld::sequence {

namespace {

std::optional<std::uint16_t> readOptionalId(
    eld::binary::ByteReader& reader
) {
    const auto id = reader.readU16();

    return id == 65535
        ? std::nullopt
        : std::optional{id};
}

}


Sequence SequenceDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    eld::binary::ByteReader reader(payload);

    Sequence sequence;

    while (!reader.atEnd()) {
        const auto opcode = reader.readU8();

        switch (opcode) {
            case 0:
                if (!reader.atEnd()) {
                    throw std::runtime_error(
                        "Sequence payload contains trailing data"
                    );
                }

                return sequence;

            case 1: {
                const auto count = reader.readU8();

                sequence.frames.reserve(count);

                for (
                    std::uint8_t index = 0;
                    index < count;
                    ++index
                ) {
                    sequence.frames.push_back({
                        reader.readU16(),
                        readOptionalId(reader),
                        reader.readU16()
                    });
                }

                break;
            }

            case 2:
                sequence.frameStep =
                    reader.readU16();
                break;

            case 3: {
                const auto count = reader.readU8();

                sequence.interleaveOrder.reserve(count);

                for (
                    std::uint8_t index = 0;
                    index < count;
                    ++index
                ) {
                    sequence.interleaveOrder.push_back(
                        reader.readU8()
                    );
                }

                break;
            }

            case 4:
                sequence.stretches = true;
                break;

            case 5:
                sequence.priority =
                    reader.readU8();
                break;

            case 6:
                sequence.shieldItemId =
                    readOptionalId(reader);
                break;

            case 7:
                sequence.weaponItemId =
                    readOptionalId(reader);
                break;

            case 8:
                sequence.maximumLoops =
                    reader.readU8();
                break;

            case 9:
                sequence.animatingPrecedence =
                    reader.readU8();
                break;

            case 10:
                sequence.walkingPrecedence =
                    reader.readU8();
                break;

            case 11:
                sequence.replayMode =
                    reader.readU8();
                break;

            case 12:
                sequence.packedData =
                    reader.readU32();
                break;

            default:
                throw std::runtime_error(
                    "Unknown sequence opcode"
                );
        }
    }

    throw std::runtime_error(
        "Sequence payload is missing terminator"
    );
}

}
