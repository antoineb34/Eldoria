#include "BootScreen.h"

#include "ScreenManager.h"

#include "../../platform/sdl/SdlContext.h"

#include <iostream>

namespace eldoria::apps::elclient {

void BootScreen::onEnter(ScreenManager& manager, eld::platform::SdlContext& context) {
    (void)manager;
    (void)context;
    std::cout << "BootScreen: onEnter\n";
}

void BootScreen::onExit(ScreenManager& manager, eld::platform::SdlContext& context) {
    (void)manager;
    (void)context;
    std::cout << "BootScreen: onExit\n";
}

void BootScreen::update(ScreenManager& manager, eld::platform::SdlContext& context) {
    (void)context;

    // Simulate boot completion - transition to loading after a moment
    // In reality, this would do actual initialization work
    static int frameCount = 0;
    frameCount++;

    if (frameCount >= 5) {
        std::cout << "BootScreen: boot complete, requesting transition to Loading\n";
        manager.requestTransition(ScreenId::Loading);
    }
}

void BootScreen::render(ScreenManager& manager, eld::platform::SdlContext& context) {
    (void)manager;
    (void)context;
    // Render is handled by the main render loop clearing the screen
    // Boot screen could render a logo/progress here
}

} // namespace eldoria::apps::elclient