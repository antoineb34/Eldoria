#pragma once

#include <array>
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
    // SDL buttons are 1-based (1=left, 2=middle, 3=right, 4=X1, 5=X2)
    bool isButtonDown(uint8_t button) const {
        if (button == 0 || button > MaxMouseButtons) return false;
        return current_[button - 1];
    }

    // Check if a mouse button was pressed this frame
    bool isButtonPressed(uint8_t button) const {
        if (button == 0 || button > MaxMouseButtons) return false;
        return pressed_[button - 1];
    }

    // Check if a mouse button was released this frame
    bool isButtonReleased(uint8_t button) const {
        if (button == 0 || button > MaxMouseButtons) return false;
        return released_[button - 1];
    }

    // Check if any button is currently held
    bool anyButtonDown() const {
        for (bool down : current_) {
            if (down) return true;
        }
        return false;
    }

    // Update state for a new frame - clears pressed/released, resets delta and wheel
    void beginFrame() {
        pressed_.fill(false);
        released_.fill(false);
        deltaX_ = 0;
        deltaY_ = 0;
        wheelX_ = 0;
        wheelY_ = 0;
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
        if (button == 0 || button > MaxMouseButtons) return;
        const auto index = button - 1;
        if (!current_[index]) {
            pressed_[index] = true;
        }
        current_[index] = true;
    }

    // Process mouse button up event
    void onButtonUp(uint8_t button) {
        if (button == 0 || button > MaxMouseButtons) return;
        const auto index = button - 1;
        if (current_[index]) {
            released_[index] = true;
        }
        current_[index] = false;
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