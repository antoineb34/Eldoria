#include "LoadingScreen.h"

#include "ScreenManager.h"

#include "../../platform/sdl/SdlContext.h"

#include <iostream>

namespace eldoria::apps::elclient {

void LoadingScreen::onEnter(ScreenManager& manager, eld::platform::SdlContext& context) {
    (void)manager;
    (void)context;
    std::cout << "LoadingScreen: onEnter\n";
}

void LoadingScreen::onExit(ScreenManager& manager, eld::platform::SdlContext& context) {
    (void)manager;
    (void)context;
    std::cout << "LoadingScreen: onExit\n";
}

void LoadingScreen::update(ScreenManager& manager, eld::platform::SdlContext& context) {
    (void)context;

    // Simulate loading completion
    static int frameCount = 0;
    frameCount++;

    if (frameCount >= 5) {
        std::cout << "LoadingScreen: loading complete, requesting transition to Login\n";
        manager.requestTransition(ScreenId::Login);
    }
}

void LoadingScreen::render(ScreenManager& manager, eld::platform::SdlContext& context) {
    (void)manager;
    (void)context;
}

} // namespace eldoria::apps::elclient