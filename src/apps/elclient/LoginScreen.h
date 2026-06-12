#pragma once

#include "Screen.h"

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class InputManager;
class ClientRenderContext;
class UIContext;
class UIManager;

// Login screen skeleton - placeholder credentials, local state only
class LoginScreen : public Screen {
public:
    LoginScreen() = default;
    ~LoginScreen() override = default;

    LoginScreen(const LoginScreen&) = delete;
    LoginScreen& operator=(const LoginScreen&) = delete;
    LoginScreen(LoginScreen&&) = delete;
    LoginScreen& operator=(LoginScreen&&) = delete;

    void onEnter(ScreenManager& manager, eld::platform::SdlContext& context, UIManager& uiManager) override;
    void onExit(ScreenManager& manager, eld::platform::SdlContext& context, UIManager& uiManager) override;
    void update(ScreenManager& manager, eld::platform::SdlContext& context, InputManager& input) override;
    void render(ScreenManager& manager, ClientRenderContext& renderContext, UIContext& uiContext) override;
    ScreenId id() const override { return ScreenId::Login; }

private:
    // Store UIManager reference for accessing UI context during updates
    UIManager* uiManager_ = nullptr;

    // Local login state (no networking, no real auth)
    std::string username_;
    std::string password_;
    std::string statusText_;
    bool usernameFocused_ = true;
    bool loginRequested_ = false;
};

} // namespace eldoria::apps::elclient