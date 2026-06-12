#include "LabelWidget.h"
#include "InputManager.h"
#include "UIContext.h"

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
    (void)backend;
    // Rendering is handled by the render backend - placeholder for now
}

bool LabelWidget::contains(int x, int y) const {
    return false; // Labels don't handle input
}

} // namespace eldoria::apps::elclient