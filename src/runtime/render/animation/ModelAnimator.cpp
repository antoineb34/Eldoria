#include "ModelAnimator.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include "ModelSkinGroups.h"

namespace eld::render {

namespace {

struct PivotState {
    int x = 0;
    int y = 0;
    int z = 0;
};

int vertexCoordinate(float value) {
    return static_cast<int>(value);
}

template <typename Function>
std::size_t forEachVertexInGroups(
    eld::model::Model& mesh,
    const ModelSkinGroups& groups,
    const std::vector<std::uint8_t>& skinGroups,
    Function&& function
) {
    std::size_t count = 0;

    for (const std::uint8_t groupId : skinGroups) {
        for (
            const std::size_t vertexIndex :
            groups.vertices(groupId)
        ) {
            if (vertexIndex >= mesh.vertices.size()) {
                continue;
            }

            function(mesh.vertices[vertexIndex]);
            ++count;
        }
    }

    return count;
}

void applyPivot(
    eld::model::Model& mesh,
    const ModelSkinGroups& groups,
    const eld::animation::SkeletonSlot& slot,
    int x,
    int y,
    int z,
    PivotState& pivot
) {
    long long totalX = 0;
    long long totalY = 0;
    long long totalZ = 0;
    std::size_t vertexCount = 0;

    forEachVertexInGroups(
        mesh,
        groups,
        slot.groups,
        [&](eld::model::Vertex& vertex) {
            totalX += vertexCoordinate(vertex.x);
            totalY += vertexCoordinate(vertex.y);
            totalZ += vertexCoordinate(vertex.z);
            ++vertexCount;
        }
    );

    if (vertexCount > 0) {
        pivot.x =
            static_cast<int>(
                totalX /
                static_cast<long long>(vertexCount)
            ) +
            x;

        pivot.y =
            static_cast<int>(
                totalY /
                static_cast<long long>(vertexCount)
            ) +
            y;

        pivot.z =
            static_cast<int>(
                totalZ /
                static_cast<long long>(vertexCount)
            ) +
            z;
    }
    else {
        pivot.x = x;
        pivot.y = y;
        pivot.z = z;
    }
}

std::size_t applyTranslation(
    eld::model::Model& mesh,
    const ModelSkinGroups& groups,
    const eld::animation::SkeletonSlot& slot,
    int x,
    int y,
    int z
) {
    return forEachVertexInGroups(
        mesh,
        groups,
        slot.groups,
        [&](eld::model::Vertex& vertex) {
            vertex.x =
                static_cast<float>(
                    vertexCoordinate(vertex.x) + x
                );

            vertex.y =
                static_cast<float>(
                    vertexCoordinate(vertex.y) + y
                );

            vertex.z =
                static_cast<float>(
                    vertexCoordinate(vertex.z) + z
                );
        }
    );
}

// Java's signed arithmetic >> 16, expressed without relying on
// implementation-defined right shift behavior for negative C++ integers.
int arithmeticShift16(
    std::int64_t value
) {
    constexpr std::int64_t divisor =
        static_cast<std::int64_t>(1) << 16;

    if (value >= 0) {
        return static_cast<int>(
            value / divisor
        );
    }

    return -static_cast<int>(
        (
            -value +
            divisor -
            1
        ) /
        divisor
    );
}

const std::array<int, 2048>& rsSineTable() {
    static const std::array<int, 2048> table = [] {
        std::array<int, 2048> values {};

        constexpr double twoPi =
            6.283185307179586476925286766559;

        for (
            std::size_t i = 0;
            i < values.size();
            ++i
        ) {
            const double angle =
                static_cast<double>(i) *
                twoPi /
                2048.0;

            values[i] =
                static_cast<int>(
                    65536.0 *
                    std::sin(angle)
                );
        }

        return values;
    }();

    return table;
}

const std::array<int, 2048>& rsCosineTable() {
    static const std::array<int, 2048> table = [] {
        std::array<int, 2048> values {};

        constexpr double twoPi =
            6.283185307179586476925286766559;

        for (
            std::size_t i = 0;
            i < values.size();
            ++i
        ) {
            const double angle =
                static_cast<double>(i) *
                twoPi /
                2048.0;

            values[i] =
                static_cast<int>(
                    65536.0 *
                    std::cos(angle)
                );
        }

        return values;
    }();

    return table;
}

// Classic RS317 transform type 2.
//
// Each component becomes (value & 0xff) * 8 in the 2048-entry trig table.
// Rotation order is exactly Z -> X -> Y around the current pivot.
std::size_t applyRotation(
    eld::model::Model& mesh,
    const ModelSkinGroups& groups,
    const eld::animation::SkeletonSlot& slot,
    int x,
    int y,
    int z,
    const PivotState& pivot
) {
    const int angleX = (x & 0xff) * 8;
    const int angleY = (y & 0xff) * 8;
    const int angleZ = (z & 0xff) * 8;

    const auto& sine = rsSineTable();
    const auto& cosine = rsCosineTable();

    return forEachVertexInGroups(
        mesh,
        groups,
        slot.groups,
        [&](eld::model::Vertex& vertex) {
            int vx =
                vertexCoordinate(vertex.x) -
                pivot.x;

            int vy =
                vertexCoordinate(vertex.y) -
                pivot.y;

            int vz =
                vertexCoordinate(vertex.z) -
                pivot.z;

            if (angleZ != 0) {
                const int sinZ = sine[angleZ];
                const int cosZ = cosine[angleZ];

                const int rotatedX =
                    arithmeticShift16(
                        static_cast<std::int64_t>(vy) * sinZ +
                        static_cast<std::int64_t>(vx) * cosZ
                    );

                vy =
                    arithmeticShift16(
                        static_cast<std::int64_t>(vy) * cosZ -
                        static_cast<std::int64_t>(vx) * sinZ
                    );

                vx = rotatedX;
            }

            if (angleX != 0) {
                const int sinX = sine[angleX];
                const int cosX = cosine[angleX];

                const int rotatedY =
                    arithmeticShift16(
                        static_cast<std::int64_t>(vy) * cosX -
                        static_cast<std::int64_t>(vz) * sinX
                    );

                vz =
                    arithmeticShift16(
                        static_cast<std::int64_t>(vy) * sinX +
                        static_cast<std::int64_t>(vz) * cosX
                    );

                vy = rotatedY;
            }

            if (angleY != 0) {
                const int sinY = sine[angleY];
                const int cosY = cosine[angleY];

                const int rotatedX =
                    arithmeticShift16(
                        static_cast<std::int64_t>(vz) * sinY +
                        static_cast<std::int64_t>(vx) * cosY
                    );

                vz =
                    arithmeticShift16(
                        static_cast<std::int64_t>(vz) * cosY -
                        static_cast<std::int64_t>(vx) * sinY
                    );

                vx = rotatedX;
            }

            vertex.x =
                static_cast<float>(
                    vx + pivot.x
                );

            vertex.y =
                static_cast<float>(
                    vy + pivot.y
                );

            vertex.z =
                static_cast<float>(
                    vz + pivot.z
                );
        }
    );
}

std::size_t applyScale(
    eld::model::Model& mesh,
    const ModelSkinGroups& groups,
    const eld::animation::SkeletonSlot& slot,
    int x,
    int y,
    int z,
    const PivotState& pivot
) {
    return forEachVertexInGroups(
        mesh,
        groups,
        slot.groups,
        [&](eld::model::Vertex& vertex) {
            const int sourceX =
                vertexCoordinate(vertex.x);

            const int sourceY =
                vertexCoordinate(vertex.y);

            const int sourceZ =
                vertexCoordinate(vertex.z);

            vertex.x =
                static_cast<float>(
                    (
                        (sourceX - pivot.x) *
                        x /
                        128
                    ) +
                    pivot.x
                );

            vertex.y =
                static_cast<float>(
                    (
                        (sourceY - pivot.y) *
                        y /
                        128
                    ) +
                    pivot.y
                );

            vertex.z =
                static_cast<float>(
                    (
                        (sourceZ - pivot.z) *
                        z /
                        128
                    ) +
                    pivot.z
                );
        }
    );
}

// Classic transform type 5. Only X is used. RS alpha uses 0 = opaque,
// 255 = transparent, and each animation unit changes alpha by x * 8.
std::size_t applyAlpha(
    eld::model::Model& mesh,
    const ModelSkinGroups& groups,
    const eld::animation::SkeletonSlot& slot,
    int x
) {
    std::size_t count = 0;

    for (const std::uint8_t groupId : slot.groups) {
        for (
            const std::size_t faceIndex :
            groups.faces(groupId)
        ) {
            if (faceIndex >= mesh.faces.size()) {
                continue;
            }

            eld::model::Face& face =
                mesh.faces[faceIndex];

            const int alpha =
                std::clamp(
                    static_cast<int>(face.alpha) +
                    x * 8,
                    0,
                    255
                );

            face.alpha =
                static_cast<std::uint8_t>(alpha);

            ++count;
        }
    }

    return count;
}

void applyTransform(
    eld::model::Model& mesh,
    const ModelSkinGroups& groups,
    const eld::animation::SkeletonSlot& slot,
    int x,
    int y,
    int z,
    PivotState& pivot,
    AnimationApplyStats& stats,
    bool implicit
) {
    switch (slot.type) {
        case 0:
            applyPivot(
                mesh,
                groups,
                slot,
                x,
                y,
                z,
                pivot
            );

            if (implicit) {
                ++stats.implicitPivots;
            }
            break;

        case 1:
            stats.translatedVertices +=
                applyTranslation(
                    mesh,
                    groups,
                    slot,
                    x,
                    y,
                    z
                );
            break;

        case 2:
            stats.rotatedVertices +=
                applyRotation(
                    mesh,
                    groups,
                    slot,
                    x,
                    y,
                    z,
                    pivot
                );
            break;

        case 3:
            stats.scaledVertices +=
                applyScale(
                    mesh,
                    groups,
                    slot,
                    x,
                    y,
                    z,
                    pivot
                );
            break;

        case 4:
            ++stats.ignoredUnknownType4;
            break;

        case 5:
            stats.alphaFaces +=
                applyAlpha(
                    mesh,
                    groups,
                    slot,
                    x
                );
            break;

        default:
            break;
    }
}

}

AnimatedModelFrame ModelAnimator::apply(
    const eld::model::Model& source,
    const eld::animation::AnimationFrame& frame,
    const eld::animation::Skeleton& skeleton
) const {
    AnimatedModelFrame result;
    result.mesh = source;

    result.stats =
        applyInPlace(
            result.mesh,
            frame,
            skeleton
        );

    return result;
}

AnimationApplyStats ModelAnimator::applyInPlace(
    eld::model::Model& mesh,
    const eld::animation::AnimationFrame& frame,
    const eld::animation::Skeleton& skeleton
) const {
    const ModelSkinGroups groups =
        ModelSkinGroups::build(mesh);

    AnimationApplyStats stats;
    PivotState pivot;

    int lastExplicitSlot = -1;

    for (
        const eld::animation::FrameTransform& transform :
        frame.transforms
    ) {
        ++stats.explicitTransforms;

        if (transform.slot >= skeleton.slots.size()) {
            ++stats.invalidSkeletonSlots;
            continue;
        }

        const int currentSlot =
            static_cast<int>(transform.slot);

        const eld::animation::SkeletonSlot& current =
            skeleton.slots[transform.slot];

        // Classic decoder/application behavior: when a non-pivot explicit
        // slot is reached, execute the nearest skipped type-0 slot between
        // the previous explicit transform and this one as an implicit pivot.
        if (current.type != 0) {
            for (
                int candidate = currentSlot - 1;
                candidate > lastExplicitSlot;
                --candidate
            ) {
                if (candidate < 0) {
                    break;
                }

                const eld::animation::SkeletonSlot& skipped =
                    skeleton.slots[
                        static_cast<std::size_t>(candidate)
                    ];

                if (skipped.type == 0) {
                    applyTransform(
                        mesh,
                        groups,
                        skipped,
                        0,
                        0,
                        0,
                        pivot,
                        stats,
                        true
                    );
                    break;
                }
            }
        }

        applyTransform(
            mesh,
            groups,
            current,
            transform.x,
            transform.y,
            transform.z,
            pivot,
            stats,
            false
        );

        lastExplicitSlot = currentSlot;
    }

    return stats;
}

}
