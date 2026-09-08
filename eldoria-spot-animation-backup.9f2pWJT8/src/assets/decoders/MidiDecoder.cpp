#include "decoders/MidiDecoder.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <sstream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <MidiFile.h>

namespace eld::midi {

namespace {

bool matches(
    std::span<const std::uint8_t> payload,
    std::size_t offset,
    std::string_view text
) {
    if (
        offset > payload.size() ||
        text.size() > payload.size() - offset
    ) {
        return false;
    }

    return std::equal(
        text.begin(),
        text.end(),
        payload.begin() + offset,
        [](char left, std::uint8_t right) {
            return
                static_cast<std::uint8_t>(left) ==
                right;
        }
    );
}


std::uint32_t readU32Le(
    std::span<const std::uint8_t> payload,
    std::size_t offset
) {
    if (
        offset > payload.size() ||
        4 > payload.size() - offset
    ) {
        throw std::runtime_error(
            "RIFF section exceeds payload"
        );
    }

    return
        static_cast<std::uint32_t>(payload[offset]) |
        (static_cast<std::uint32_t>(payload[offset + 1]) << 8) |
        (static_cast<std::uint32_t>(payload[offset + 2]) << 16) |
        (static_cast<std::uint32_t>(payload[offset + 3]) << 24);
}


std::span<const std::uint8_t> unwrap(
    std::span<const std::uint8_t> payload
) {
    if (matches(payload, 0, "MThd")) {
        return payload;
    }

    if (
        payload.size() < 12 ||
        !matches(payload, 0, "RIFF") ||
        !matches(payload, 8, "RMID")
    ) {
        throw std::runtime_error(
            "Unknown MIDI container"
        );
    }

    const auto riffSize = readU32Le(payload, 4);

    const std::uint64_t declaredEnd =
        static_cast<std::uint64_t>(riffSize) + 8;

    if (
        declaredEnd > payload.size() ||
        declaredEnd > std::numeric_limits<std::size_t>::max()
    ) {
        throw std::runtime_error(
            "Invalid RIFF MIDI layout"
        );
    }

    const auto end =
        static_cast<std::size_t>(declaredEnd);

    std::size_t offset = 12;

    while (offset < end) {
        if (
            offset > end ||
            8 > end - offset
        ) {
            throw std::runtime_error(
                "Invalid RIFF MIDI chunk"
            );
        }

        const auto size =
            readU32Le(payload, offset + 4);

        const std::size_t dataOffset =
            offset + 8;

        const std::uint64_t dataEnd =
            static_cast<std::uint64_t>(dataOffset) +
            size;

        if (dataEnd > end) {
            throw std::runtime_error(
                "RIFF MIDI chunk exceeds payload"
            );
        }

        if (matches(payload, offset, "data")) {
            return payload.subspan(
                dataOffset,
                size
            );
        }

        const std::uint64_t next =
            dataEnd + (size & 1);

        if (next > end) {
            throw std::runtime_error(
                "Invalid RIFF MIDI padding"
            );
        }

        offset =
            static_cast<std::size_t>(next);
    }

    throw std::runtime_error(
        "RIFF MIDI data chunk is missing"
    );
}

}


Midi MidiDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    const auto midiPayload =
        unwrap(payload);

    const std::string bytes(
        reinterpret_cast<const char*>(
            midiPayload.data()
        ),
        midiPayload.size()
    );

    std::istringstream stream(
        bytes,
        std::ios::in | std::ios::binary
    );

    smf::MidiFile file;

    if (!file.readSmf(stream)) {
        throw std::runtime_error(
            "Invalid MIDI payload"
        );
    }

    file.makeAbsoluteTicks();


    // MIDI

    Midi midi;

    midi.division =
        static_cast<std::uint16_t>(
            file.getTicksPerQuarterNote()
        );

    midi.tracks.reserve(file.getTrackCount());

    midi.bytes.assign(
        midiPayload.begin(),
        midiPayload.end()
    );


    // Tracks

    for (
        int trackIndex = 0;
        trackIndex < file.getTrackCount();
        ++trackIndex
    ) {
        MidiTrack track;

        track.events.reserve(
            file.getEventCount(trackIndex)
        );

        for (
            int eventIndex = 0;
            eventIndex < file.getEventCount(trackIndex);
            ++eventIndex
        ) {
            const smf::MidiEvent& source =
                file.getEvent(
                    trackIndex,
                    eventIndex
                );

            if (source.tick < 0) {
                throw std::runtime_error(
                    "Invalid MIDI event tick"
                );
            }

            MidiEvent event;

            event.tick =
                static_cast<std::uint32_t>(
                    source.tick
                );

            event.message.assign(
                source.begin(),
                source.end()
            );

            midi.totalTicks =
                std::max(
                    midi.totalTicks,
                    event.tick
                );

            track.events.push_back(
                std::move(event)
            );
        }

        midi.tracks.push_back(
            std::move(track)
        );
    }

    return midi;
}

}
