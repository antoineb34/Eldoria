#include "decoders/VarpDecoder.h"

#include <cstdint>
#include <span>
#include <stdexcept>

#include "binary/ByteReader.h"

namespace eld::varp {

Varp VarpDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    eld::binary::ByteReader reader(payload);

    Varp varp;

    while (!reader.atEnd()) {
        const auto opcode = reader.readU8();

        switch (opcode) {
            case 0:
                if (!reader.atEnd()) {
                    throw std::runtime_error(
                        "Varp payload contains trailing data"
                    );
                }

                return varp;

            case 1:
                varp.opcode1Value =
                    reader.readU8();
                break;

            case 2:
                varp.opcode2Value =
                    reader.readU8();
                break;

            case 3:
                varp.tracked = true;
                break;

            case 4:
                varp.persistent = false;
                break;

            case 5:
                varp.clientCode =
                    reader.readU16();
                break;

            case 6:
                varp.opcode6Flag = true;
                break;

            case 7:
                varp.opcode7Value =
                    reader.readU32();
                break;

            case 8:
                varp.active = true;
                varp.mode = 1;
                break;

            case 10:
                varp.name =
                    reader.readTerminatedString(10);
                break;

            case 11:
                varp.active = true;
                break;

            case 12:
                varp.opcode12Value =
                    reader.readU32();
                break;

            case 13:
                varp.mode = 2;
                break;

            default:
                throw std::runtime_error(
                    "Unknown varp opcode"
                );
        }
    }

    throw std::runtime_error(
        "Varp payload is missing terminator"
    );
}

}
