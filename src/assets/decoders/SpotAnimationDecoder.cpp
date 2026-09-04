#include "decoders/SpotAnimationDecoder.h"

#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>

#include "binary/ByteReader.h"

namespace eld::spot_animation {

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


SpotAnimation SpotAnimationDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    eld::binary::ByteReader reader(payload);

    SpotAnimation spotAnimation;

    while (!reader.atEnd()) {
        const auto opcode = reader.readU8();

        switch (opcode) {
            case 0:
                if (!reader.atEnd()) {
                    throw std::runtime_error(
                        "Spot animation payload contains trailing data"
                    );
                }

                return spotAnimation;

            case 1:
                spotAnimation.modelId =
                    readOptionalId(reader);
                break;

            case 2:
                spotAnimation.sequenceId =
                    readOptionalId(reader);
                break;

            case 4:
                spotAnimation.scaleX =
                    reader.readU16();
                break;

            case 5:
                spotAnimation.scaleY =
                    reader.readU16();
                break;

            case 6:
                spotAnimation.rotation =
                    reader.readU16();
                break;

            case 7:
                spotAnimation.ambient =
                    reader.readU8();
                break;

            case 8:
                spotAnimation.contrast =
                    reader.readU8();
                break;

            default:
                if (
                    opcode >= 40 &&
                    opcode < 50
                ) {
                    spotAnimation.recolorSources[
                        opcode - 40
                    ] = reader.readU16();

                    break;
                }

                if (
                    opcode >= 50 &&
                    opcode < 60
                ) {
                    spotAnimation.recolorDestinations[
                        opcode - 50
                    ] = reader.readU16();

                    break;
                }

                throw std::runtime_error(
                    "Unknown spot animation opcode"
                );
        }
    }

    throw std::runtime_error(
        "Spot animation payload is missing terminator"
    );
}

}
