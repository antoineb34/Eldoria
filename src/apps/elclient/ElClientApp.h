#pragma once

#include <memory>

#include <SDL3/SDL.h>

#include "ClientState.h"
#include "ScreenManager.h"
#include "InputManager.h"
#include "ClientRenderContext.h"

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class ElClientApp {
public:
    ElClientApp();
    ~ElClientApp();

    // Deleted copy/move - app owns unique resources
    ElClientApp(const ElClientApp&) = delete;
    ElClientApp& operator=(const ElClientApp&) = delete;
    ElClientApp(ElClientApp&&) = delete;
    ElClientApp& operator=(ElClientApp&&) = delete;

    // Lifecycle
    bool initialize();
    void update();
    void render();
    void shutdown();

    // Run loop
    int run();

    // State access
    bool isRunning() const;
    const ClientState& state() const;

    // Screen management
    ScreenManager& screenManager();

    // Input management
    InputManager& inputManager();

    // Render management
    ClientRenderContext& renderContext();

private:
    std::unique_ptr<eld::platform::SdlContext> sdlContext_;
    ClientState state_;
    ScreenManager screenManager_;
    InputManager inputManager_;
    std::unique_ptr<ClientRenderContext> renderContext_;
    bool initialized_ = false;
    bool running_ = false;
};

} // namespace eldoria::apps::elclient