#include "UIManager.h"

#include "InputManager.h"
#include "../../platform/sdl/SdlContext.h"

#include <iostream>

namespace eldoria::apps::elclient {

bool UIManager::initialize(eld::platform::SdlContext& context) {
    (void)context;
    if (initialized_) {
        return true;
    }

    initialized_ = true;
    std::cout << "UIManager: initialized\n";
    return true;
}

void UIManager::beginFrame() {
    if (!initialized_) {
        return;
    }
    // Clear per-frame UI state if needed
}

void UIManager::endFrame() {
    if (!initialized_) {
        return;
    }
    // End of frame UI cleanup if needed
}

void UIManager::update(InputManager& input) {
    if (!initialized_) {
        return;
    }

    context_.update(input);
}

void UIManager::render(eld::render::SoftwareRenderBackend& backend) {
    if (!initialized_) {
        return;
    }

    context_.render(backend);
}

} // namespace eldoria::apps::elclient