#pragma once

#include "Screen.h"
#include "ScreenId.h"

#include <array>
#include <memory>

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class InputManager;
class ClientRenderContext;

class Screen;

class ScreenManager {
public:
    ScreenManager() = default;
    ~ScreenManager() = default;

    // Deleted copy/move
    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;
    ScreenManager(ScreenManager&&) = delete;
    ScreenManager& operator=(ScreenManager&&) = delete;

    // Register a screen (takes ownership)
    void registerScreen(ScreenPtr screen);

    // Request a screen transition
    void requestTransition(ScreenId to);

    // Get the currently active screen
    Screen* activeScreen();
    const Screen* activeScreen() const;

    // Get the active screen ID
    ScreenId activeScreenId() const;

    // Check if a transition is pending
    bool hasPendingTransition() const;

    // Get the pending transition target
    ScreenId pendingTransition() const;

    // Process pending transition (call at start of frame)
    void processTransition(eld::platform::SdlContext& context);

    // Update the active screen
    void update(eld::platform::SdlContext& context, InputManager& input);

    // Render the active screen through client render context
    void render(ClientRenderContext& renderContext);

    // Check if a screen is registered
    bool hasScreen(ScreenId id) const;

private:
    static constexpr size_t MaxScreens = static_cast<size_t>(ScreenId::Count);

    std::array<ScreenPtr, MaxScreens> screens_;
    Screen* active_ = nullptr;
    ScreenId pending_ = ScreenId::Invalid;
    bool transitionPending_ = false;

    // Internal transition helper
    void performTransition(ScreenId to, eld::platform::SdlContext& context);
};

} // namespace eldoria::apps::elclient