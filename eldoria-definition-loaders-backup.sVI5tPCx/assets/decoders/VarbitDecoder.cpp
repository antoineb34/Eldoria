#include "decoders/VarbitDecoder.h"

#include <cstdint>
#include <span>
#include <stdexcept>

#include "binary/ByteReader.h"

namespace eld::varbit {

Varbit VarbitDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    eld::binary::ByteReader reader(payload);

    Varbit varbit;

    while (!reader.atEnd()) {
        const auto opcode = reader.readU8();

        switch (opcode) {
            case 0:
                if (!reader.atEnd()) {
                    throw std::runtime_error(
                        "Varbit payload contains trailing data"
                    );
                }

                return varbit;

            case 1:
                varbit.varpId =
                    reader.readU16();

                varbit.leastSignificantBit =
                    reader.readU8();

                varbit.mostSignificantBit =
                    reader.readU8();
                break;

            case 2:
                varbit.tracked = true;
                break;

            case 3:
                varbit.opcode3Value =
                    reader.readU32();
                break;

            case 4:
                varbit.opcode4Value =
                    reader.readU32();
                break;

            case 10:
                varbit.name =
                    reader.readTerminatedString(10);
                break;

            default:
                throw std::runtime_error(
                    "Unknown varbit opcode"
                );
        }
    }

    throw std::runtime_error(
        "Varbit payload is missing terminator"
    );
}

}
