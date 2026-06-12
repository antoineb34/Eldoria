#pragma once

#include "Widget.h"

namespace eld::platform {
class SdlContext;
}

namespace eldoria::apps::elclient {

class InputManager;
class UIContext;

// Text input widget for login fields
class TextInputWidget : public Widget {
public:
    TextInputWidget(int x, int y, int width, int height, std::string& textRef, bool isPassword = false);
    ~TextInputWidget() override = default;

    TextInputWidget(const TextInputWidget&) = delete;
    TextInputWidget& operator=(const TextInputWidget&) = delete;
    TextInputWidget(TextInputWidget&&) = delete;
    TextInputWidget& operator=(TextInputWidget&&) = delete;

    void update(UIContext& uiContext, InputManager& input) override;
    void render(UIContext& uiContext, eld::render::SoftwareRenderBackend& backend) override;
    bool contains(int x, int y) const override;

    // Get the current text
    const std::string& getText() const { return textRef_; }

private:
    std::string& textRef_;
    bool isPassword_;
    bool focused_ = false;
    size_t cursorPos_ = 0;
};

} // namespace eldoria::apps::elclient