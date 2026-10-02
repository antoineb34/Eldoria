#include "HelloPackets.h"

#include <span>
#include <stdexcept>

#include "Opcodes.h"
#include "PacketFraming.h"
#include "binary/ByteReader.h"
#include "binary/ByteWriter.h"


namespace eld::protocol
{

    std::vector<std::uint8_t> encode(
        const ClientHello& packet
    )
    {
        eld::binary::ByteWriter
            payload;


        payload.writeU16(
            packet.protocolVersion
        );


        const auto& bytes =
            payload.data();


        return
            framePacket(
                static_cast<std::uint8_t>(
                    ClientOpcode::Hello
                ),

                std::span<const std::uint8_t>(
                    bytes.data(),
                    bytes.size()
                )
            );
    }


    std::vector<std::uint8_t> encode(
        const ServerHello& packet
    )
    {
        eld::binary::ByteWriter
            payload;


        payload.writeU8(
            packet.accepted
                ? 1
                : 0
        );


        payload.writeU16(
            packet.protocolVersion
        );


        const auto& bytes =
            payload.data();


        return
            framePacket(
                static_cast<std::uint8_t>(
                    ServerOpcode::Hello
                ),

                std::span<const std::uint8_t>(
                    bytes.data(),
                    bytes.size()
                )
            );
    }


    ClientHello decodeClientHello(
        const Packet& packet
    )
    {
        if (
            packet.opcode !=
            static_cast<std::uint8_t>(
                ClientOpcode::Hello
            )
        ) {
            throw std::runtime_error(
                "Not a ClientHello packet"
            );
        }


        eld::binary::ByteReader
            reader(
                packet.payload
            );


        ClientHello result;


        result.protocolVersion =
            reader.readU16();


        if (
            !reader.atEnd()
        ) {
            throw std::runtime_error(
                "ClientHello contains trailing bytes"
            );
        }


        return result;
    }


    ServerHello decodeServerHello(
        const Packet& packet
    )
    {
        if (
            packet.opcode !=
            static_cast<std::uint8_t>(
                ServerOpcode::Hello
            )
        ) {
            throw std::runtime_error(
                "Not a ServerHello packet"
            );
        }


        eld::binary::ByteReader
            reader(
                packet.payload
            );


        const auto accepted =
            reader.readU8();


        if (
            accepted >
            1
        ) {
            throw std::runtime_error(
                "ServerHello contains invalid accepted flag"
            );
        }


        ServerHello result;


        result.accepted =
            accepted != 0;


        result.protocolVersion =
            reader.readU16();


        if (
            !reader.atEnd()
        ) {
            throw std::runtime_error(
                "ServerHello contains trailing bytes"
            );
        }


        return result;
    }

}
