#include "UIContext.h"

#include "InputManager.h"

namespace eldoria::apps::elclient {

void UIContext::addWidget(WidgetPtr widget) {
    widgets_.push_back(std::move(widget));
}

void UIContext::update(InputManager& input) {
    for (auto& widget : widgets_) {
        if (widget) {
            // We need UIManager for widget update - this is a simple approach
            // In a more complete system, widgets would have direct access to what they need
            widget->update(*this, input);
        }
    }
}

void UIContext::render(eld::render::SoftwareRenderBackend& backend) {
    for (auto& widget : widgets_) {
        if (widget) {
            widget->render(*this, backend);
        }
    }
}

void UIContext::clear() {
    widgets_.clear();
}

} // namespace eldoria::apps::elclient