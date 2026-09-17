#pragma once

#include <SDL3/SDL.h>

#include "math/Vec3.h"
#include "render/camera/Camera.h"


namespace eld::client
{

    struct CameraEventResult
    {
        bool lockChanged =
            false;
    };


    class CameraController
    {
    public:
        void reset(
            eld::render::Camera& camera
        );


        CameraEventResult handleEvent(
            const SDL_Event& event,
            eld::render::Camera& camera
        );


        void update(
            eld::render::Camera& camera,
            float dt,
            const bool* keys
        );


        void follow(
            eld::render::Camera& camera,
            const eld::math::Vec3& target
        );


        void setViewport(
            eld::render::Camera& camera,
            int width,
            int height
        ) const;


        bool locked() const noexcept;


    private:
        static void clampPitch(
            eld::render::Camera& camera
        );


        bool locked_ =
            true;

        float distance_ =
            12.0f;

        float minDistance_ =
            4.0f;

        float maxDistance_ =
            30.0f;

        float targetHeight_ =
            1.0f;

        bool dragging_ =
            false;

        bool orbitInitialized_ =
            false;
    };

}
