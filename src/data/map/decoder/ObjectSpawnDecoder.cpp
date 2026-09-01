#include "ObjectSpawnDecoder.h"

#include <limits>
#include <stdexcept>

#include "binary/ByteReader.h"

namespace eld::map {

std::vector<MapObjectSpawn> ObjectSpawnDecoder::decode(
    const std::vector<std::uint8_t>& bytes
) const {
    eld::binary::ByteReader reader(bytes);
    std::vector<MapObjectSpawn> objects;

    int objectId = -1;

    while (true) {
        if (reader.atEnd()) {
            throw std::runtime_error(
                "Object map ended before the object-id terminator"
            );
        }

        const int idDelta = reader.readUnsignedSmart();
        if (idDelta == 0) {
            break;
        }

        if (
            idDelta < 0 ||
            objectId > std::numeric_limits<int>::max() - idDelta
        ) {
            throw std::runtime_error(
                "Object-id delta overflow"
            );
        }
        objectId += idDelta;

        int packedLocation = 0;

        while (true) {
            if (reader.atEnd()) {
                throw std::runtime_error(
                    "Object map ended before the placement terminator"
                );
            }

            const int locationDelta = reader.readUnsignedSmart();
            if (locationDelta == 0) {
                break;
            }

            if (
                locationDelta < 1 ||
                locationDelta - 1 >
                    std::numeric_limits<int>::max() - packedLocation
            ) {
                throw std::runtime_error(
                    "Object-location delta overflow"
                );
            }
            packedLocation += locationDelta - 1;

            if (reader.atEnd()) {
                throw std::runtime_error(
                    "Object placement is missing its attributes byte"
                );
            }

            const std::uint8_t attributes = reader.readU8();
            const int plane = (packedLocation >> 12) & 3;
            const int x = (packedLocation >> 6) & 63;
            const int y = packedLocation & 63;
            const int type = attributes >> 2;
            const int rotation = attributes & 3;

            if (objectId < 0 || objectId > 65535 || type > 63) {
                throw std::runtime_error(
                    "Object placement is outside the classic map range"
                );
            }

            objects.push_back(MapObjectSpawn{
                static_cast<std::uint16_t>(objectId),
                static_cast<std::uint8_t>(plane),
                static_cast<std::uint8_t>(x),
                static_cast<std::uint8_t>(y),
                static_cast<std::uint8_t>(type),
                static_cast<std::uint8_t>(rotation)
            });
        }
    }

    if (!reader.atEnd()) {
        throw std::runtime_error(
            "Object decoder left trailing bytes"
        );
    }

    return objects;
}

}
