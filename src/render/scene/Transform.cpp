#include "Transform.h"

namespace eld::render {

eld::render::Mat4 buildModelMatrix(
    const Transform& transform
) {
    eld::render::Mat4 scale =
        eld::render::Mat4::scale(transform.scale);

    eld::render::Mat4 rotationX =
        eld::render::Mat4::rotationX(transform.rotation.x);

    eld::render::Mat4 rotationY =
        eld::render::Mat4::rotationY(transform.rotation.y);

    eld::render::Mat4 rotationZ =
        eld::render::Mat4::rotationZ(transform.rotation.z);

    eld::render::Mat4 translation =
        eld::render::Mat4::translation(transform.position);

    return
        scale *
        rotationX *
        rotationY *
        rotationZ *
        translation;
}

}
