#pragma once

#include "Widget.h"
#include <functional>

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class InputManager;
class UIContext;

// Button widget with callback
class ButtonWidget : public Widget {
public:
    using Callback = std::function<void()>;

    ButtonWidget(int x, int y, int width, int height, const std::string& label, Callback callback);
    ~ButtonWidget() override = default;

    ButtonWidget(const ButtonWidget&) = delete;
    ButtonWidget& operator=(const ButtonWidget&) = delete;
    ButtonWidget(ButtonWidget&&) = delete;
    ButtonWidget& operator=(ButtonWidget&&) = delete;

    void update(UIContext& uiContext, InputManager& input) override;
    void render(UIContext& uiContext, eld::render::SoftwareRenderBackend& backend) override;
    bool contains(int x, int y) const override;

private:
    std::string label_;
    Callback callback_;
    bool hovered_ = false;
    bool pressed_ = false;
};

} // namespace eldoria::apps::elclient