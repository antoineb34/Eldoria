#pragma once

#include "ScreenId.h"

#include <memory>

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class ScreenManager;
class InputManager;
class ClientRenderContext;
class UIContext;
class UIManager;

class Screen {
public:
    virtual ~Screen() = default;

    // Called when this screen becomes active
    virtual void onEnter(ScreenManager& manager, eld::platform::SdlContext& context, UIManager& uiManager) = 0;

    // Called when this screen is no longer active
    virtual void onExit(ScreenManager& manager, eld::platform::SdlContext& context, UIManager& uiManager) = 0;

    // Update screen logic
    virtual void update(ScreenManager& manager, eld::platform::SdlContext& context, InputManager& input) = 0;

    // Render screen through client render context
    virtual void render(ScreenManager& manager, ClientRenderContext& renderContext, UIContext& uiContext) = 0;

    // Get the screen identity
    virtual ScreenId id() const = 0;

protected:
    Screen() = default;

    // Deleted copy/move - screens are managed by ScreenManager
    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;
    Screen(Screen&&) = delete;
    Screen& operator=(Screen&&) = delete;
};

using ScreenPtr = std::unique_ptr<Screen>;

} // namespace eldoria::apps::elclient