#pragma once

#include "Screen.h"

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class InputManager;

// Minimal verification screen - no product meaning, no app flow
class PlaceholderScreen : public Screen {
public:
    PlaceholderScreen() = default;
    ~PlaceholderScreen() override = default;

    PlaceholderScreen(const PlaceholderScreen&) = delete;
    PlaceholderScreen& operator=(const PlaceholderScreen&) = delete;
    PlaceholderScreen(PlaceholderScreen&&) = delete;
    PlaceholderScreen& operator=(PlaceholderScreen&&) = delete;

    void onEnter(ScreenManager& manager, eld::platform::SdlContext& context) override;
    void onExit(ScreenManager& manager, eld::platform::SdlContext& context) override;
    void update(ScreenManager& manager, eld::platform::SdlContext& context, InputManager& input) override;
    void render(ScreenManager& manager, eld::platform::SdlContext& context) override;
    ScreenId id() const override { return ScreenId::Placeholder; }
};

} // namespace eldoria::apps::elclient