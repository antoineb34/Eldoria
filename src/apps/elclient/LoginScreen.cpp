#include "LoginScreen.h"
#include "ScreenManager.h"
#include "InputManager.h"
#include "ClientRenderContext.h"
#include "UIContext.h"
#include "UIManager.h"
#include "TextInputWidget.h"
#include "ButtonWidget.h"
#include "LabelWidget.h"

#include "../../platform/sdl/SdlContext.h"

#include <iostream>
#include <memory>
#include <string>

namespace eldoria::apps::elclient {

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

    // Add status text label - keep explicit reference
    auto statusLabel = std::make_unique<LabelWidget>(300, 420, 200, 30, statusText_);
    statusLabel_ = statusLabel.get();
    uiContext.addWidget(std::move(statusLabel));
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
    statusLabel_ = nullptr;
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

        // Update status text label via explicit reference
        if (statusLabel_) {
            statusLabel_->setText(statusText_);
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