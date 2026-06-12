#include "PlaceholderWidget.h"
#include "UIContext.h"
#include "InputManager.h"

#include <iostream>

namespace eldoria::apps::elclient {

void PlaceholderWidget::update(UIContext& uiContext, InputManager& input) {
    (void)uiContext;
    (void)input;
    // No-op - placeholder widget
}

void PlaceholderWidget::render(UIContext& uiContext, eld::render::SoftwareRenderBackend& backend) {
    (void)uiContext;
    (void)backend;
    // No-op - placeholder widget doesn't render anything
    // In a real implementation, this would draw to the backend's framebuffer
}

bool PlaceholderWidget::contains(int x, int y) const {
    (void)x;
    (void)y;
    // Placeholder widget has no bounds
    return false;
}

} // namespace eldoria::apps::elclient