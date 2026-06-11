#pragma once

#include <cstdint>

namespace eldoria::apps::elclient {

enum class ScreenId : uint8_t {
    Invalid = 0,
    Boot,
    Loading,
    Login,
    Game,
    Disconnected,
    Error,
    Count
};

constexpr ScreenId ScreenId_Invalid = ScreenId::Invalid;
constexpr ScreenId ScreenId_Boot = ScreenId::Boot;
constexpr ScreenId ScreenId_Loading = ScreenId::Loading;
constexpr ScreenId ScreenId_Login = ScreenId::Login;
constexpr ScreenId ScreenId_Game = ScreenId::Game;
constexpr ScreenId ScreenId_Disconnected = ScreenId::Disconnected;
constexpr ScreenId ScreenId_Error = ScreenId::Error;

inline const char* screenIdToString(ScreenId id) {
    switch (id) {
        case ScreenId::Invalid: return "Invalid";
        case ScreenId::Boot: return "Boot";
        case ScreenId::Loading: return "Loading";
        case ScreenId::Login: return "Login";
        case ScreenId::Game: return "Game";
        case ScreenId::Disconnected: return "Disconnected";
        case ScreenId::Error: return "Error";
        default: return "Unknown";
    }
}

} // namespace eldoria::apps::elclient