#pragma once

#include <memory>
#include <SDL3/SDL.h>

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class ClientRenderContext;

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

    // Render management
    ClientRenderContext& renderContext();

private:
    std::unique_ptr<eld::platform::SdlContext> sdlContext_;
    std::unique_ptr<ClientRenderContext> renderContext_;
    bool initialized_ = false;
    bool running_ = false;
};

} // namespace eldoria::apps::elclient
