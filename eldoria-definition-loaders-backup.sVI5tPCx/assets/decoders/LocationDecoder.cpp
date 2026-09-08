#include "decoders/LocationDecoder.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>

#include "binary/ByteReader.h"

namespace eld::location {

namespace {

std::optional<std::uint16_t> readOptionalId(
    eld::binary::ByteReader& reader
) {
    const auto id = reader.readU16();

    return
        id == 65535
            ? std::nullopt
            : std::optional{id};
}


void readMorphs(
    eld::binary::ByteReader& reader,
    Location& location,
    bool hasFallback
) {
    location.morphVarbitId =
        readOptionalId(reader);

    location.morphVarpId =
        readOptionalId(reader);

    std::optional<std::uint16_t> fallback;

    if (hasFallback) {
        fallback = readOptionalId(reader);
    }

    const auto count = reader.readU8();

    location.morphIds.reserve(
        static_cast<std::size_t>(count) +
        1 +
        hasFallback
    );

    for (
        std::uint16_t index = 0;
        index <= count;
        ++index
    ) {
        location.morphIds.push_back(
            readOptionalId(reader)
        );
    }

    if (hasFallback) {
        location.morphIds.push_back(
            fallback
        );
    }
}

}


Location LocationDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    eld::binary::ByteReader reader(payload);

    Location location;


    // Opcodes

    while (!reader.atEnd()) {
        const auto opcode = reader.readU8();

        switch (opcode) {
            case 0:
                if (!reader.atEnd()) {
                    throw std::runtime_error(
                        "Invalid location payload"
                    );
                }

                return location;

            case 1: {
                const auto count = reader.readU8();

                location.models.clear();
                location.models.reserve(count);

                for (
                    std::uint8_t index = 0;
                    index < count;
                    ++index
                ) {
                    location.models.push_back({
                        reader.readU16(),
                        reader.readU8()
                    });
                }

                break;
            }

            case 2:
                location.name =
                    reader.readTerminatedString(10);
                break;

            case 3:
                location.description =
                    reader.readTerminatedString(10);
                break;

            case 5: {
                const auto count = reader.readU8();

                location.models.clear();
                location.models.reserve(count);

                for (
                    std::uint8_t index = 0;
                    index < count;
                    ++index
                ) {
                    location.models.push_back({
                        reader.readU16(),
                        std::nullopt
                    });
                }

                break;
            }

            case 14:
                location.width = reader.readU8();
                break;

            case 15:
                location.length = reader.readU8();
                break;

            case 17:
                location.solid = false;
                break;

            case 18:
                location.impenetrable = false;
                break;

            case 19:
                location.interactionType =
                    reader.readU8();
                break;

            case 21:
                location.contouredGround = true;
                break;

            case 22:
                location.nonFlatShading = true;
                break;

            case 23:
                location.modelClipped = true;
                break;

            case 24:
                location.animationId =
                    readOptionalId(reader);
                break;

            case 28:
                location.decorDisplacement =
                    reader.readU8();
                break;

            case 29:
                location.ambient = reader.readI8();
                break;

            case 39:
                location.contrast = reader.readI8();
                break;

            case 40: {
                const auto count = reader.readU8();

                location.recolors.reserve(count);

                for (
                    std::uint8_t index = 0;
                    index < count;
                    ++index
                ) {
                    location.recolors.push_back({
                        reader.readU16(),
                        reader.readU16()
                    });
                }

                break;
            }

            case 60:
                location.mapFunctionId =
                    readOptionalId(reader);
                break;

            case 62:
                location.rotated = true;
                break;

            case 64:
                location.castsShadow = false;
                break;

            case 65:
                location.scaleX = reader.readU16();
                break;

            case 66:
                location.scaleY = reader.readU16();
                break;

            case 67:
                location.scaleZ = reader.readU16();
                break;

            case 68:
                location.mapSceneId =
                    readOptionalId(reader);
                break;

            case 69:
                location.surroundings =
                    reader.readU8();
                break;

            case 70:
                location.offsetX = reader.readI16();
                break;

            case 71:
                location.offsetY = reader.readI16();
                break;

            case 72:
                location.offsetZ = reader.readI16();
                break;

            case 73:
                location.obstructsGround = true;
                break;

            case 74:
                location.hollow = true;
                break;

            case 75:
                location.supportItems =
                    reader.readU8();
                break;

            case 77:
                readMorphs(
                    reader,
                    location,
                    false
                );
                break;

            case 92:
                readMorphs(
                    reader,
                    location,
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
                        location.actions[
                            opcode - 30
                        ] = std::move(action);
                    }

                    break;
                }

                throw std::runtime_error(
                    "Unknown location opcode " +
                    std::to_string(opcode)
                );
        }
    }

    throw std::runtime_error(
        "Location payload is missing terminator"
    );
}

}
