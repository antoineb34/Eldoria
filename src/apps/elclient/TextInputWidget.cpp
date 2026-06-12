#include "TextInputWidget.h"
#include "InputManager.h"
#include "UIContext.h"

#include <iostream>

namespace eldoria::apps::elclient {

TextInputWidget::TextInputWidget(int x, int y, int width, int height, std::string& textRef, bool isPassword)
    : textRef_(textRef), isPassword_(isPassword) {
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
}

void TextInputWidget::update(UIContext& uiContext, InputManager& input) {
    (void)uiContext;

    // Handle focus via mouse click
    if (input.mouse().isButtonPressed(1)) { // Left click
        int mx = input.mouse().x();
        int my = input.mouse().y();
        focused_ = contains(mx, my);
    }

    // Handle keyboard input when focused
    if (focused_) {
        // Check for key presses
        for (uint32_t scancode = 4; scancode <= 255; ++scancode) {
            if (input.keyboard().isKeyPressed(scancode)) {
                // Handle backspace
                if (scancode == 42) { // SDL_SCANCODE_BACKSPACE
                    if (!textRef_.empty() && cursorPos_ > 0) {
                        textRef_.erase(cursorPos_ - 1, 1);
                        cursorPos_--;
                    }
                }
                // Handle delete
                else if (scancode == 76) { // SDL_SCANCODE_DELETE
                    if (cursorPos_ < textRef_.size()) {
                        textRef_.erase(cursorPos_, 1);
                    }
                }
                // Handle left arrow
                else if (scancode == 80) { // SDL_SCANCODE_LEFT
                    if (cursorPos_ > 0) cursorPos_--;
                }
                // Handle right arrow
                else if (scancode == 79) { // SDL_SCANCODE_RIGHT
                    if (cursorPos_ < textRef_.size()) cursorPos_++;
                }
                // Handle home
                else if (scancode == 74) { // SDL_SCANCODE_HOME
                    cursorPos_ = 0;
                }
                // Handle end
                else if (scancode == 77) { // SDL_SCANCODE_END
                    cursorPos_ = textRef_.size();
                }
                // Handle printable characters (A-Z, 0-9, etc.)
                else {
                    // Simple character mapping for common keys
                    char c = 0;
                    bool shift = input.keyboard().isKeyDown(225) || input.keyboard().isKeyDown(229); // LSHIFT/RSHIFT

                    // Letters A-Z (scancodes 4-29)
                    if (scancode >= 4 && scancode <= 29) {
                        c = 'a' + (scancode - 4);
                        if (shift) c = 'A' + (scancode - 4);
                    }
                    // Numbers 1-0 (scancodes 30-39)
                    else if (scancode >= 30 && scancode <= 39) {
                        if (shift) {
                            const char* shifted = "!@#$%^&*()";
                            c = shifted[scancode - 30];
                        } else {
                            c = (scancode == 39) ? '0' : '1' + (scancode - 30);
                        }
                    }
                    // Space (scancode 44)
                    else if (scancode == 44) {
                        c = ' ';
                    }
                    // Period (scancode 55)
                    else if (scancode == 55) {
                        c = shift ? '>' : '.';
                    }
                    // Comma (scancode 54)
                    else if (scancode == 54) {
                        c = shift ? '<' : ',';
                    }
                    // Minus/Underscore (scancode 45)
                    else if (scancode == 45) {
                        c = shift ? '_' : '-';
                    }
                    // Equals/Plus (scancode 46)
                    else if (scancode == 46) {
                        c = shift ? '+' : '=';
                    }
                    // Slash/Question (scancode 56)
                    else if (scancode == 56) {
                        c = shift ? '?' : '/';
                    }

                    if (c != 0 && textRef_.size() < 64) { // Max length limit
                        textRef_.insert(cursorPos_, 1, c);
                        cursorPos_++;
                    }
                }
            }
        }
    }
}

void TextInputWidget::render(UIContext& uiContext, eld::render::SoftwareRenderBackend& backend) {
    (void)uiContext;
    (void)backend;
    // Rendering is handled by the render backend - placeholder for now
    // In a real implementation, this would draw the text box and text
}

bool TextInputWidget::contains(int x, int y) const {
    return x >= x_ && x < x_ + width_ && y >= y_ && y < y_ + height_;
}

} // namespace eldoria::apps::elclient