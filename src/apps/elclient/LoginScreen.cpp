#include "LoginScreen.h"
#include "ScreenManager.h"
#include "InputManager.h"
#include "ClientRenderContext.h"
#include "UIContext.h"
#include "UIManager.h"
#include "Widget.h"
#include "PlaceholderWidget.h"

#include "../../platform/sdl/SdlContext.h"

#include <iostream>
#include <memory>
#include <string>
#include <functional>

namespace eldoria::apps::elclient {

// Simple text input widget for login fields
class TextInputWidget : public Widget {
public:
    TextInputWidget(int x, int y, int width, int height, std::string& textRef, bool isPassword = false)
        : textRef_(textRef), isPassword_(isPassword) {
        x_ = x;
        y_ = y;
        width_ = width;
        height_ = height;
    }

    void update(UIContext& uiContext, InputManager& input) override {
        (void)uiContext;
        (void)input;
        // Text input handling would go here - for now placeholder
        // Real implementation would handle keyboard input
    }

    void render(UIContext& uiContext, eld::render::SoftwareRenderBackend& backend) override {
        (void)uiContext;
        (void)backend;
        // Rendering would go here - for now placeholder
        // Real implementation would draw text box and text
    }

    bool contains(int x, int y) const override {
        return x >= x_ && x < x_ + width_ && y >= y_ && y < y_ + height_;
    }

private:
    std::string& textRef_;
    bool isPassword_;
};

// Simple button widget
class ButtonWidget : public Widget {
public:
    using Callback = std::function<void()>;

    ButtonWidget(int x, int y, int width, int height, const std::string& label, Callback callback)
        : label_(label), callback_(std::move(callback)) {
        x_ = x;
        y_ = y;
        width_ = width;
        height_ = height;
    }

    void update(UIContext& uiContext, InputManager& input) override {
        (void)uiContext;
        // Check for mouse click on button
        if (input.mouse().isButtonPressed(1)) { // Left click
            int mx = input.mouse().x();
            int my = input.mouse().y();
            if (contains(mx, my) && callback_) {
                callback_();
            }
        }
    }

    void render(UIContext& uiContext, eld::render::SoftwareRenderBackend& backend) override {
        (void)uiContext;
        (void)backend;
        // Rendering would go here - for now placeholder
    }

    bool contains(int x, int y) const override {
        return x >= x_ && x < x_ + width_ && y >= y_ && y < y_ + height_;
    }

private:
    std::string label_;
    Callback callback_;
};

// Simple text label widget
class LabelWidget : public Widget {
public:
    LabelWidget(int x, int y, int width, int height, const std::string& text)
        : text_(text) {
        x_ = x;
        y_ = y;
        width_ = width;
        height_ = height;
    }

    void update(UIContext& uiContext, InputManager& input) override {
        (void)uiContext;
        (void)input;
    }

    void render(UIContext& uiContext, eld::render::SoftwareRenderBackend& backend) override {
        (void)uiContext;
        (void)backend;
        // Rendering would go here - for now placeholder
    }

    bool contains(int x, int y) const override {
        return false; // Labels don't handle input
    }

    void setText(const std::string& text) { text_ = text; }
    const std::string& getText() const { return text_; }

private:
    std::string text_;
};

void LoginScreen::onEnter(ScreenManager& manager, eld::platform::SdlContext& context, UIManager& uiManager) {
    (void)manager;
    (void)context;
    std::cout << "LoginScreen: onEnter\n";

    // Store UIManager reference for later use
    uiManager_ = &uiManager;

    auto& uiContext = uiManager.context();
    uiContext.clear();

    // Add username label
    uiContext.addWidget(std::make_unique<LabelWidget>(300, 200, 200, 30, "Username:"));

    // Add username input field
    uiContext.addWidget(std::make_unique<TextInputWidget>(300, 230, 200, 30, username_, false));

    // Add password label
    uiContext.addWidget(std::make_unique<LabelWidget>(300, 280, 200, 30, "Password:"));

    // Add password input field
    uiContext.addWidget(std::make_unique<TextInputWidget>(300, 310, 200, 30, password_, true));

    // Add login button
    uiContext.addWidget(std::make_unique<ButtonWidget>(300, 360, 200, 40, "Login", [this]() {
        loginRequested_ = true;
    }));

    // Add status text label
    uiContext.addWidget(std::make_unique<LabelWidget>(300, 420, 200, 30, statusText_));
}

void LoginScreen::onExit(ScreenManager& manager, eld::platform::SdlContext& context, UIManager& uiManager) {
    (void)manager;
    (void)context;
    (void)uiManager;
    std::cout << "LoginScreen: onExit\n";

    if (uiManager_) {
        auto& uiContext = uiManager_->context();
        uiContext.clear();
    }
    uiManager_ = nullptr;
}

void LoginScreen::update(ScreenManager& manager, eld::platform::SdlContext& context, InputManager& input) {
    (void)context;
    (void)input;

    // Handle login button action
    if (loginRequested_) {
        loginRequested_ = false;

        // Local mock authentication - no networking, no real auth
        if (!username_.empty() && !password_.empty()) {
            statusText_ = "Login successful (local mock)";
            std::cout << "LoginScreen: Mock login successful for user: " << username_ << "\n";

            // Transition to placeholder screen (game screen placeholder)
            manager.requestTransition(ScreenId::Placeholder);
        } else {
            statusText_ = "Error: Enter username and password";
            std::cout << "LoginScreen: Mock login failed - empty credentials\n";
        }

        // Update status text widget
        if (uiManager_) {
            auto& uiContext = uiManager_->context();
            for (auto& widget : uiContext.widgets()) {
                if (auto label = dynamic_cast<LabelWidget*>(widget.get())) {
                    if (label->getText() != "Username:" && label->getText() != "Password:") {
                        label->setText(statusText_);
                        break;
                    }
                }
            }
        }
    }
}

void LoginScreen::render(ScreenManager& manager, ClientRenderContext& renderContext, UIContext& uiContext) {
    (void)manager;
    (void)renderContext;
    (void)uiContext;
    // No-op - UI is rendered separately by UIManager
    // The screen's widgets are managed by UIContext
}

} // namespace eldoria::apps::elclient