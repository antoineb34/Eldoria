#pragma once

#include "Screen.h"

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class LoginScreen : public Screen {
public:
    LoginScreen() = default;
    ~LoginScreen() override = default;

    // Deleted copy/move
    LoginScreen(const LoginScreen&) = delete;
    LoginScreen& operator=(const LoginScreen&) = delete;
    LoginScreen(LoginScreen&&) = delete;
    LoginScreen& operator=(LoginScreen&&) = delete;

    void onEnter(ScreenManager& manager, eld::platform::SdlContext& context) override;
    void onExit(ScreenManager& manager, eld::platform::SdlContext& context) override;
    void update(ScreenManager& manager, eld::platform::SdlContext& context) override;
    void render(ScreenManager& manager, eld::platform::SdlContext& context) override;
    ScreenId id() const override { return ScreenId::Login; }
};

} // namespace eldoria::apps::elclient