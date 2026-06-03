#include "Transform.h"

namespace rf::render_next {

rf::render::Mat4 buildModelMatrix(
    const Transform& transform
) {
    rf::render::Mat4 scale =
        rf::render::Mat4::scale(transform.scale);

    rf::render::Mat4 rotationX =
        rf::render::Mat4::rotationX(transform.rotation.x);

    rf::render::Mat4 rotationY =
        rf::render::Mat4::rotationY(transform.rotation.y);

    rf::render::Mat4 rotationZ =
        rf::render::Mat4::rotationZ(transform.rotation.z);

    rf::render::Mat4 translation =
        rf::render::Mat4::translation(transform.position);

    return
        scale *
        rotationX *
        rotationY *
        rotationZ *
        translation;
}

}
