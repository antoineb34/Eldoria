#pragma once

#include "Widget.h"

namespace eld::render {
class SoftwareRenderBackend;
}

namespace eldoria::apps::elclient {

class InputManager;
class UIContext;

// Minimal placeholder widget - no product meaning, proves UI ownership works
class PlaceholderWidget : public Widget {
public:
    PlaceholderWidget() = default;
    ~PlaceholderWidget() override = default;

    PlaceholderWidget(const PlaceholderWidget&) = delete;
    PlaceholderWidget& operator=(const PlaceholderWidget&) = delete;
    PlaceholderWidget(PlaceholderWidget&&) = delete;
    PlaceholderWidget& operator=(PlaceholderWidget&&) = delete;

    void update(UIContext& uiContext, InputManager& input) override;
    void render(UIContext& uiContext, eld::render::SoftwareRenderBackend& backend) override;
    bool contains(int x, int y) const override;
};

} // namespace eldoria::apps::elclient