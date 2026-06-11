#include "ElClientApp.h"

#include "PlaceholderScreen.h"

#include "../../platform/sdl/SdlContext.h"

#include <iostream>

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

    // Create and initialize client render context
    renderContext_ = std::make_unique<ClientRenderContext>(*sdlContext_);
    if (!renderContext_->initialize(WINDOW_WIDTH, WINDOW_HEIGHT)) {
        std::cerr << "ElClient: failed to initialize render context\n";
        return false;
    }

    // Register placeholder screen for verification
    screenManager_.registerScreen(std::make_unique<PlaceholderScreen>());

    // Activate placeholder screen
    screenManager_.requestTransition(ScreenId::Placeholder);

    state_.screen = ClientScreen::Startup;
    initialized_ = true;
    running_ = true;

    std::cout << "ElClient: initialized\n";
    return true;
}

void ElClientApp::update() {
    if (!initialized_ || !running_) {
        return;
    }

    // Begin new input frame
    inputManager_.beginFrame();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Process input events through InputManager
        inputManager_.processEvents(&event);

        if (event.type == SDL_EVENT_QUIT) {
            running_ = false;
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
            running_ = false;
        }
    }

    // Update screen manager (handles transitions and active screen update)
    screenManager_.update(*sdlContext_, inputManager_);
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

    // Render active screen through client render context
    screenManager_.render(*renderContext_);

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

const ClientState& ElClientApp::state() const {
    return state_;
}

ScreenManager& ElClientApp::screenManager() {
    return screenManager_;
}

InputManager& ElClientApp::inputManager() {
    return inputManager_;
}

ClientRenderContext& ElClientApp::renderContext() {
    return *renderContext_;
}

} // namespace eldoria::apps::elclient