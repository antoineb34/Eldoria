#include "Transform.h"

namespace eld::render {

eld::math::Mat4 buildModelMatrix(
    const Transform& transform
) {
    eld::math::Mat4 scale =
        eld::math::Mat4::scale(transform.scale);

    eld::math::Mat4 rotationX =
        eld::math::Mat4::rotationX(transform.rotation.x);

    eld::math::Mat4 rotationY =
        eld::math::Mat4::rotationY(transform.rotation.y);

    eld::math::Mat4 rotationZ =
        eld::math::Mat4::rotationZ(transform.rotation.z);

    eld::math::Mat4 translation =
        eld::math::Mat4::translation(transform.position);

    return
        scale *
        rotationX *
        rotationY *
        rotationZ *
        translation;
}

}
