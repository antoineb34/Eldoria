#include "decoders/ParameterDecoder.h"

#include <cstdint>
#include <span>
#include <stdexcept>

#include "binary/ByteReader.h"

namespace eld::parameter {

Parameter ParameterDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    eld::binary::ByteReader reader(payload);

    Parameter parameter;

    while (!reader.atEnd()) {
        const auto opcode = reader.readU8();

        switch (opcode) {
            case 0:
                if (!reader.atEnd()) {
                    throw std::runtime_error(
                        "Parameter payload contains trailing data"
                    );
                }

                return parameter;

            case 1:
                parameter.type =
                    static_cast<char>(
                        reader.readU8()
                    );
                break;

            case 2:
                parameter.defaultInteger =
                    reader.readI32();
                break;

            case 4:
                parameter.autoDisable = false;
                break;

            case 5:
                parameter.defaultString =
                    reader.readTerminatedString(10);
                break;

            default:
                throw std::runtime_error(
                    "Unknown parameter opcode"
                );
        }
    }

    throw std::runtime_error(
        "Parameter payload is missing terminator"
    );
}

}
