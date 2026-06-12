#pragma once

#include "Widget.h"

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class InputManager;
class UIContext;

// Simple text label widget
class LabelWidget : public Widget {
public:
    LabelWidget(int x, int y, int width, int height, const std::string& text);
    ~LabelWidget() override = default;

    LabelWidget(const LabelWidget&) = delete;
    LabelWidget& operator=(const LabelWidget&) = delete;
    LabelWidget(LabelWidget&&) = delete;
    LabelWidget& operator=(LabelWidget&&) = delete;

    void update(UIContext& uiContext, InputManager& input) override;
    void render(UIContext& uiContext, eld::render::SoftwareRenderBackend& backend) override;
    bool contains(int x, int y) const override;

    void setText(const std::string& text) { text_ = text; }
    const std::string& getText() const { return text_; }

private:
    std::string text_;
};

} // namespace eldoria::apps::elclient