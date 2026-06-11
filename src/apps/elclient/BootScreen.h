#pragma once

#include "Screen.h"

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class BootScreen : public Screen {
public:
    BootScreen() = default;
    ~BootScreen() override = default;

    // Deleted copy/move
    BootScreen(const BootScreen&) = delete;
    BootScreen& operator=(const BootScreen&) = delete;
    BootScreen(BootScreen&&) = delete;
    BootScreen& operator=(BootScreen&&) = delete;

    void onEnter(ScreenManager& manager, eld::platform::SdlContext& context) override;
    void onExit(ScreenManager& manager, eld::platform::SdlContext& context) override;
    void update(ScreenManager& manager, eld::platform::SdlContext& context) override;
    void render(ScreenManager& manager, eld::platform::SdlContext& context) override;
    ScreenId id() const override { return ScreenId::Boot; }
};

} // namespace eldoria::apps::elclient