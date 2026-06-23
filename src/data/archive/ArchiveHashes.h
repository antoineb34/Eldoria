#pragma once

#include <cstdint>
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

namespace hashes {

inline constexpr std::uint32_t IndexDat =
    hashName("index.dat");

}

}
