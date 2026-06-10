#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace eld::cache::archive {

constexpr std::uint32_t hashName(
    std::string_view name
) {
    std::uint32_t hash = 0;

    for (char character : name) {
        hash =
            hash * 61 +
            static_cast<unsigned char>(
                character >= 'a' && character <= 'z'
                    ? character - 32
                    : character
            ) -
            32;
    }

    return hash;
}

constexpr std::string_view AnimIndexName = "anim_index";
constexpr std::string_view AnimCrcName = "anim_crc";
constexpr std::string_view AnimVersionName = "anim_version";

constexpr std::string_view MapIndexName = "map_index";
constexpr std::string_view MapCrcName = "map_crc";
constexpr std::string_view MapVersionName = "map_version";

constexpr std::string_view MidiIndexName = "midi_index";
constexpr std::string_view MidiCrcName = "midi_crc";
constexpr std::string_view MidiVersionName = "midi_version";

constexpr std::string_view ModelIndexName = "model_index";
constexpr std::string_view ModelCrcName = "model_crc";
constexpr std::string_view ModelVersionName = "model_version";

constexpr std::uint32_t AnimIndex = hashName(AnimIndexName);
constexpr std::uint32_t AnimCrc = hashName(AnimCrcName);
constexpr std::uint32_t AnimVersion = hashName(AnimVersionName);

constexpr std::uint32_t MapIndex = hashName(MapIndexName);
constexpr std::uint32_t MapCrc = hashName(MapCrcName);
constexpr std::uint32_t MapVersion = hashName(MapVersionName);

constexpr std::uint32_t MidiIndex = hashName(MidiIndexName);
constexpr std::uint32_t MidiCrc = hashName(MidiCrcName);
constexpr std::uint32_t MidiVersion = hashName(MidiVersionName);

constexpr std::uint32_t ModelIndex = hashName(ModelIndexName);
constexpr std::uint32_t ModelCrc = hashName(ModelCrcName);
constexpr std::uint32_t ModelVersion = hashName(ModelVersionName);

static_assert(AnimIndex == 715169772);
static_assert(AnimCrc == 4254738632u);
static_assert(AnimVersion == 3497468394u);

static_assert(MapCrc == 1915414053);
static_assert(MapIndex == 1987120305);
static_assert(MapVersion == 3371441495u);

static_assert(MidiIndex == 2603484342u);
static_assert(MidiCrc == 3173713090u);
static_assert(MidiVersion == 3349487108u);

static_assert(ModelVersion == 252137566);
static_assert(ModelCrc == 2533368572u);
static_assert(ModelIndex == 3588382144u);

inline std::optional<std::string_view> knownName(
    std::uint32_t hash
) {
    switch (hash) {
        case AnimIndex:
            return AnimIndexName;

        case AnimCrc:
            return AnimCrcName;

        case AnimVersion:
            return AnimVersionName;

        case MapIndex:
            return MapIndexName;

        case MapCrc:
            return MapCrcName;

        case MapVersion:
            return MapVersionName;

        case MidiIndex:
            return MidiIndexName;

        case MidiCrc:
            return MidiCrcName;

        case MidiVersion:
            return MidiVersionName;

        case ModelIndex:
            return ModelIndexName;

        case ModelCrc:
            return ModelCrcName;

        case ModelVersion:
            return ModelVersionName;

        default:
            return std::nullopt;
    }
}

}
