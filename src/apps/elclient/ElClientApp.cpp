#include "ElClientApp.h"
#include "ClientRenderContext.h"

#include "../../platform/sdl/SdlContext.h"

#include <iostream>
#include <filesystem>

namespace eldoria::apps::elclient {

ElClientApp::ElClientApp() = default;

ElClientApp::~ElClientApp() {
    shutdown();
}

bool ElClientApp::initialize() {
    if (initialized_) {
        return true;
    }

    constexpr int WINDOW_WIDTH = 800;
    constexpr int WINDOW_HEIGHT = 600;

    sdlContext_ = std::make_unique<eld::platform::SdlContext>(
        "ElClient",
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    if (!sdlContext_->window() || !sdlContext_->renderer()) {
        std::cerr << "ElClient: failed to initialize SDL context\n";
        sdlContext_.reset();
        return false;
    }

    // Initialize cache with default path (./cache)
    cache_ = eld::cache::Cache("cache");
    std::cout << "ElClient: cache path = " << std::filesystem::absolute("cache") << "\n";

    auto cacheRoot = std::filesystem::path("cache");
    auto datPath = cacheRoot / "main_file_cache.dat";
    std::vector<std::string> missingFiles;

    if (!std::filesystem::exists(datPath)) {
        missingFiles.push_back("main_file_cache.dat");
    }

    for (int i = 0; i <= 4; ++i) {
        auto idxPath = cacheRoot / ("main_file_cache.idx" + std::to_string(i));
        if (!std::filesystem::exists(idxPath)) {
            missingFiles.push_back("main_file_cache.idx" + std::to_string(i));
        }
    }

    if (missingFiles.empty() && cache_.isValid()) {
        std::cout << "ElClient: cache validation passed\n";
    } else {
        std::cerr << "ElClient: cache validation failed\n";
        if (missingFiles.empty()) {
            std::cerr << "ElClient: cache directory exists but is invalid\n";
        } else {
            for (const auto& file : missingFiles) {
                std::cerr << "ElClient: missing: " << file << "\n";
            }
        }
    }

    // Create and initialize client render context
    renderContext_ = std::make_unique<ClientRenderContext>(*sdlContext_);
    if (!renderContext_->initialize(WINDOW_WIDTH, WINDOW_HEIGHT)) {
        std::cerr << "ElClient: failed to initialize render context\n";
        return false;
    }

    initialized_ = true;
    running_ = true;

    std::cout << "ElClient: initialized\n";
    return true;
}

void ElClientApp::update() {
    if (!initialized_ || !running_) {
        return;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running_ = false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
            running_ = false;
        }
    }
}

void ElClientApp::render() {
    if (!initialized_ || !running_) {
        return;
    }

    if (!renderContext_ || !renderContext_->isInitialized()) {
        return;
    }

    // Begin frame (clears framebuffer, sets up camera)
    renderContext_->beginFrame();

    // End frame (renders scene through pipeline, presents to SDL)
    renderContext_->endFrame();
}

void ElClientApp::shutdown() {
    if (!initialized_) {
        return;
    }

    running_ = false;
    renderContext_.reset();
    sdlContext_.reset();
    initialized_ = false;

    std::cout << "ElClient: shutdown complete\n";
}

int ElClientApp::run() {
    if (!initialize()) {
        std::cerr << "ElClient: initialization failed\n";
        shutdown();
        return 1;
    }

    std::cout << "ElClient: starting main loop\n";

    while (running_) {
        update();
        render();
        SDL_Delay(16); // ~60 FPS cap
    }

    shutdown();
    std::cout << "ElClient: exited cleanly\n";
    return 0;
}

bool ElClientApp::isRunning() const {
    return running_;
}

ClientRenderContext& ElClientApp::renderContext() {
    return *renderContext_;
}

} // namespace eldoria::apps::elclient
