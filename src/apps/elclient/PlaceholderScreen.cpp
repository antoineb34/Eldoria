#include "PlaceholderScreen.h"

#include "ScreenManager.h"
#include "InputManager.h"
#include "ClientRenderContext.h"

#include "../../platform/sdl/SdlContext.h"

#include <iostream>

namespace eldoria::apps::elclient {

void PlaceholderScreen::onEnter(ScreenManager& manager, eld::platform::SdlContext& context) {
    (void)manager;
    (void)context;
    std::cout << "PlaceholderScreen: onEnter\n";
}

void PlaceholderScreen::onExit(ScreenManager& manager, eld::platform::SdlContext& context) {
    (void)manager;
    (void)context;
    std::cout << "PlaceholderScreen: onExit\n";
}

void PlaceholderScreen::update(ScreenManager& manager, eld::platform::SdlContext& context, InputManager& input) {
    (void)manager;
    (void)context;
    (void)input;
    // No-op - no automatic transitions, no app flow
    // Input available via input.keyboard() and input.mouse()
}

void PlaceholderScreen::render(ScreenManager& manager, ClientRenderContext& renderContext) {
    (void)manager;
    (void)renderContext;
    // No-op - client render context handles clear/present
    // Screens can populate renderContext.scene() for actual rendering
}

} // namespace eldoria::apps::elclient