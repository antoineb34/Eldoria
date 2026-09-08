#include "decoders/FloorDecoder.h"

#include <cstdint>
#include <span>
#include <stdexcept>

#include "binary/ByteReader.h"

namespace eld::floor {

Floor FloorDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    eld::binary::ByteReader reader(payload);

    Floor floor;


    // Properties

    while (!reader.atEnd()) {
        const auto opcode = reader.readU8();

        switch (opcode) {
            case 0:
                if (!reader.atEnd()) {
                    throw std::runtime_error(
                        "Invalid floor layout"
                    );
                }

                return floor;

            case 1:
                floor.rgb = reader.readU24();
                break;

            case 2:
                floor.textureId = reader.readU8();
                break;

            case 3:
                break;

            case 5:
                floor.occlude = false;
                break;

            case 6:
                floor.name =
                    reader.readTerminatedString(10);
                break;

            case 7:
                floor.secondaryRgb = reader.readU24();
                break;

            default:
                throw std::runtime_error(
                    "Unknown floor opcode"
                );
        }
    }

    throw std::runtime_error(
        "Floor payload is missing terminator"
    );
}

}
