#pragma once

#include <memory>
#include <vector>

namespace eld::render {
class SoftwareRenderBackend;
}

namespace eldoria::apps::elclient {

class InputManager;
class UIContext;

class Widget {
public:
    Widget() = default;
    virtual ~Widget() = default;

    // Deleted copy/move - widgets are managed by UIManager
    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
    Widget(Widget&&) = delete;
    Widget& operator=(Widget&&) = delete;

    // Update widget logic
    virtual void update(UIContext& uiContext, InputManager& input) = 0;

    // Render widget through the software render backend
    virtual void render(UIContext& uiContext, eld::render::SoftwareRenderBackend& backend) = 0;

    // Get widget bounds (for hit testing)
    virtual bool contains(int x, int y) const = 0;

protected:
    // Widget position and size
    int x_ = 0;
    int y_ = 0;
    int width_ = 0;
    int height_ = 0;
};

using WidgetPtr = std::unique_ptr<Widget>;

} // namespace eldoria::apps::elclient