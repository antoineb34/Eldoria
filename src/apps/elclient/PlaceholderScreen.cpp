#include "PlaceholderScreen.h"
#include "PlaceholderScreen.h"
#include "ScreenManager.h"
#include "InputManager.h"
#include "ClientRenderContext.h"
#include "UIContext.h"
#include "UIManager.h"
#include "PlaceholderWidget.h"
#include "Widget.h"

#include "../../platform/sdl/SdlContext.h"

#include <iostream>
#include <memory>

namespace eldoria::apps::elclient {

void PlaceholderScreen::onEnter(ScreenManager& manager, eld::platform::SdlContext& context, UIManager& uiManager) {
    (void)manager;
    (void)context;
    std::cout << "PlaceholderScreen: onEnter\n";

    // Add a placeholder widget to the UI context for verification
    auto& uiContext = uiManager.context();
    uiContext.addWidget(std::make_unique<PlaceholderWidget>());
}

void PlaceholderScreen::onExit(ScreenManager& manager, eld::platform::SdlContext& context, UIManager& uiManager) {
    (void)manager;
    (void)context;
    std::cout << "PlaceholderScreen: onExit\n";

    // Clear UI widgets when exiting
    auto& uiContext = uiManager.context();
    uiContext.clear();
}

void PlaceholderScreen::update(ScreenManager& manager, eld::platform::SdlContext& context, InputManager& input) {
    (void)manager;
    (void)context;
    (void)input;
    // No-op - no automatic transitions, no app flow
    // Input available via input.keyboard() and input.mouse()
}

void PlaceholderScreen::render(ScreenManager& manager, ClientRenderContext& renderContext, UIContext& uiContext) {
    (void)manager;
    (void)renderContext;
    (void)uiContext;
    // No-op - client render context handles clear/present
    // Screens can populate renderContext.scene() for actual rendering
    // UI is rendered separately by UIManager
}

} // namespace eldoria::apps::elclient