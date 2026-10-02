#include "WalkPackets.h"

#include <stdexcept>

#include "Opcodes.h"
#include "PacketFraming.h"
#include "binary/ByteReader.h"
#include "binary/ByteWriter.h"


namespace eld::protocol
{

    namespace
    {

        void validateStep(
            const WalkStep& step
        )
        {
            if (
                step.dx < -1 ||
                step.dx > 1 ||
                step.dy < -1 ||
                step.dy > 1 ||
                (
                    step.dx == 0 &&
                    step.dy == 0
                )
            ) {
                throw std::runtime_error(
                    "WalkRequest contains invalid step"
                );
            }
        }

    }


    std::vector<std::uint8_t> encode(
        const WalkRequest& packet
    )
    {
        if (
            packet.steps.empty()
        ) {
            throw std::runtime_error(
                "WalkRequest path is empty"
            );
        }


        if (
            packet.steps.size() >
            MaxWalkSteps
        ) {
            throw std::runtime_error(
                "WalkRequest path is too long"
            );
        }


        eld::binary::ByteWriter payload;


        payload.writeU16(
            packet.sourceX
        );


        payload.writeU16(
            packet.sourceY
        );


        payload.writeU8(
            packet.plane
        );


        payload.writeU8(
            packet.running
                ? 1
                : 0
        );


        payload.writeU16(
            static_cast<std::uint16_t>(
                packet.steps.size()
            )
        );


        for (
            const auto& step :
            packet.steps
        ) {
            validateStep(
                step
            );


            payload.writeI8(
                step.dx
            );


            payload.writeI8(
                step.dy
            );
        }


        return
            framePacket(
                static_cast<std::uint8_t>(
                    ClientOpcode::Walk
                ),
                payload.data()
            );
    }


    WalkRequest decodeWalkRequest(
        const Packet& packet
    )
    {
        if (
            packet.opcode !=
            static_cast<std::uint8_t>(
                ClientOpcode::Walk
            )
        ) {
            throw std::runtime_error(
                "Not a WalkRequest packet"
            );
        }


        eld::binary::ByteReader reader(
            packet.payload
        );


        WalkRequest result;


        result.sourceX =
            reader.readU16();


        result.sourceY =
            reader.readU16();


        result.plane =
            reader.readU8();


        const auto running =
            reader.readU8();


        if (
            running >
            1
        ) {
            throw std::runtime_error(
                "WalkRequest contains invalid running flag"
            );
        }


        result.running =
            running != 0;


        const auto stepCount =
            reader.readU16();


        if (
            stepCount == 0 ||
            stepCount >
            MaxWalkSteps
        ) {
            throw std::runtime_error(
                "WalkRequest contains invalid step count"
            );
        }


        if (
            reader.remaining() !=
            static_cast<std::size_t>(
                stepCount
            ) * 2
        ) {
            throw std::runtime_error(
                "WalkRequest payload size does not match step count"
            );
        }


        result.steps.reserve(
            stepCount
        );


        for (
            std::uint16_t i = 0;
            i < stepCount;
            ++i
        ) {
            WalkStep step{
                .dx =
                    reader.readI8(),

                .dy =
                    reader.readI8()
            };


            validateStep(
                step
            );


            result.steps.push_back(
                step
            );
        }


        return result;
    }

}
