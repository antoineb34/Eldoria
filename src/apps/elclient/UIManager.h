#pragma once

#include "UIContext.h"

namespace eld::platform {
class SdlContext;
}

namespace eld::render {
class SoftwareRenderBackend;
}

namespace eldoria::apps::elclient {

class InputManager;

class UIManager {
public:
    UIManager() = default;
    ~UIManager() = default;

    // Deleted copy/move
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;
    UIManager(UIManager&&) = delete;
    UIManager& operator=(UIManager&&) = delete;

    // Initialize UI manager
    bool initialize(eld::platform::SdlContext& context);

    // Begin new UI frame
    void beginFrame();

    // End UI frame
    void endFrame();

    // Get UI context for screens to use
    UIContext& context() { return context_; }
    const UIContext& context() const { return context_; }

    // Update UI (process input, update widgets)
    void update(InputManager& input);

    // Render UI
    void render(eld::render::SoftwareRenderBackend& backend);

    // Check if initialized
    bool isInitialized() const { return initialized_; }

private:
    UIContext context_;
    bool initialized_ = false;
};

} // namespace eldoria::apps::elclient