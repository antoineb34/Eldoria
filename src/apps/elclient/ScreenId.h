#pragma once

#include <cstdint>

namespace eldoria::apps::elclient {

enum class ScreenId : uint8_t {
    Invalid = 0,
    Placeholder,
    Count
};

inline const char* screenIdToString(ScreenId id) {
    switch (id) {
        case ScreenId::Invalid: return "Invalid";
        case ScreenId::Placeholder: return "Placeholder";
        default: return "Unknown";
    }
}

} // namespace eldoria::apps::elclient