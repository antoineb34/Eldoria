#include "decoders/IdentityKitDecoder.h"

#include <cstdint>
#include <span>
#include <stdexcept>

#include "binary/ByteReader.h"

namespace eld::identity_kit {

IdentityKit IdentityKitDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    eld::binary::ByteReader reader(payload);

    IdentityKit kit;


    // Properties

    while (!reader.atEnd()) {
        const auto opcode = reader.readU8();

        switch (opcode) {
            case 0:
                if (!reader.atEnd()) {
                    throw std::runtime_error(
                        "Invalid identity-kit layout"
                    );
                }

                return kit;

            case 1:
                kit.bodyPartId = reader.readU8();
                break;

            case 2: {
                const auto count = reader.readU8();

                kit.modelIds.reserve(count);

                for (
                    std::uint8_t index = 0;
                    index < count;
                    ++index
                ) {
                    kit.modelIds.push_back(
                        reader.readU16()
                    );
                }

                break;
            }

            case 3:
                kit.selectable = false;
                break;

            default:
                if (
                    opcode >= 40 &&
                    opcode < 50
                ) {
                    kit.recolorSources[
                        opcode - 40
                    ] =
                        reader.readU16();

                    break;
                }

                if (
                    opcode >= 50 &&
                    opcode < 60
                ) {
                    kit.recolorDestinations[
                        opcode - 50
                    ] =
                        reader.readU16();

                    break;
                }

                if (
                    opcode >= 60 &&
                    opcode < 65
                ) {
                    kit.headModelIds[
                        opcode - 60
                    ] =
                        reader.readU16();

                    break;
                }

                throw std::runtime_error(
                    "Unknown identity-kit opcode"
                );
        }
    }

    throw std::runtime_error(
        "Identity-kit payload is missing terminator"
    );
}

}
