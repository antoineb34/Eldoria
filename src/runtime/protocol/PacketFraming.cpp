#include "PacketFraming.h"

#include <cstddef>
#include <stdexcept>

#include "Packet.h"
#include "binary/ByteWriter.h"


namespace eld::protocol
{

    std::vector<std::uint8_t> framePacket(
        std::uint8_t opcode,
        std::span<const std::uint8_t> payload
    )
    {
        if (
            payload.size() >
            MaxPacketPayload
        ) {
            throw std::length_error(
                "Packet payload exceeds maximum size"
            );
        }


        eld::binary::ByteWriter writer(
            3 +
            payload.size()
        );


        writer.writeU16(
            static_cast<std::uint16_t>(
                payload.size()
            )
        );


        writer.writeU8(
            opcode
        );


        writer.writeBytes(
            payload
        );


        return
            writer.takeData();
    }

}
