#include "ButtonWidget.h"
#include "InputManager.h"
#include "UIContext.h"
#include "backend/software/SoftwareRenderBackend.h"

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

    // Background color based on state
    eld::render::ColorPixel bgColor;
    if (pressed_) {
        bgColor = eld::render::ColorPixel{60, 100, 180, 255};      // Darker blue when pressed
    } else if (hovered_) {
        bgColor = eld::render::ColorPixel{80, 140, 220, 255};      // Lighter blue when hovered
    } else {
        bgColor = eld::render::ColorPixel{100, 150, 220, 255};     // Normal blue
    }

    // Draw background
    backend.drawRect(x_, y_, width_, height_, bgColor);

    // Draw border
    eld::render::ColorPixel borderColor{40, 80, 160, 255};
    backend.drawRectOutline(x_, y_, width_, height_, borderColor, 2);

    // Draw label text representation (simple centered rectangle)
    // In a real implementation, this would render actual text
    eld::render::ColorPixel textColor{255, 255, 255, 255};
    int textWidth = static_cast<int>(label_.size()) * 8;
    int textHeight = 14;
    int textX = x_ + (width_ - textWidth) / 2;
    int textY = y_ + (height_ - textHeight) / 2;
    backend.drawRect(textX, textY, textWidth, textHeight, textColor);
}

bool ButtonWidget::contains(int x, int y) const {
    return x >= x_ && x < x_ + width_ && y >= y_ && y < y_ + height_;
}

} // namespace eldoria::apps::elclient