#include "InputManager.h"

#include <SDL3/SDL.h>

namespace eldoria::apps::elclient {

void InputManager::processEvents(SDL_Event* event) {
    switch (event->type) {
        case SDL_EVENT_KEY_DOWN:
            // Ignore key repeats for pressed state
            if (!event->key.repeat) {
                keyboard_.onKeyDown(event->key.scancode);
            }
            break;

        case SDL_EVENT_KEY_UP:
            keyboard_.onKeyUp(event->key.scancode);
            break;

        case SDL_EVENT_MOUSE_MOTION:
            mouse_.onMouseMove(event->motion.x, event->motion.y);
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            mouse_.onButtonDown(event->button.button);
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            mouse_.onButtonUp(event->button.button);
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            mouse_.onWheel(event->wheel.x, event->wheel.y);
            break;

        default:
            // Ignore other events
            break;
    }
}

void InputManager::beginFrame() {
    keyboard_.beginFrame();
    mouse_.beginFrame();
}

} // namespace eldoria::apps::elclient