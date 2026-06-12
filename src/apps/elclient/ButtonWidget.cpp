#include "ButtonWidget.h"
#include "InputManager.h"
#include "UIContext.h"

namespace eldoria::apps::elclient {

ButtonWidget::ButtonWidget(int x, int y, int width, int height, const std::string& label, Callback callback)
    : label_(label), callback_(std::move(callback)) {
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
}

void ButtonWidget::update(UIContext& uiContext, InputManager& input) {
    (void)uiContext;

    int mx = input.mouse().x();
    int my = input.mouse().y();
    bool inside = contains(mx, my);

    // Handle hover
    hovered_ = inside;

    // Handle press/release
    if (inside && input.mouse().isButtonPressed(1)) { // Left click pressed
        pressed_ = true;
    }

    if (pressed_ && input.mouse().isButtonReleased(1)) { // Left click released
        if (inside && callback_) {
            callback_();
        }
        pressed_ = false;
    }

    // Cancel press if mouse leaves while held
    if (pressed_ && !inside) {
        pressed_ = false;
    }
}

void ButtonWidget::render(UIContext& uiContext, eld::render::SoftwareRenderBackend& backend) {
    (void)uiContext;
    (void)backend;
    // Rendering is handled by the render backend - placeholder for now
}

bool ButtonWidget::contains(int x, int y) const {
    return x >= x_ && x < x_ + width_ && y >= y_ && y < y_ + height_;
}

} // namespace eldoria::apps::elclient