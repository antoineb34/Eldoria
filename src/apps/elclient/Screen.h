#pragma once

#include "ScreenId.h"

#include <memory>

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class ScreenManager;
class InputManager;

class Screen {
public:
    virtual ~Screen() = default;

    // Called when this screen becomes active
    virtual void onEnter(ScreenManager& manager, eld::platform::SdlContext& context) = 0;

    // Called when this screen is no longer active
    virtual void onExit(ScreenManager& manager, eld::platform::SdlContext& context) = 0;

    // Update screen logic
    virtual void update(ScreenManager& manager, eld::platform::SdlContext& context, InputManager& input) = 0;

    // Render screen
    virtual void render(ScreenManager& manager, eld::platform::SdlContext& context) = 0;

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