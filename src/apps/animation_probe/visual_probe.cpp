#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#include "animation/AnimationRepository.h"
#include "cache/Cache.h"
#include "cache/Index.h"
#include "definition/DefinitionRepository.h"
#include "definition/sequence/SequenceRepository.h"
#include "definition/spot_animation/SpotAnimationRepository.h"
#include "model/ModelRepository.h"
#include "sdl/SdlContext.h"

namespace {

constexpr std::uint16_t DefaultSpotAnimationId = 123;

struct FrameLocation {
    std::uint16_t archiveId = 0;
    std::size_t frameIndex = 0;
};

struct GlobalAnimationIndex {
    std::map<std::uint16_t, eld::animation::Animation> archives;
    std::map<std::uint16_t, FrameLocation> frames;
};

struct VertexGroups {
    std::array<std::vector<std::size_t>, 256> bySkin;
};

struct ApplyStats {
    std::size_t implicitPivots = 0;
    std::size_t explicitTransforms = 0;
    std::size_t translatedVertices = 0;
    std::size_t rotatedVertices = 0;
    std::size_t scaledVertices = 0;
    std::size_t alphaFaces = 0;
    std::size_t ignoredUnknownType4 = 0;
};

struct PivotState {
    int x = 0;
    int y = 0;
    int z = 0;
};

struct ResolvedFrame {
    const eld::animation::AnimationFrame* frame = nullptr;
    const eld::animation::Skeleton* skeleton = nullptr;
    std::uint16_t archiveId = 0;
};

struct ViewerState {
    std::size_t sequenceFrameIndex = 0;
    float yaw = 0.55f;
    float pitch = -0.30f;
};

struct ProjectedPoint {
    float x = 0.0f;
    float y = 0.0f;
};

struct ProjectionBounds {
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    void include(const ProjectedPoint& point) {
        minX = std::min(minX, point.x);
        maxX = std::max(maxX, point.x);
        minY = std::min(minY, point.y);
        maxY = std::max(maxY, point.y);
    }
};

int vertexCoordinate(float value) {
    return static_cast<int>(value);
}

GlobalAnimationIndex buildGlobalAnimationIndex(
    const eld::animation::AnimationRepository& repository
) {
    GlobalAnimationIndex index;

    for (const std::uint16_t archiveId : repository.listIds()) {
        eld::animation::Animation animation = repository.get(archiveId);

        for (
            std::size_t frameIndex = 0;
            frameIndex < animation.asset.frames.size();
            ++frameIndex
        ) {
            const std::uint16_t frameId =
                animation.asset.frames[frameIndex].id;

            const auto [iterator, inserted] =
                index.frames.emplace(
                    frameId,
                    FrameLocation{
                        archiveId,
                        frameIndex
                    }
                );

            if (!inserted) {
                throw std::runtime_error(
                    "duplicate global animation frame id " +
                    std::to_string(frameId)
                );
            }

            (void) iterator;
        }

        index.archives.emplace(
            archiveId,
            std::move(animation)
        );
    }

    return index;
}

ResolvedFrame resolveFrame(
    const GlobalAnimationIndex& index,
    std::uint16_t frameId
) {
    const auto frameLocation = index.frames.find(frameId);

    if (frameLocation == index.frames.end()) {
        return {};
    }

    const auto archive =
        index.archives.find(
            frameLocation->second.archiveId
        );

    if (archive == index.archives.end()) {
        return {};
    }

    const eld::animation::Animation& animation =
        archive->second;

    if (
        frameLocation->second.frameIndex >=
        animation.asset.frames.size()
    ) {
        return {};
    }

    return ResolvedFrame{
        &animation.asset.frames[
            frameLocation->second.frameIndex
        ],
        &animation.asset.skeleton,
        frameLocation->second.archiveId
    };
}

VertexGroups buildVertexGroups(
    const eld::model::ModelMesh& mesh
) {
    VertexGroups groups;

    for (
        std::size_t vertexIndex = 0;
        vertexIndex < mesh.vertices.size();
        ++vertexIndex
    ) {
        const eld::model::Vertex& vertex =
            mesh.vertices[vertexIndex];

        if (!vertex.skin.has_value()) {
            continue;
        }

        groups.bySkin[
            *vertex.skin
        ].push_back(vertexIndex);
    }

    return groups;
}

template <typename Function>
std::size_t forEachVertexInGroups(
    eld::model::ModelMesh& mesh,
    const VertexGroups& groups,
    const std::vector<std::uint8_t>& skinGroups,
    Function&& function
) {
    std::size_t count = 0;

    for (const std::uint8_t groupId : skinGroups) {
        for (
            const std::size_t vertexIndex :
            groups.bySkin[groupId]
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
    eld::model::ModelMesh& mesh,
    const VertexGroups& groups,
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
    eld::model::ModelMesh& mesh,
    const VertexGroups& groups,
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
                    vertexCoordinate(vertex.x) +
                    x
                );

            vertex.y =
                static_cast<float>(
                    vertexCoordinate(vertex.y) +
                    y
                );

            vertex.z =
                static_cast<float>(
                    vertexCoordinate(vertex.z) +
                    z
                );
        }
    );
}

std::size_t applyScale(
    eld::model::ModelMesh& mesh,
    const VertexGroups& groups,
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
            const int sourceX = vertexCoordinate(vertex.x);
            const int sourceY = vertexCoordinate(vertex.y);
            const int sourceZ = vertexCoordinate(vertex.z);

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

// Classic RS317 type-2 animation rotation.
//
// The frame values are byte-sized angle units. The original client converts
// each component to the 2048-entry trig-table domain with:
//
//     angle = (value & 0xff) * 8
//
// Vertices rotate around the current animation pivot in this exact order:
// Z, then X, then Y. The original client uses 16.16 fixed-point trig values
// and signed arithmetic >> 16 after each multiply/add.
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

std::size_t applyRotation(
    eld::model::ModelMesh& mesh,
    const VertexGroups& groups,
    const eld::animation::SkeletonSlot& slot,
    int x,
    int y,
    int z,
    const PivotState& pivot
) {
    const int angleX =
        (x & 0xff) *
        8;

    const int angleY =
        (y & 0xff) *
        8;

    const int angleZ =
        (z & 0xff) *
        8;

    const std::array<int, 2048>& sine =
        rsSineTable();

    const std::array<int, 2048>& cosine =
        rsCosineTable();

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

            // Z first.
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

            // X second.
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

            // Y last.
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

std::size_t countFacesInGroups(
    const eld::model::ModelMesh& mesh,
    const std::vector<std::uint8_t>& groups
) {
    std::array<bool, 256> selected {};

    for (const std::uint8_t group : groups) {
        selected[group] = true;
    }

    std::size_t count = 0;

    for (const eld::model::Face& face : mesh.faces) {
        if (
            face.skin.has_value() &&
            selected[*face.skin]
        ) {
            ++count;
        }
    }

    return count;
}

void applyTransform(
    eld::model::ModelMesh& mesh,
    const VertexGroups& groups,
    const eld::animation::SkeletonSlot& slot,
    int x,
    int y,
    int z,
    PivotState& pivot,
    ApplyStats& stats,
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
            // Geometry-only visual proof. We count the faces but do not
            // hide them yet, otherwise frame 6393 would largely vanish.
            stats.alphaFaces +=
                countFacesInGroups(
                    mesh,
                    slot.groups
                );
            break;

        default:
            break;
    }
}

ApplyStats applyFrameGeometry(
    eld::model::ModelMesh& mesh,
    const eld::animation::AnimationFrame& frame,
    const eld::animation::Skeleton& skeleton
) {
    const VertexGroups groups =
        buildVertexGroups(mesh);

    ApplyStats stats;
    PivotState pivot;

    int lastExplicitSlot = -1;

    for (
        const eld::animation::FrameTransform& transform :
        frame.transforms
    ) {
        ++stats.explicitTransforms;

        if (transform.slot >= skeleton.slots.size()) {
            continue;
        }

        const int currentSlot =
            static_cast<int>(transform.slot);

        const eld::animation::SkeletonSlot& current =
            skeleton.slots[transform.slot];

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

ProjectedPoint projectVertex(
    const eld::model::Vertex& vertex,
    float yaw,
    float pitch
) {
    float x = vertex.x;
    float y = vertex.y;
    float z = vertex.z;

    const float sinYaw = std::sin(yaw);
    const float cosYaw = std::cos(yaw);
    const float sinPitch = std::sin(pitch);
    const float cosPitch = std::cos(pitch);

    const float yawedX =
        x * cosYaw +
        z * sinYaw;

    const float yawedZ =
        z * cosYaw -
        x * sinYaw;

    x = yawedX;
    z = yawedZ;

    const float pitchedY =
        y * cosPitch -
        z * sinPitch;

    return ProjectedPoint{
        x,
        -pitchedY
    };
}

std::vector<ProjectedPoint> projectMesh(
    const eld::model::ModelMesh& mesh,
    float yaw,
    float pitch,
    ProjectionBounds& bounds
) {
    std::vector<ProjectedPoint> projected;
    projected.reserve(mesh.vertices.size());

    for (const eld::model::Vertex& vertex : mesh.vertices) {
        const ProjectedPoint point =
            projectVertex(
                vertex,
                yaw,
                pitch
            );

        projected.push_back(point);
        bounds.include(point);
    }

    return projected;
}

void drawWireframe(
    SDL_Renderer* renderer,
    const eld::model::ModelMesh& mesh,
    const std::vector<ProjectedPoint>& projected,
    float centerX,
    float centerY,
    float worldCenterX,
    float worldCenterY,
    float scale,
    std::uint8_t red,
    std::uint8_t green,
    std::uint8_t blue
) {
    SDL_SetRenderDrawColor(
        renderer,
        red,
        green,
        blue,
        255
    );

    auto screenPoint =
        [&](std::uint32_t vertexIndex) {
            const ProjectedPoint& point =
                projected.at(vertexIndex);

            return SDL_FPoint{
                centerX +
                    (point.x - worldCenterX) *
                    scale,
                centerY +
                    (point.y - worldCenterY) *
                    scale
            };
        };

    for (const eld::model::Face& face : mesh.faces) {
        if (
            face.a >= projected.size() ||
            face.b >= projected.size() ||
            face.c >= projected.size()
        ) {
            continue;
        }

        const SDL_FPoint a = screenPoint(face.a);
        const SDL_FPoint b = screenPoint(face.b);
        const SDL_FPoint c = screenPoint(face.c);

        SDL_RenderLine(
            renderer,
            a.x,
            a.y,
            b.x,
            b.y
        );

        SDL_RenderLine(
            renderer,
            b.x,
            b.y,
            c.x,
            c.y
        );

        SDL_RenderLine(
            renderer,
            c.x,
            c.y,
            a.x,
            a.y
        );
    }
}

void printSelectedFrame(
    std::size_t sequenceFrameIndex,
    const eld::definition::SequenceFrame& sequenceFrame,
    const ResolvedFrame& resolved,
    const ApplyStats& stats
) {
    std::cout
        << "\nVisual frame "
        << sequenceFrameIndex
        << '\n'
        << "  global frame id:        "
        << sequenceFrame.primaryFrameId
        << '\n'
        << "  animation archive:      "
        << resolved.archiveId
        << '\n'
        << "  sequence duration:      "
        << sequenceFrame.duration
        << '\n'
        << "  animation frame delay:  "
        << static_cast<unsigned int>(
            resolved.frame->delay
        )
        << '\n'
        << "  explicit transforms:    "
        << stats.explicitTransforms
        << '\n'
        << "  implicit pivots:        "
        << stats.implicitPivots
        << '\n'
        << "  translated vertex hits: "
        << stats.translatedVertices
        << '\n'
        << "  rotated vertex hits:    "
        << stats.rotatedVertices
        << '\n'
        << "  scaled vertex hits:     "
        << stats.scaledVertices
        << '\n'
        << "  alpha face hits:        "
        << stats.alphaFaces
        << " (kept visible)\n";
}

std::optional<std::uint16_t> parseSpotId(
    const char* value
) {
    try {
        const unsigned long parsed =
            std::stoul(value);

        if (parsed > 65535UL) {
            return std::nullopt;
        }

        return static_cast<std::uint16_t>(parsed);
    }
    catch (...) {
        return std::nullopt;
    }
}

int run(
    const std::filesystem::path& cacheRoot,
    std::uint16_t spotAnimationId
) {
    eld::cache::Cache cache(cacheRoot);

    eld::definition::DefinitionRepository definitions(
        cache.open(
            eld::cache::IndexId::Config
        ),
        2
    );

    eld::definition::SpotAnimationRepository spots(
        definitions.get("spotanim")
    );

    eld::definition::SequenceRepository sequences(
        definitions.get("seq")
    );

    eld::model::ModelRepository models(
        cache.open(
            eld::cache::IndexId::Models
        )
    );

    eld::animation::AnimationRepository animations(
        cache.open(
            eld::cache::IndexId::Animations
        )
    );

    const eld::definition::SpotAnimationDefinition* spot =
        spots.find(spotAnimationId);

    if (spot == nullptr) {
        throw std::runtime_error(
            "SpotAnim " +
            std::to_string(spotAnimationId) +
            " not found"
        );
    }

    if (
        !spot->modelId.has_value() ||
        !spot->sequenceId.has_value()
    ) {
        throw std::runtime_error(
            "selected SpotAnim has no complete model + sequence link"
        );
    }

    const eld::definition::SequenceDefinition* sequence =
        sequences.find(*spot->sequenceId);

    if (
        sequence == nullptr ||
        sequence->frames.empty()
    ) {
        throw std::runtime_error(
            "selected SpotAnim sequence is missing or empty"
        );
    }

    const eld::model::Model sourceModel =
        models.get(*spot->modelId);

    const eld::model::ModelMesh originalMesh =
        sourceModel.mesh;

    const GlobalAnimationIndex animationIndex =
        buildGlobalAnimationIndex(animations);

    std::cout
        << "Eldoria Animation Visual Probe\n"
        << "==============================\n"
        << "SpotAnim: "
        << spot->id
        << '\n'
        << "model:    "
        << *spot->modelId
        << '\n'
        << "sequence: "
        << *spot->sequenceId
        << '\n'
        << "frames:   "
        << sequence->frames.size()
        << '\n'
        << '\n'
        << "LEFT  = original model\n"
        << "RIGHT = selected animation frame applied\n"
        << '\n'
        << "Controls\n"
        << "--------\n"
        << "Left / Right : previous / next sequence frame\n"
        << "A / D        : rotate view left / right\n"
        << "W / S        : rotate view up / down\n"
        << "R            : reset view\n"
        << "Esc          : quit\n"
        << '\n'
        << "Alpha transforms are counted but intentionally kept visible.\n"
        << "Type-2 rotation uses classic RS fixed-point Z -> X -> Y order.\n";

    eld::platform::SdlContext sdl(
        "Eldoria Animation Visual Probe",
        1500,
        850
    );

    SDL_Window* window = sdl.window();
    SDL_Renderer* renderer = sdl.renderer();

    if (
        window == nullptr ||
        renderer == nullptr
    ) {
        return 1;
    }

    ViewerState state;

    bool running = true;
    bool frameDirty = true;

    eld::model::ModelMesh deformedMesh;
    ResolvedFrame resolved;
    ApplyStats stats;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                continue;
            }

            if (event.type != SDL_EVENT_KEY_DOWN) {
                continue;
            }

            if (event.key.key == SDLK_ESCAPE) {
                running = false;
                continue;
            }

            switch (event.key.scancode) {
                case SDL_SCANCODE_LEFT:
                    if (state.sequenceFrameIndex > 0) {
                        --state.sequenceFrameIndex;
                    }
                    else {
                        state.sequenceFrameIndex =
                            sequence->frames.size() - 1;
                    }

                    frameDirty = true;
                    break;

                case SDL_SCANCODE_RIGHT:
                    state.sequenceFrameIndex =
                        (
                            state.sequenceFrameIndex +
                            1
                        ) %
                        sequence->frames.size();

                    frameDirty = true;
                    break;

                case SDL_SCANCODE_A:
                    state.yaw -= 0.12f;
                    break;

                case SDL_SCANCODE_D:
                    state.yaw += 0.12f;
                    break;

                case SDL_SCANCODE_W:
                    state.pitch -= 0.10f;
                    break;

                case SDL_SCANCODE_S:
                    state.pitch += 0.10f;
                    break;

                case SDL_SCANCODE_R:
                    state.yaw = 0.55f;
                    state.pitch = -0.30f;
                    break;

                default:
                    break;
            }
        }

        if (frameDirty) {
            const eld::definition::SequenceFrame& sequenceFrame =
                sequence->frames[
                    state.sequenceFrameIndex
                ];

            resolved =
                resolveFrame(
                    animationIndex,
                    sequenceFrame.primaryFrameId
                );

            if (
                resolved.frame == nullptr ||
                resolved.skeleton == nullptr
            ) {
                throw std::runtime_error(
                    "failed to resolve sequence frame " +
                    std::to_string(
                        sequenceFrame.primaryFrameId
                    )
                );
            }

            deformedMesh = originalMesh;

            stats =
                applyFrameGeometry(
                    deformedMesh,
                    *resolved.frame,
                    *resolved.skeleton
                );

            printSelectedFrame(
                state.sequenceFrameIndex,
                sequenceFrame,
                resolved,
                stats
            );

            const std::string title =
                "Eldoria Animation Probe | SpotAnim " +
                std::to_string(spot->id) +
                " | frame " +
                std::to_string(
                    state.sequenceFrameIndex
                ) +
                "/" +
                std::to_string(
                    sequence->frames.size() - 1
                ) +
                " | global " +
                std::to_string(
                    sequenceFrame.primaryFrameId
                ) +
                " | delay " +
                std::to_string(
                    static_cast<unsigned int>(
                        resolved.frame->delay
                    )
                );

            SDL_SetWindowTitle(
                window,
                title.c_str()
            );

            frameDirty = false;
        }

        int windowWidth = 0;
        int windowHeight = 0;

        SDL_GetWindowSize(
            window,
            &windowWidth,
            &windowHeight
        );

        SDL_SetRenderDrawColor(
            renderer,
            18,
            20,
            24,
            255
        );

        SDL_RenderClear(renderer);

        ProjectionBounds bounds;

        const std::vector<ProjectedPoint> originalProjected =
            projectMesh(
                originalMesh,
                state.yaw,
                state.pitch,
                bounds
            );

        const std::vector<ProjectedPoint> deformedProjected =
            projectMesh(
                deformedMesh,
                state.yaw,
                state.pitch,
                bounds
            );

        const float spanX =
            std::max(
                1.0f,
                bounds.maxX - bounds.minX
            );

        const float spanY =
            std::max(
                1.0f,
                bounds.maxY - bounds.minY
            );

        const float worldCenterX =
            (
                bounds.minX +
                bounds.maxX
            ) *
            0.5f;

        const float worldCenterY =
            (
                bounds.minY +
                bounds.maxY
            ) *
            0.5f;

        const float panelWidth =
            static_cast<float>(windowWidth) *
            0.5f;

        const float panelHeight =
            static_cast<float>(windowHeight);

        const float fitScale =
            std::min(
                (
                    panelWidth *
                    0.82f
                ) /
                spanX,
                (
                    panelHeight *
                    0.82f
                ) /
                spanY
            );

        SDL_SetRenderDrawColor(
            renderer,
            70,
            76,
            88,
            255
        );

        SDL_RenderLine(
            renderer,
            panelWidth,
            0.0f,
            panelWidth,
            panelHeight
        );

        drawWireframe(
            renderer,
            originalMesh,
            originalProjected,
            panelWidth * 0.5f,
            panelHeight * 0.5f,
            worldCenterX,
            worldCenterY,
            fitScale,
            185,
            190,
            205
        );

        drawWireframe(
            renderer,
            deformedMesh,
            deformedProjected,
            panelWidth +
                panelWidth *
                    0.5f,
            panelHeight * 0.5f,
            worldCenterX,
            worldCenterY,
            fitScale,
            110,
            225,
            155
        );

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    return 0;
}

}

int main(
    int argc,
    char** argv
) {
    if (
        argc < 2 ||
        argc > 3
    ) {
        std::cerr
            << "usage: "
            << (
                argc > 0
                    ? argv[0]
                    : "animation_visual_probe"
            )
            << " <cache-root> [spotanim-id]\n"
            << "default spotanim-id: "
            << DefaultSpotAnimationId
            << '\n';

        return 1;
    }

    std::uint16_t spotAnimationId =
        DefaultSpotAnimationId;

    if (argc == 3) {
        const std::optional<std::uint16_t> parsed =
            parseSpotId(argv[2]);

        if (!parsed.has_value()) {
            std::cerr
                << "invalid spotanim id: "
                << argv[2]
                << '\n';

            return 1;
        }

        spotAnimationId = *parsed;
    }

    try {
        return run(
            std::filesystem::path(argv[1]),
            spotAnimationId
        );
    }
    catch (
        const std::exception& exception
    ) {
        std::cerr
            << "animation_visual_probe failed: "
            << exception.what()
            << '\n';

        return 1;
    }
}

