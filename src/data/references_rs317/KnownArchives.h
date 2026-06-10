#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace eld::cache {

struct KnownArchiveName {
    uint32_t hash = 0;
    std::string_view name;
    std::string_view description;
};

inline const std::vector<KnownArchiveName> KNOWN_ARCHIVES = {
    {0x62a3f043, "p11_full.dat", "plain 11pt font"},
    {0xf2748da0, "p12_full.dat", "plain 12pt font"},
    {0xbcfe5ada, "b12_full.dat", "bold 12pt font"},
    {0x0c29bdfe, "q8_full.dat", "quill 8pt font"},
    {0x9788a968, "logo.dat", "RS logo sprite"},
    {0xde3bdc91, "title.dat", "login background JPEG"},
    {0x8f41ded6, "titlebox.dat", "login box sprite"},
    {0x74916959, "titlebutton.dat", "login buttons sprite"},
    {0x9c888208, "runes.dat", "rune icon sprites"},
    {0x8d00a607, "index.dat", "shared index"},

    {0xa276f8ac, "flo.dat", "floor/tile definitions"},
    {0xa2774214, "flo.idx", "floor/tile index"},
    {0x08fd540b, "idk.dat", "identity kit definitions"},
    {0x08fd9d73, "idk.idx", "identity kit index"},
    {0x28b56bdd, "loc.dat", "object/location definitions"},
    {0x28b5b545, "loc.idx", "object/location index"},
    {0x58c1fcdc, "npc.dat", "NPC definitions"},
    {0x58c24644, "npc.idx", "NPC index"},
    {0x9c9a2c36, "obj.dat", "item definitions"},
    {0x9c9a759e, "obj.idx", "item index"},
    {0x34d1b7b8, "seq.dat", "animation sequence definitions"},
    {0x34d20120, "seq.idx", "animation sequence index"},
    {0xc7114176, "spotanim.dat", "spot animation definitions"},
    {0xc7118ade, "spotanim.idx", "spot animation index"},
    {0x16df653c, "varp.dat", "variable player definitions"},
    {0x16dfaea4, "varp.idx", "variable player index"},
    {0xe14fb6af, "varbit.dat", "variable bit definitions"},
    {0xe1500017, "varbit.idx", "variable bit index"},

    {0x0ae38f79, "mesanim.dat", "message animation data"},
    {0x0ae3d8e1, "mesanim.idx", "message animation index"},
    {0x3d591c44, "mes.dat", "message definitions"},
    {0x3d5965ac, "mes.idx", "message definition index"},
    {0x93a322ec, "param.dat", "parameter definitions"},
    {0x93a36c54, "param.idx", "parameter definition index"},
};

inline std::string_view findKnownArchiveName(
    uint32_t hash
) {
    for (const KnownArchiveName& entry : KNOWN_ARCHIVES) {
        if (entry.hash == hash) {
            return entry.name;
        }
    }

    return {};
}

inline std::string_view findKnownArchiveDescription(
    uint32_t hash
) {
    for (const KnownArchiveName& entry : KNOWN_ARCHIVES) {
        if (entry.hash == hash) {
            return entry.description;
        }
    }

    return {};
}

}
