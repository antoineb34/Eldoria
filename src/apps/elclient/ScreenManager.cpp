#include "ScreenManager.h"

#include "Screen.h"
#include "InputManager.h"
#include "ClientRenderContext.h"

#include <iostream>

namespace eldoria::apps::elclient {

void ScreenManager::registerScreen(ScreenPtr screen) {
    ScreenId id = screen->id();
    size_t index = static_cast<size_t>(id);

    if (index >= MaxScreens) {
        std::cerr << "ScreenManager: invalid screen ID " << static_cast<uint8_t>(id) << "\n";
        return;
    }

    if (screens_[index]) {
        std::cerr << "ScreenManager: screen " << screenIdToString(id) << " already registered\n";
        return;
    }

    screens_[index] = std::move(screen);
    std::cout << "ScreenManager: registered screen " << screenIdToString(id) << "\n";
}

void ScreenManager::requestTransition(ScreenId to) {
    if (to == ScreenId::Invalid) {
        std::cerr << "ScreenManager: cannot transition to Invalid screen\n";
        return;
    }

    size_t index = static_cast<size_t>(to);
    if (index >= MaxScreens || !screens_[index]) {
        std::cerr << "ScreenManager: target screen " << screenIdToString(to) << " not registered\n";
        return;
    }

    pending_ = to;
    transitionPending_ = true;
    std::cout << "ScreenManager: transition requested to " << screenIdToString(to) << "\n";
}

Screen* ScreenManager::activeScreen() {
    return active_;
}

const Screen* ScreenManager::activeScreen() const {
    return active_;
}

ScreenId ScreenManager::activeScreenId() const {
    return active_ ? active_->id() : ScreenId::Invalid;
}

bool ScreenManager::hasPendingTransition() const {
    return transitionPending_;
}

ScreenId ScreenManager::pendingTransition() const {
    return pending_;
}

void ScreenManager::processTransition(eld::platform::SdlContext& context) {
    if (!transitionPending_) {
        return;
    }

    performTransition(pending_, context);
    pending_ = ScreenId::Invalid;
    transitionPending_ = false;
}

void ScreenManager::update(eld::platform::SdlContext& context, InputManager& input) {
    // Process any pending transition first
    processTransition(context);

    if (active_) {
        active_->update(*this, context, input);
    }
}

void ScreenManager::render(ClientRenderContext& renderContext) {
    if (active_) {
        active_->render(*this, renderContext);
    }
}

bool ScreenManager::hasScreen(ScreenId id) const {
    size_t index = static_cast<size_t>(id);
    return index < MaxScreens && screens_[index] != nullptr;
}

void ScreenManager::performTransition(ScreenId to, eld::platform::SdlContext& context) {
    if (active_) {
        active_->onExit(*this, context);
        std::cout << "ScreenManager: exited screen " << screenIdToString(active_->id()) << "\n";
    }

    size_t index = static_cast<size_t>(to);
    active_ = screens_[index].get();

    if (active_) {
        active_->onEnter(*this, context);
        std::cout << "ScreenManager: entered screen " << screenIdToString(active_->id()) << "\n";
    } else {
        std::cerr << "ScreenManager: failed to transition to " << screenIdToString(to) << " - not registered\n";
    }
}

} // namespace eldoria::apps::elclient