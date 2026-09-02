#include "MidiTimeline.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "midi/MidiFile.h"

namespace eld::audio {

namespace {

bool readVariableLengthQuantity(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& cursor,
    std::size_t end,
    std::uint32_t& value
) {
    value = 0;

    for (int count = 0; count < 4; ++count) {
        if (cursor >= end) {
            return false;
        }

        const std::uint8_t byte =
            bytes[cursor++];

        value =
            (value << 7) |
            static_cast<std::uint32_t>(
                byte & 0x7Fu
            );

        if ((byte & 0x80u) == 0) {
            return true;
        }
    }

    return false;
}

}

MidiTimeline readMidiTimeline(
    const eld::midi::MidiFile& midi
) {
    MidiTimeline timeline;

    const std::vector<std::uint8_t>& bytes =
        midi.data.bytes;

    for (const eld::midi::MidiTrackInfo& track :
         midi.data.tracks) {
        std::size_t cursor =
            track.dataOffset;

        const std::size_t end =
            std::min(
                track.dataOffset +
                    static_cast<std::size_t>(
                        track.dataSize
                    ),
                bytes.size()
            );

        std::uint32_t tick = 0;
        std::uint8_t runningStatus = 0;

        while (cursor < end) {
            std::uint32_t delta = 0;

            if (!readVariableLengthQuantity(
                    bytes,
                    cursor,
                    end,
                    delta
                )) {
                break;
            }

            tick += delta;

            timeline.totalTicks =
                std::max(
                    timeline.totalTicks,
                    tick
                );

            if (cursor >= end) {
                break;
            }

            std::uint8_t status =
                bytes[cursor];

            if ((status & 0x80u) != 0) {
                ++cursor;

                if (status < 0xF0u) {
                    runningStatus = status;
                }
            }
            else {
                if (runningStatus == 0) {
                    break;
                }

                status = runningStatus;
            }

            if (status == 0xFFu) {
                if (cursor >= end) {
                    break;
                }

                ++cursor;

                std::uint32_t length = 0;

                if (
                    !readVariableLengthQuantity(
                        bytes,
                        cursor,
                        end,
                        length
                    ) ||
                    length > end - cursor
                ) {
                    break;
                }

                cursor += length;
                continue;
            }

            if (
                status == 0xF0u ||
                status == 0xF7u
            ) {
                runningStatus = 0;

                std::uint32_t length = 0;

                if (
                    !readVariableLengthQuantity(
                        bytes,
                        cursor,
                        end,
                        length
                    ) ||
                    length > end - cursor
                ) {
                    break;
                }

                cursor += length;
                continue;
            }

            if (status >= 0xF0u) {
                std::size_t dataLength = 0;

                switch (status) {
                    case 0xF1u:
                    case 0xF3u:
                        dataLength = 1;
                        break;

                    case 0xF2u:
                        dataLength = 2;
                        break;

                    case 0xF6u:
                    case 0xF8u:
                    case 0xFAu:
                    case 0xFBu:
                    case 0xFCu:
                    case 0xFEu:
                        dataLength = 0;
                        break;

                    default:
                        cursor = end;
                        continue;
                }

                if (dataLength > end - cursor) {
                    break;
                }

                cursor += dataLength;
                continue;
            }

            const std::uint8_t command =
                status & 0xF0u;

            const std::uint8_t channel =
                status & 0x0Fu;

            const std::size_t dataLength =
                command == 0xC0u ||
                command == 0xD0u
                    ? 1
                    : 2;

            if (dataLength > end - cursor) {
                break;
            }

            const std::uint8_t first =
                bytes[cursor];

            const std::uint8_t second =
                dataLength == 2
                    ? bytes[cursor + 1]
                    : 0;

            cursor += dataLength;

            if (
                command == 0x90u &&
                second != 0
            ) {
                timeline.noteOnEvents.push_back({
                    .tick = tick,
                    .channel = channel,
                    .note = first,
                    .velocity = second
                });
            }
        }
    }

    return timeline;
}

}
