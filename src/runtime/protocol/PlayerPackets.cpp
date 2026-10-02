#include "PlayerPackets.h"

#include <stdexcept>

#include "Opcodes.h"
#include "PacketFraming.h"
#include "binary/ByteReader.h"
#include "binary/ByteWriter.h"


namespace eld::protocol
{

    std::vector<std::uint8_t> encode(
        const PlayerPosition& packet
    )
    {
        eld::binary::ByteWriter
            payload;


        payload.writeU16(
            packet.x
        );


        payload.writeU16(
            packet.y
        );


        payload.writeU8(
            packet.plane
        );


        return
            framePacket(
                static_cast<std::uint8_t>(
                    ServerOpcode::PlayerPosition
                ),
                payload.data()
            );
    }


    PlayerPosition decodePlayerPosition(
        const Packet& packet
    )
    {
        if (
            packet.opcode !=
            static_cast<std::uint8_t>(
                ServerOpcode::PlayerPosition
            )
        ) {
            throw std::runtime_error(
                "Not a PlayerPosition packet"
            );
        }


        eld::binary::ByteReader
            reader(
                packet.payload
            );


        PlayerPosition result{
            .x =
                reader.readU16(),

            .y =
                reader.readU16(),

            .plane =
                reader.readU8()
        };


        if (
            !reader.atEnd()
        ) {
            throw std::runtime_error(
                "PlayerPosition contains trailing bytes"
            );
        }


        return result;
    }

}
