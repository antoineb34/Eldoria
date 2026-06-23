#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace eld::archive {

constexpr std::uint32_t hashName(
    std::string_view name
) {
    std::uint32_t hash = 0;

    for (char character : name) {
        if (
            character >= 'a' &&
            character <= 'z'
        ) {
            character =
                static_cast<char>(
                    character - 'a' + 'A'
                );
        }

        hash =
            hash * 61U +
            static_cast<std::uint8_t>(
                character
            ) -
            32U;
    }

    return hash;
}

struct FileName {
    std::string_view name;
    std::string_view description;
};

inline constexpr std::array FileNames{
    FileName{"index.dat", "shared index"},

    FileName{"anim_index", "animation index"},
    FileName{"anim_crc", "animation checksums"},
    FileName{"anim_version", "animation versions"},

    FileName{"map_index", "map index"},
    FileName{"map_crc", "map checksums"},
    FileName{"map_version", "map versions"},

    FileName{"midi_index", "MIDI index"},
    FileName{"midi_crc", "MIDI checksums"},
    FileName{"midi_version", "MIDI versions"},

    FileName{"model_index", "model index"},
    FileName{"model_crc", "model checksums"},
    FileName{"model_version", "model versions"},

    FileName{"p11_full.dat", "plain 11pt font"},
    FileName{"p12_full.dat", "plain 12pt font"},
    FileName{"b12_full.dat", "bold 12pt font"},
    FileName{"q8_full.dat", "quill 8pt font"},
    FileName{"logo.dat", "RS logo sprite"},
    FileName{"title.dat", "login background JPEG"},
    FileName{"titlebox.dat", "login box sprite"},
    FileName{"titlebutton.dat", "login button sprite"},
    FileName{"runes.dat", "rune icon sprites"},

    FileName{"flo.dat", "floor definitions"},
    FileName{"flo.idx", "floor index"},
    FileName{"idk.dat", "identity kit definitions"},
    FileName{"idk.idx", "identity kit index"},
    FileName{"loc.dat", "object definitions"},
    FileName{"loc.idx", "object index"},
    FileName{"npc.dat", "NPC definitions"},
    FileName{"npc.idx", "NPC index"},
    FileName{"obj.dat", "item definitions"},
    FileName{"obj.idx", "item index"},
    FileName{"seq.dat", "animation sequence definitions"},
    FileName{"seq.idx", "animation sequence index"},
    FileName{"spotanim.dat", "spot animation definitions"},
    FileName{"spotanim.idx", "spot animation index"},
    FileName{"varp.dat", "player variable definitions"},
    FileName{"varp.idx", "player variable index"},
    FileName{"varbit.dat", "variable bit definitions"},
    FileName{"varbit.idx", "variable bit index"},

    FileName{"mesanim.dat", "message animation data"},
    FileName{"mesanim.idx", "message animation index"},
    FileName{"mes.dat", "message definitions"},
    FileName{"mes.idx", "message definition index"},
    FileName{"param.dat", "parameter definitions"},
    FileName{"param.idx", "parameter definition index"}
};

constexpr std::optional<std::string_view> findName(
    std::uint32_t hash
) {
    for (const FileName& entry : FileNames) {
        if (hashName(entry.name) == hash) {
            return entry.name;
        }
    }

    return std::nullopt;
}

constexpr std::optional<std::string_view> findDescription(
    std::uint32_t hash
) {
    for (const FileName& entry : FileNames) {
        if (hashName(entry.name) == hash) {
            return entry.description;
        }
    }

    return std::nullopt;
}

}
