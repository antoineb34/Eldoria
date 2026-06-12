#include "LabelWidget.h"
#include "InputManager.h"
#include "UIContext.h"
#include "backend/software/SoftwareRenderBackend.h"

namespace eldoria::apps::elclient {

LabelWidget::LabelWidget(int x, int y, int width, int height, const std::string& text)
    : text_(text) {
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
}

void LabelWidget::update(UIContext& uiContext, InputManager& input) {
    (void)uiContext;
    (void)input;
    // Labels don't handle input
}

void LabelWidget::render(UIContext& uiContext, eld::render::SoftwareRenderBackend& backend) {
    (void)uiContext;

    // Draw text representation (simple rectangle)
    // In a real implementation, this would render actual text
    eld::render::ColorPixel textColor{0, 0, 0, 255}; // Black text
    int textWidth = static_cast<int>(text_.size()) * 8;
    int textHeight = 14;
    backend.drawRect(x_, y_, textWidth, textHeight, textColor);
}

bool LabelWidget::contains(int x, int y) const {
    return false; // Labels don't handle input
}

} // namespace eldoria::apps::elclient