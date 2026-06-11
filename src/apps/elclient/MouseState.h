#pragma once

#include <cstdint>

namespace eldoria::apps::elclient {

// Maximum mouse buttons we track (typically 5: left, right, middle, X1, X2)
constexpr uint8_t MaxMouseButtons = 5;

class MouseState {
public:
    MouseState() = default;

    // Get current mouse position
    int32_t x() const { return x_; }
    int32_t y() const { return y_; }

    // Get mouse position delta since last frame
    int32_t deltaX() const { return deltaX_; }
    int32_t deltaY() const { return deltaY_; }

    // Check if a mouse button is currently held down
    bool isButtonDown(uint8_t button) const {
        return button < MaxMouseButtons && current_[button];
    }

    // Check if a mouse button was pressed this frame
    bool isButtonPressed(uint8_t button) const {
        return button < MaxMouseButtons && pressed_[button];
    }

    // Check if a mouse button was released this frame
    bool isButtonReleased(uint8_t button) const {
        return button < MaxMouseButtons && released_[button];
    }

    // Check if any button is currently held
    bool anyButtonDown() const {
        for (bool down : current_) {
            if (down) return true;
        }
        return false;
    }

    // Update state for a new frame - clears pressed/released, resets delta
    void beginFrame() {
        pressed_.fill(false);
        released_.fill(false);
        deltaX_ = 0;
        deltaY_ = 0;
    }

    // Process mouse motion event
    void onMouseMove(int32_t x, int32_t y) {
        deltaX_ = x - x_;
        deltaY_ = y - y_;
        x_ = x;
        y_ = y;
    }

    // Process mouse button down event
    void onButtonDown(uint8_t button) {
        if (button >= MaxMouseButtons) return;
        if (!current_[button]) {
            pressed_[button] = true;
        }
        current_[button] = true;
    }

    // Process mouse button up event
    void onButtonUp(uint8_t button) {
        if (button >= MaxMouseButtons) return;
        if (current_[button]) {
            released_[button] = true;
        }
        current_[button] = false;
    }

    // Process mouse wheel event
    void onWheel(int32_t x, int32_t y) {
        wheelX_ = x;
        wheelY_ = y;
    }

    // Get wheel delta (cleared each frame)
    int32_t wheelX() const { return wheelX_; }
    int32_t wheelY() const { return wheelY_; }

private:
    int32_t x_ = 0;
    int32_t y_ = 0;
    int32_t deltaX_ = 0;
    int32_t deltaY_ = 0;
    int32_t wheelX_ = 0;
    int32_t wheelY_ = 0;

    std::array<bool, MaxMouseButtons> current_{};
    std::array<bool, MaxMouseButtons> pressed_{};
    std::array<bool, MaxMouseButtons> released_{};
};

} // namespace eldoria::apps::elclient