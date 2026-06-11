#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace eldoria::apps::elclient {

// Maximum number of scancodes we track (SDL_SCANCODE_COUNT is typically 512)
constexpr size_t MaxScancodes = 512;

class KeyboardState {
public:
    KeyboardState() = default;

    // Check if a key is currently held down
    bool isKeyDown(uint32_t scancode) const {
        return scancode < MaxScancodes && current_[scancode];
    }

    // Check if a key was pressed this frame
    bool isKeyPressed(uint32_t scancode) const {
        return scancode < MaxScancodes && pressed_[scancode];
    }

    // Check if a key was released this frame
    bool isKeyReleased(uint32_t scancode) const {
        return scancode < MaxScancodes && released_[scancode];
    }

    // Check if any key is currently held
    bool anyKeyDown() const {
        for (bool down : current_) {
            if (down) return true;
        }
        return false;
    }

    // Update state for a new frame - clears pressed/released arrays
    void beginFrame() {
        pressed_.fill(false);
        released_.fill(false);
    }

    // Process a key down event
    void onKeyDown(uint32_t scancode) {
        if (scancode >= MaxScancodes) return;
        if (!current_[scancode]) {
            pressed_[scancode] = true;
        }
        current_[scancode] = true;
    }

    // Process a key up event
    void onKeyUp(uint32_t scancode) {
        if (scancode >= MaxScancodes) return;
        if (current_[scancode]) {
            released_[scancode] = true;
        }
        current_[scancode] = false;
    }

private:
    std::array<bool, MaxScancodes> current_{};
    std::array<bool, MaxScancodes> pressed_{};
    std::array<bool, MaxScancodes> released_{};
};

} // namespace eldoria::apps::elclient