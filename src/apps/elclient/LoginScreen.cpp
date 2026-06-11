#include "LoginScreen.h"

#include "ScreenManager.h"

#include "../../platform/sdl/SdlContext.h"

#include <iostream>

namespace eldoria::apps::elclient {

void LoginScreen::onEnter(ScreenManager& manager, eld::platform::SdlContext& context) {
    (void)manager;
    (void)context;
    std::cout << "LoginScreen: onEnter\n";
}

void LoginScreen::onExit(ScreenManager& manager, eld::platform::SdlContext& context) {
    (void)manager;
    (void)context;
    std::cout << "LoginScreen: onExit\n";
}

void LoginScreen::update(ScreenManager& manager, eld::platform::SdlContext& context) {
    (void)manager;
    (void)context;

    // In a real implementation, this would handle login form input
    // For now, just stay on login screen
}

void LoginScreen::render(ScreenManager& manager, eld::platform::SdlContext& context) {
    (void)manager;
    (void)context;
}

} // namespace eldoria::apps::elclient