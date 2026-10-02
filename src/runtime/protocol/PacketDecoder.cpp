#include "PacketDecoder.h"

#include <cstddef>
#include <stdexcept>

#include "binary/ByteReader.h"


namespace eld::protocol
{

    void PacketDecoder::append(
        std::span<const std::uint8_t> bytes
    )
    {
        buffer_.insert(
            buffer_.end(),
            bytes.begin(),
            bytes.end()
        );
    }


    std::optional<Packet>
    PacketDecoder::next()
    {
        constexpr std::size_t
            HeaderSize =
                3;


        if (
            buffer_.size() <
            HeaderSize
        ) {
            return std::nullopt;
        }


        eld::binary::ByteReader
            header(
                buffer_
            );


        const auto payloadLength =
            static_cast<std::size_t>(
                header.readU16()
            );


        const auto opcode =
            header.readU8();


        if (
            payloadLength >
            MaxPacketPayload
        ) {
            throw std::runtime_error(
                "PacketDecoder: payload exceeds maximum size"
            );
        }


        const auto frameLength =
            HeaderSize +
            payloadLength;


        if (
            buffer_.size() <
            frameLength
        ) {
            return std::nullopt;
        }


        Packet packet;


        packet.opcode =
            opcode;


        packet.payload.assign(
            buffer_.begin() +
                HeaderSize,
            buffer_.begin() +
                frameLength
        );


        buffer_.erase(
            buffer_.begin(),
            buffer_.begin() +
                frameLength
        );


        return packet;
    }

}
