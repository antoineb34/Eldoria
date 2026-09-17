#include "CameraController.h"

#include <algorithm>
#include <cmath>
#include <cstdint>


namespace eld::client
{

    void CameraController::reset(
        eld::render::Camera& camera
    )
    {
        locked_ =
            true;

        distance_ =
            12.0f;

        minDistance_ =
            4.0f;

        maxDistance_ =
            30.0f;

        targetHeight_ =
            1.0f;

        dragging_ =
            false;

        orbitInitialized_ =
            false;


        camera.position = {
            31.5f,
            25.0f,
            85.0f
        };

        camera.rotation = {
            -0.45f,
            0.0f,
            0.0f
        };

        camera.viewportWidth =
            1000;

        camera.viewportHeight =
            700;
    }


    bool CameraController::locked() const noexcept
    {
        return locked_;
    }


    void CameraController::clampPitch(
        eld::render::Camera& camera
    )
    {
        constexpr float minPitch =
            -1.35f;

        constexpr float maxPitch =
            -0.15f;


        camera.rotation.x =
            std::clamp(
                camera.rotation.x,
                minPitch,
                maxPitch
            );
    }


    CameraEventResult
    CameraController::handleEvent(
        const SDL_Event& event,
        eld::render::Camera& camera
    )
    {
        CameraEventResult result;


        // ----------------------------------------------------
        // Mouse wheel zoom
        // ----------------------------------------------------

        if (
            event.type ==
            SDL_EVENT_MOUSE_WHEEL
        ) {
            float wheel =
                event.wheel.y;


            if (
                event.wheel.direction ==
                SDL_MOUSEWHEEL_FLIPPED
            ) {
                wheel =
                    -wheel;
            }


            constexpr float zoomStep =
                1.5f;


            distance_ =
                std::clamp(
                    distance_ -
                        wheel *
                        zoomStep,
                    minDistance_,
                    maxDistance_
                );
        }


        // ----------------------------------------------------
        // Middle mouse orbit
        // ----------------------------------------------------

        if (
            event.type ==
                SDL_EVENT_MOUSE_BUTTON_DOWN &&
            event.button.button ==
                SDL_BUTTON_MIDDLE
        ) {
            dragging_ =
                true;
        }


        if (
            event.type ==
                SDL_EVENT_MOUSE_BUTTON_UP &&
            event.button.button ==
                SDL_BUTTON_MIDDLE
        ) {
            dragging_ =
                false;
        }


        if (
            event.type ==
                SDL_EVENT_MOUSE_MOTION &&
            dragging_
        ) {
            constexpr float sensitivity =
                0.005f;


            camera.rotation.y -=
                event.motion.xrel *
                sensitivity;


            camera.rotation.x +=
                event.motion.yrel *
                sensitivity;


            clampPitch(
                camera
            );
        }


        // ----------------------------------------------------
        // Lock/free toggle
        // ----------------------------------------------------

        if (
            event.type ==
                SDL_EVENT_KEY_DOWN &&
            !event.key.repeat &&
            event.key.scancode ==
                SDL_SCANCODE_C
        ) {
            locked_ =
                !locked_;

            result.lockChanged =
                true;
        }


        return result;
    }


    void CameraController::setViewport(
        eld::render::Camera& camera,
        int width,
        int height
    ) const
    {
        if (
            width <= 0 ||
            height <= 0
        ) {
            return;
        }


        camera.viewportWidth =
            static_cast<std::uint32_t>(
                width
            );

        camera.viewportHeight =
            static_cast<std::uint32_t>(
                height
            );
    }


    void CameraController::update(
        eld::render::Camera& camera,
        float dt,
        const bool* keys
    )
    {
        if (!keys) {
            return;
        }


        constexpr float speed =
            20.0f;

        const float step =
            speed *
            dt;


        float localX =
            0.0f;

        float localY =
            0.0f;

        float localZ =
            0.0f;


        // Free-flight translation only applies while the
        // player-follow camera is unlocked.
        if (!locked_) {
            if (keys[SDL_SCANCODE_W]) {
                localZ -=
                    step;
            }

            if (keys[SDL_SCANCODE_S]) {
                localZ +=
                    step;
            }

            if (keys[SDL_SCANCODE_A]) {
                localX -=
                    step;
            }

            if (keys[SDL_SCANCODE_D]) {
                localX +=
                    step;
            }

            if (keys[SDL_SCANCODE_Q]) {
                localY -=
                    step;
            }

            if (keys[SDL_SCANCODE_E]) {
                localY +=
                    step;
            }
        }


        const float yaw =
            camera.rotation.y;

        const float cosYaw =
            std::cos(
                yaw
            );

        const float sinYaw =
            std::sin(
                yaw
            );


        camera.position.x +=
            localX *
                cosYaw -
            localZ *
                sinYaw;

        camera.position.z +=
            localX *
                sinYaw +
            localZ *
                cosYaw;

        camera.position.y +=
            localY;


        // Arrow rotation intentionally works in both locked
        // and free modes, matching the previous Client logic.
        constexpr float turnSpeed =
            1.5f;

        const float turn =
            turnSpeed *
            dt;


        if (keys[SDL_SCANCODE_LEFT]) {
            camera.rotation.y +=
                turn;
        }

        if (keys[SDL_SCANCODE_RIGHT]) {
            camera.rotation.y -=
                turn;
        }

        if (keys[SDL_SCANCODE_UP]) {
            camera.rotation.x +=
                turn;
        }

        if (keys[SDL_SCANCODE_DOWN]) {
            camera.rotation.x -=
                turn;
        }


        clampPitch(
            camera
        );
    }


    void CameraController::follow(
        eld::render::Camera& camera,
        const eld::math::Vec3& target
    )
    {
        if (!locked_) {
            return;
        }


        if (!orbitInitialized_) {
            camera.rotation.x =
                -0.55f;

            camera.rotation.z =
                0.0f;

            orbitInitialized_ =
                true;
        }


        const float yaw =
            camera.rotation.y;

        const float pitch =
            camera.rotation.x;


        const float sinYaw =
            std::sin(
                yaw
            );

        const float cosYaw =
            std::cos(
                yaw
            );

        const float sinPitch =
            std::sin(
                pitch
            );

        const float cosPitch =
            std::cos(
                pitch
            );


        const float targetX =
            target.x;

        const float targetY =
            target.y +
            targetHeight_;

        const float targetZ =
            target.z;


        camera.position = {
            targetX +
                sinYaw *
                cosPitch *
                distance_,

            targetY -
                sinPitch *
                distance_,

            targetZ +
                cosYaw *
                cosPitch *
                distance_
        };
    }

}
