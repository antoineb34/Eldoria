#pragma once

#include "Screen.h"

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class LoadingScreen : public Screen {
public:
    LoadingScreen() = default;
    ~LoadingScreen() override = default;

    // Deleted copy/move
    LoadingScreen(const LoadingScreen&) = delete;
    LoadingScreen& operator=(const LoadingScreen&) = delete;
    LoadingScreen(LoadingScreen&&) = delete;
    LoadingScreen& operator=(LoadingScreen&&) = delete;

    void onEnter(ScreenManager& manager, eld::platform::SdlContext& context) override;
    void onExit(ScreenManager& manager, eld::platform::SdlContext& context) override;
    void update(ScreenManager& manager, eld::platform::SdlContext& context) override;
    void render(ScreenManager& manager, eld::platform::SdlContext& context) override;
    ScreenId id() const override { return ScreenId::Loading; }
};

} // namespace eldoria::apps::elclient