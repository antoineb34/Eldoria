#pragma once

#include <cstdint>

namespace eldoria::apps::elclient {

enum class ScreenId : uint8_t {
    Invalid = 0,
    Placeholder,
    Login,
    Count
};

inline const char* screenIdToString(ScreenId id) {
    switch (id) {
        case ScreenId::Invalid: return "Invalid";
        case ScreenId::Placeholder: return "Placeholder";
        case ScreenId::Login: return "Login";
        default: return "Unknown";
    }
}

} // namespace eldoria::apps::elclient