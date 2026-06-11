#pragma once

#include "KeyboardState.h"
#include "MouseState.h"

#include <SDL3/SDL.h>

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class InputManager {
public:
    InputManager() = default;
    ~InputManager() = default;

    // Deleted copy/move
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    InputManager(InputManager&&) = delete;
    InputManager& operator=(InputManager&&) = delete;

    // Process all pending SDL events, updating keyboard and mouse state
    void processEvents(SDL_Event* event);

    // Call at start of each frame to clear per-frame state (pressed/released/delta)
    void beginFrame();

    // Access keyboard state
    const KeyboardState& keyboard() const { return keyboard_; }
    KeyboardState& keyboard() { return keyboard_; }

    // Access mouse state
    const MouseState& mouse() const { return mouse_; }
    MouseState& mouse() { return mouse_; }

private:
    KeyboardState keyboard_;
    MouseState mouse_;
};

} // namespace eldoria::apps::elclient