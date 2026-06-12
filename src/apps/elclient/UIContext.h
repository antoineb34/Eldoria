#pragma once

#include "Widget.h"

#include <memory>
#include <vector>

namespace eld::render {
class SoftwareRenderBackend;
}

namespace eldoria::apps::elclient {

class InputManager;

class UIContext {
public:
    UIContext() = default;
    ~UIContext() = default;

    // Deleted copy/move
    UIContext(const UIContext&) = delete;
    UIContext& operator=(const UIContext&) = delete;
    UIContext(UIContext&&) = delete;
    UIContext& operator=(UIContext&&) = delete;

    // Add a widget to the UI context (takes ownership)
    void addWidget(WidgetPtr widget);

    // Get all widgets
    const std::vector<WidgetPtr>& widgets() const { return widgets_; }
    std::vector<WidgetPtr>& widgets() { return widgets_; }

    // Update all widgets
    void update(InputManager& input);

    // Render all widgets
    void render(eld::render::SoftwareRenderBackend& backend);

    // Clear all widgets
    void clear();

private:
    std::vector<WidgetPtr> widgets_;
};

} // namespace eldoria::apps::elclient