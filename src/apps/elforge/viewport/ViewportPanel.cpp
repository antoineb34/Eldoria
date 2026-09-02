#include "ui/ElForgeTheme.h"
#include "ui/IconButton.h"
#include "ui/WorkspaceUi.h"
#include "viewport/ViewportPanel.h"
#include "views/animation/AnimationViewPanel.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <exception>
#include <limits>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <imgui.h>

#include "midi/MidiPlayer.h"

#include "math/Mat4.h"
#include "math/Vec3.h"
#include "math/Vec4.h"
#include "render/camera/Projection.h"
#include "render/scene/Transform.h"

#include "explorer/CacheExplorerState.h"
#include "views/interface/InterfaceView.h"

#include "graphics/GraphicsResources.h"
#include "interface/InterfaceRepository.h"
#include "sprite/SpriteRepository.h"

#include "../../../render/RenderPipeline.h"
#include "../../../render/backend/software/SoftwareRenderBackend.h"
#include "../../../render/camera/Projection.h"
#include "../../../render/scene/Transform.h"

namespace eld::elforge {

namespace {

constexpr int InterfaceSlotSize = 32;
constexpr int InterfaceModelRenderTargetSize = 512;
constexpr int DebugFontWidth = 8;
constexpr int DebugFontHeight = 8;

struct PixelSize {
    int width = 1;
    int height = 1;
};

struct SpriteRef {
    std::string group;
    std::uint16_t frame = 0;
};

struct SpriteTexture {
    SDL_Texture* texture = nullptr;
    int width = 0;
    int height = 0;
};

PixelSize widgetPixelSize(
    const eld::interface::InterfaceFileWidget& widget
) {
    if (widget.type == 2) {
        const int columns =
            static_cast<int>(
                widget.width
            );

        const int rows =
            static_cast<int>(
                widget.height
            );

        return {
            std::max(
                columns * InterfaceSlotSize +
                    static_cast<int>(
                        widget.inventoryPaddingX
                    ) *
                    std::max(columns - 1, 0),
                1
            ),
            std::max(
                rows * InterfaceSlotSize +
                    static_cast<int>(
                        widget.inventoryPaddingY
                    ) *
                    std::max(rows - 1, 0),
                1
            )
        };
    }

    if (widget.type == 7) {
        const int columns =
            static_cast<int>(
                widget.width
            );

        const int rows =
            static_cast<int>(
                widget.height
            );

        return {
            std::max(
                columns * InterfaceSlotSize +
                    static_cast<int>(
                        widget.itemPaddingX
                    ) *
                    std::max(columns - 1, 0),
                1
            ),
            std::max(
                rows * InterfaceSlotSize +
                    static_cast<int>(
                        widget.itemPaddingY
                    ) *
                    std::max(rows - 1, 0),
                1
            )
        };
    }

    return {
        std::max(
            static_cast<int>(
                widget.width
            ),
            1
        ),
        std::max(
            static_cast<int>(
                widget.height
            ),
            1
        )
    };
}

std::vector<std::string> splitText(
    const std::string& text
) {
    std::vector<std::string> lines;
    std::string current;

    for (
        std::size_t index = 0;
        index < text.size();
        ++index
    ) {
        if (
            text[index] == '\\' &&
            index + 1 < text.size() &&
            text[index + 1] == 'n'
        ) {
            lines.push_back(
                current
            );

            current.clear();
            ++index;

            continue;
        }

        if (text[index] == '\n') {
            lines.push_back(
                current
            );

            current.clear();

            continue;
        }

        current.push_back(
            text[index]
        );
    }

    lines.push_back(
        current
    );

    return lines;
}

void setInterfaceDrawColor(
    SDL_Renderer* renderer,
    std::uint32_t color,
    std::uint8_t opacity = 0
) {
    const std::uint8_t red =
        static_cast<std::uint8_t>(
            (color >> 16) & 0xFF
        );

    const std::uint8_t green =
        static_cast<std::uint8_t>(
            (color >> 8) & 0xFF
        );

    const std::uint8_t blue =
        static_cast<std::uint8_t>(
            color & 0xFF
        );

    const std::uint8_t alpha =
        opacity == 0
            ? 255
            : static_cast<std::uint8_t>(
                std::clamp(
                    256 -
                        static_cast<int>(
                            opacity
                        ),
                    0,
                    255
                )
            );

    SDL_SetRenderDrawColor(
        renderer,
        red,
        green,
        blue,
        alpha
    );
}

std::optional<SDL_Rect> intersectRects(
    const SDL_Rect& left,
    const SDL_Rect& right
) {
    const int x1 =
        std::max(
            left.x,
            right.x
        );

    const int y1 =
        std::max(
            left.y,
            right.y
        );

    const int x2 =
        std::min(
            left.x + left.w,
            right.x + right.w
        );

    const int y2 =
        std::min(
            left.y + left.h,
            right.y + right.h
        );

    if (
        x2 <= x1 ||
        y2 <= y1
    ) {
        return std::nullopt;
    }

    return SDL_Rect{
        x1,
        y1,
        x2 - x1,
        y2 - y1
    };
}

std::string trim(
    std::string value
) {
    while (
        !value.empty() &&
        std::isspace(
            static_cast<unsigned char>(
                value.front()
            )
        )
    ) {
        value.erase(
            value.begin()
        );
    }

    while (
        !value.empty() &&
        std::isspace(
            static_cast<unsigned char>(
                value.back()
            )
        )
    ) {
        value.pop_back();
    }

    return value;
}

bool endsWith(
    const std::string& value,
    const std::string& suffix
) {
    return
        value.size() >=
            suffix.size() &&
        value.compare(
            value.size() -
                suffix.size(),
            suffix.size(),
            suffix
        ) ==
            0;
}

std::string spriteGroupName(
    std::string group
) {
    group =
        trim(
            std::move(group)
        );

    if (
        group.empty() ||
        endsWith(
            group,
            ".dat"
        )
    ) {
        return group;
    }

    return
        group +
        ".dat";
}

std::optional<SpriteRef> parseSpriteRef(
    const std::string& text
) {
    if (text.empty()) {
        return std::nullopt;
    }

    SpriteRef ref;

    const std::size_t comma =
        text.find(',');

    if (comma == std::string::npos) {
        ref.group =
            spriteGroupName(
                text
            );
    }
    else {
        ref.group =
            spriteGroupName(
                text.substr(
                    0,
                    comma
                )
            );

        try {
            const std::string frameText =
                trim(
                    text.substr(
                        comma + 1
                    )
                );

            if (!frameText.empty()) {
                const int frame =
                    std::stoi(
                        frameText
                    );

                if (
                    frame < 0 ||
                    frame >
                        std::numeric_limits<
                            std::uint16_t
                        >::max()
                ) {
                    return std::nullopt;
                }

                ref.frame =
                    static_cast<std::uint16_t>(
                        frame
                    );
            }
        }
        catch (const std::exception&) {
            return std::nullopt;
        }
    }

    if (ref.group.empty()) {
        return std::nullopt;
    }

    return ref;
}

std::string spriteKey(
    const SpriteRef& ref
) {
    return
        ref.group +
        "," +
        std::to_string(
            ref.frame
        );
}

class InterfaceSpriteCache {
public:
    InterfaceSpriteCache(
        SDL_Renderer* renderer,
        eld::sprite::SpriteRepository& repository
    )
        : renderer_(renderer),
          repository_(repository) {
    }

    ~InterfaceSpriteCache() {
        for (
            auto& [key, texture] :
            textures_
        ) {
            (void) key;

            if (texture.texture != nullptr) {
                SDL_DestroyTexture(
                    texture.texture
                );
            }
        }
    }

    SpriteTexture* find(
        const std::string& spriteText
    ) {
        const std::optional<SpriteRef> ref =
            parseSpriteRef(
                spriteText
            );

        if (!ref.has_value()) {
            return nullptr;
        }

        const std::string key =
            spriteKey(
                *ref
            );

        if (
            auto found =
                textures_.find(key);
            found != textures_.end()
        ) {
            return
                &found->second;
        }

        std::optional<eld::sprite::Sprite> sprite =
            repository_.find(
                ref->group,
                ref->frame
            );

        if (!sprite.has_value()) {
            return nullptr;
        }

        SpriteTexture texture =
            makeTexture(
                sprite->image
            );

        if (texture.texture == nullptr) {
            return nullptr;
        }

        auto inserted =
            textures_.emplace(
                key,
                texture
            );

        return
            &inserted.first->second;
    }

private:
    SpriteTexture makeTexture(
        const eld::image::Image& image
    ) const {
        const std::size_t pixelCount =
            static_cast<std::size_t>(
                image.width
            ) *
            static_cast<std::size_t>(
                image.height
            );

        if (
            image.width == 0 ||
            image.height == 0 ||
            image.pixels.size() <
                pixelCount
        ) {
            return {};
        }

        SDL_Texture* texture =
            SDL_CreateTexture(
                renderer_,
                SDL_PIXELFORMAT_RGBA32,
                SDL_TEXTUREACCESS_STATIC,
                image.width,
                image.height
            );

        if (texture == nullptr) {
            return {};
        }

        SDL_SetTextureBlendMode(
            texture,
            SDL_BLENDMODE_BLEND
        );

        SDL_SetTextureScaleMode(
            texture,
            SDL_SCALEMODE_NEAREST
        );

        if (
            !SDL_UpdateTexture(
                texture,
                nullptr,
                image.pixels.data(),
                static_cast<int>(
                    image.width *
                    sizeof(
                        eld::image::RgbaPixel
                    )
                )
            )
        ) {
            SDL_DestroyTexture(
                texture
            );

            return {};
        }

        return {
            texture,
            static_cast<int>(
                image.width
            ),
            static_cast<int>(
                image.height
            )
        };
    }

    SDL_Renderer* renderer_ = nullptr;

    eld::sprite::SpriteRepository& repository_;

    std::unordered_map<
        std::string,
        SpriteTexture
    > textures_;
};

enum class GizmoAxis : int {
    None = -1,
    X = 0,
    Y = 1,
    Z = 2
};

struct ViewportEditorInteraction {
    GizmoAxis activeAxis = GizmoAxis::None;
    bool draggingGizmo = false;
};

ViewportEditorInteraction editorInteraction;

struct ProjectedPoint {
    ImVec2 screen;
    float depth = 0.0f;
    bool valid = false;
};

bool mouseInside(
    const ImVec2& mouse,
    const ImVec2& position,
    const ImVec2& size
) {
    return
        mouse.x >= position.x &&
        mouse.y >= position.y &&
        mouse.x < position.x + size.x &&
        mouse.y < position.y + size.y;
}

float distanceToSegment(
    const ImVec2& point,
    const ImVec2& a,
    const ImVec2& b
) {
    const float abX = b.x - a.x;
    const float abY = b.y - a.y;

    const float lengthSquared =
        abX * abX +
        abY * abY;

    if (lengthSquared <= 0.0001f) {
        const float dx = point.x - a.x;
        const float dy = point.y - a.y;

        return std::sqrt(
            dx * dx +
            dy * dy
        );
    }

    float t =
        (
            (point.x - a.x) * abX +
            (point.y - a.y) * abY
        ) /
        lengthSquared;

    t = std::clamp(
        t,
        0.0f,
        1.0f
    );

    const float closestX =
        a.x +
        abX * t;

    const float closestY =
        a.y +
        abY * t;

    const float dx =
        point.x -
        closestX;

    const float dy =
        point.y -
        closestY;

    return std::sqrt(
        dx * dx +
        dy * dy
    );
}

ProjectedPoint projectWorld(
    const CacheExplorerState& state,
    const ImVec2& viewportPosition,
    const eld::math::Vec3& world
) {
    eld::render::Camera camera =
        state.camera;

    camera.viewportWidth =
        static_cast<std::uint32_t>(
            std::max(
                state.viewportWidth,
                1
            )
        );

    camera.viewportHeight =
        static_cast<std::uint32_t>(
            std::max(
                state.viewportHeight,
                1
            )
        );

    const eld::math::Mat4 view =
        eld::render::buildViewMatrix(
            camera
        );

    const eld::math::Mat4 projection =
        eld::render::buildProjectionMatrix(
            camera
        );

    const eld::render::ScreenPoint point =
        eld::render::projectPoint(
            world,
            view,
            projection,
            camera
        );

    ProjectedPoint result;
    result.screen = {
        viewportPosition.x +
            point.x,
        viewportPosition.y +
            point.y
    };

    result.depth =
        point.depth;

    result.valid =
        std::isfinite(point.x) &&
        std::isfinite(point.y) &&
        std::isfinite(point.depth) &&
        point.depth >= camera.nearPlane &&
        point.depth <= camera.farPlane;

    return result;
}

eld::math::Mat4 rotationMatrix(
    const eld::render::Transform& transform
) {
    return
        eld::math::Mat4::rotationX(
            transform.rotation.x
        ) *
        eld::math::Mat4::rotationY(
            transform.rotation.y
        ) *
        eld::math::Mat4::rotationZ(
            transform.rotation.z
        );
}

eld::math::Vec3 rotateDirection(
    const eld::render::Transform& transform,
    const eld::math::Vec3& direction
) {
    const eld::math::Vec4 transformed =
        rotationMatrix(transform).transform({
            direction.x,
            direction.y,
            direction.z,
            0.0f
        });

    return eld::math::Vec3{
        transformed.x,
        transformed.y,
        transformed.z
    }.normalized();
}

eld::math::Vec3 axisDirection(
    const eld::render::Transform& transform,
    GizmoAxis axis
) {
    switch (axis) {
        case GizmoAxis::X:
            return rotateDirection(
                transform,
                {1.0f, 0.0f, 0.0f}
            );

        case GizmoAxis::Y:
            return rotateDirection(
                transform,
                {0.0f, 1.0f, 0.0f}
            );

        case GizmoAxis::Z:
            return rotateDirection(
                transform,
                {0.0f, 0.0f, 1.0f}
            );

        case GizmoAxis::None:
            break;
    }

    return {};
}

ImU32 axisColor(
    GizmoAxis axis,
    bool active
) {
    if (active) {
        return IM_COL32(
            255,
            220,
            80,
            255
        );
    }

    switch (axis) {
        case GizmoAxis::X:
            return IM_COL32(
                235,
                80,
                80,
                255
            );

        case GizmoAxis::Y:
            return IM_COL32(
                90,
                220,
                100,
                255
            );

        case GizmoAxis::Z:
            return IM_COL32(
                80,
                150,
                245,
                255
            );

        case GizmoAxis::None:
            break;
    }

    return IM_COL32_WHITE;
}

float worldUnitsPerPixel(
    const CacheExplorerState& state,
    float depth
) {
    const float safeDepth =
        std::max(
            std::abs(depth),
            1.0f
        );

    const float height =
        static_cast<float>(
            std::max(
                state.viewportHeight,
                1
            )
        );

    return
        2.0f *
        safeDepth *
        std::tan(
            state.camera.verticalFov *
            0.5f
        ) /
        height;
}

void drawEditorGrid(
    CacheExplorerState& state,
    const ImVec2& viewportPosition,
    ImDrawList* drawList
) {
    if (
        !state.showEditorGrid ||
        !state.activeModelHandle.has_value()
    ) {
        return;
    }

    const eld::math::Mat4 model =
        eld::math::Mat4::identity();

    constexpr int HalfLines = 10;
    constexpr float Spacing = 50.0f;

    constexpr float Extent =
        HalfLines *
        Spacing;

    const auto& major =
        ui::themePalette().gridMajor;

    const auto& minor =
        ui::themePalette().gridMinor;

    const ImU32 majorColor =
        IM_COL32(
            major[0],
            major[1],
            major[2],
            major[3]
        );

    const ImU32 minorColor =
        IM_COL32(
            minor[0],
            minor[1],
            minor[2],
            minor[3]
        );

    for (
        int line = -HalfLines;
        line <= HalfLines;
        ++line
    ) {
        const float offset =
            static_cast<float>(line) *
            Spacing;

        const eld::math::Vec3 xStart =
            model.transformPoint({
                -Extent,
                0.0f,
                offset
            });

        const eld::math::Vec3 xEnd =
            model.transformPoint({
                Extent,
                0.0f,
                offset
            });

        const eld::math::Vec3 zStart =
            model.transformPoint({
                offset,
                0.0f,
                -Extent
            });

        const eld::math::Vec3 zEnd =
            model.transformPoint({
                offset,
                0.0f,
                Extent
            });

        const ProjectedPoint px0 =
            projectWorld(
                state,
                viewportPosition,
                xStart
            );

        const ProjectedPoint px1 =
            projectWorld(
                state,
                viewportPosition,
                xEnd
            );

        const ProjectedPoint pz0 =
            projectWorld(
                state,
                viewportPosition,
                zStart
            );

        const ProjectedPoint pz1 =
            projectWorld(
                state,
                viewportPosition,
                zEnd
            );

        const ImU32 lineColor =
            line == 0
                ? majorColor
                : minorColor;

        const float thickness =
            line == 0
                ? 1.25f
                : 0.8f;

        if (
            px0.valid &&
            px1.valid
        ) {
            drawList->AddLine(
                px0.screen,
                px1.screen,
                lineColor,
                thickness
            );
        }

        if (
            pz0.valid &&
            pz1.valid
        ) {
            drawList->AddLine(
                pz0.screen,
                pz1.screen,
                lineColor,
                thickness
            );
        }
    }
}

GizmoAxis closestLinearAxis(
    CacheExplorerState& state,
    const ImVec2& viewportPosition,
    const ImVec2& mouse,
    float axisLengthWorld,
    std::array<ProjectedPoint, 3>& endpoints,
    ProjectedPoint& origin
) {
    origin =
        projectWorld(
            state,
            viewportPosition,
            state.modelTransform.position
        );

    if (!origin.valid) {
        return GizmoAxis::None;
    }

    float closestDistance =
        10.0f;

    GizmoAxis closest =
        GizmoAxis::None;

    for (
        int axisIndex = 0;
        axisIndex < 3;
        ++axisIndex
    ) {
        const GizmoAxis axis =
            static_cast<GizmoAxis>(
                axisIndex
            );

        const eld::math::Vec3 direction =
            axisDirection(
                state.modelTransform,
                axis
            );

        endpoints[
            static_cast<std::size_t>(
                axisIndex
            )
        ] =
            projectWorld(
                state,
                viewportPosition,
                state.modelTransform.position +
                    direction *
                    axisLengthWorld
            );

        const ProjectedPoint& endpoint =
            endpoints[
                static_cast<std::size_t>(
                    axisIndex
                )
            ];

        if (!endpoint.valid) {
            continue;
        }

        const float distance =
            distanceToSegment(
                mouse,
                origin.screen,
                endpoint.screen
            );

        if (distance < closestDistance) {
            closestDistance =
                distance;

            closest =
                axis;
        }
    }

    return closest;
}

void drawLinearGizmo(
    CacheExplorerState& state,
    const ImVec2& viewportPosition,
    ImDrawList* drawList,
    bool scaleMode
) {
    const ImVec2 mouse =
        ImGui::GetIO().MousePos;

    const ProjectedPoint originProbe =
        projectWorld(
            state,
            viewportPosition,
            state.modelTransform.position
        );

    if (!originProbe.valid) {
        return;
    }

    const float axisLengthWorld =
        worldUnitsPerPixel(
            state,
            originProbe.depth
        ) *
        78.0f;

    std::array<ProjectedPoint, 3>
        endpoints;

    ProjectedPoint origin;

    const GizmoAxis hovered =
        closestLinearAxis(
            state,
            viewportPosition,
            mouse,
            axisLengthWorld,
            endpoints,
            origin
        );

    for (
        int axisIndex = 0;
        axisIndex < 3;
        ++axisIndex
    ) {
        const GizmoAxis axis =
            static_cast<GizmoAxis>(
                axisIndex
            );

        const ProjectedPoint& endpoint =
            endpoints[
                static_cast<std::size_t>(
                    axisIndex
                )
            ];

        if (!endpoint.valid) {
            continue;
        }

        const bool active =
            editorInteraction.activeAxis ==
                axis ||
            (
                !editorInteraction.draggingGizmo &&
                hovered == axis
            );

        drawList->AddLine(
            origin.screen,
            endpoint.screen,
            axisColor(
                axis,
                active
            ),
            active
                ? 4.0f
                : 3.0f
        );

        if (scaleMode) {
            drawList->AddRectFilled(
                {
                    endpoint.screen.x - 5.0f,
                    endpoint.screen.y - 5.0f
                },
                {
                    endpoint.screen.x + 5.0f,
                    endpoint.screen.y + 5.0f
                },
                axisColor(
                    axis,
                    active
                )
            );
        }
        else {
            drawList->AddCircleFilled(
                endpoint.screen,
                5.0f,
                axisColor(
                    axis,
                    active
                )
            );
        }
    }

    if (
        !editorInteraction.draggingGizmo &&
        ImGui::IsMouseClicked(
            ImGuiMouseButton_Left
        ) &&
        hovered != GizmoAxis::None
    ) {
        editorInteraction.activeAxis =
            hovered;

        editorInteraction.draggingGizmo =
            true;
    }

    if (
        editorInteraction.draggingGizmo &&
        !ImGui::IsMouseDown(
            ImGuiMouseButton_Left
        )
    ) {
        editorInteraction.draggingGizmo =
            false;

        editorInteraction.activeAxis =
            GizmoAxis::None;

        return;
    }

    if (
        !editorInteraction.draggingGizmo ||
        editorInteraction.activeAxis ==
            GizmoAxis::None
    ) {
        return;
    }

    const int activeIndex =
        static_cast<int>(
            editorInteraction.activeAxis
        );

    const ProjectedPoint& activeEndpoint =
        endpoints[
            static_cast<std::size_t>(
                activeIndex
            )
        ];

    const float screenX =
        activeEndpoint.screen.x -
        origin.screen.x;

    const float screenY =
        activeEndpoint.screen.y -
        origin.screen.y;

    const float screenLength =
        std::sqrt(
            screenX * screenX +
            screenY * screenY
        );

    if (screenLength <= 0.001f) {
        return;
    }

    const float normalizedX =
        screenX /
        screenLength;

    const float normalizedY =
        screenY /
        screenLength;

    const ImVec2 delta =
        ImGui::GetIO().MouseDelta;

    const float pixels =
        delta.x *
            normalizedX +
        delta.y *
            normalizedY;

    if (scaleMode) {
        const float scaleDelta =
            pixels *
            0.01f;

        switch (editorInteraction.activeAxis) {
            case GizmoAxis::X:
                state.modelTransform.scale.x =
                    std::max(
                        0.01f,
                        state.modelTransform.scale.x +
                            scaleDelta
                    );
                break;

            case GizmoAxis::Y:
                state.modelTransform.scale.y =
                    std::max(
                        0.01f,
                        state.modelTransform.scale.y +
                            scaleDelta
                    );
                break;

            case GizmoAxis::Z:
                state.modelTransform.scale.z =
                    std::max(
                        0.01f,
                        state.modelTransform.scale.z +
                            scaleDelta
                    );
                break;

            case GizmoAxis::None:
                break;
        }

        return;
    }

    const eld::math::Vec3 direction =
        axisDirection(
            state.modelTransform,
            editorInteraction.activeAxis
        );

    state.modelTransform.position =
        state.modelTransform.position +
        direction *
            (
                pixels *
                worldUnitsPerPixel(
                    state,
                    origin.depth
                )
            );
}

std::vector<ImVec2> projectedRing(
    CacheExplorerState& state,
    const ImVec2& viewportPosition,
    GizmoAxis axis,
    float radius
) {
    std::vector<ImVec2> result;

    constexpr int Segments = 64;

    result.reserve(
        Segments + 1
    );

    const eld::math::Mat4 rotation =
        rotationMatrix(
            state.modelTransform
        );

    for (
        int index = 0;
        index <= Segments;
        ++index
    ) {
        const float angle =
            static_cast<float>(
                index
            ) /
            static_cast<float>(
                Segments
            ) *
            6.28318530718f;

        eld::math::Vec3 local;

        switch (axis) {
            case GizmoAxis::X:
                local = {
                    0.0f,
                    std::cos(angle) *
                        radius,
                    std::sin(angle) *
                        radius
                };
                break;

            case GizmoAxis::Y:
                local = {
                    std::cos(angle) *
                        radius,
                    0.0f,
                    std::sin(angle) *
                        radius
                };
                break;

            case GizmoAxis::Z:
                local = {
                    std::cos(angle) *
                        radius,
                    std::sin(angle) *
                        radius,
                    0.0f
                };
                break;

            case GizmoAxis::None:
                break;
        }

        const eld::math::Vec4 rotated =
            rotation.transform({
                local.x,
                local.y,
                local.z,
                0.0f
            });

        const ProjectedPoint point =
            projectWorld(
                state,
                viewportPosition,
                state.modelTransform.position +
                    eld::math::Vec3{
                        rotated.x,
                        rotated.y,
                        rotated.z
                    }
            );

        if (point.valid) {
            result.push_back(
                point.screen
            );
        }
    }

    return result;
}

float distanceToPolyline(
    const ImVec2& point,
    const std::vector<ImVec2>& line
) {
    if (line.size() < 2) {
        return 100000.0f;
    }

    float best =
        100000.0f;

    for (
        std::size_t index = 1;
        index < line.size();
        ++index
    ) {
        best =
            std::min(
                best,
                distanceToSegment(
                    point,
                    line[index - 1],
                    line[index]
                )
            );
    }

    return best;
}

void drawRotateGizmo(
    CacheExplorerState& state,
    const ImVec2& viewportPosition,
    ImDrawList* drawList
) {
    const ProjectedPoint origin =
        projectWorld(
            state,
            viewportPosition,
            state.modelTransform.position
        );

    if (!origin.valid) {
        return;
    }

    const float radius =
        worldUnitsPerPixel(
            state,
            origin.depth
        ) *
        68.0f;

    std::array<
        std::vector<ImVec2>,
        3
    > rings;

    for (
        int axisIndex = 0;
        axisIndex < 3;
        ++axisIndex
    ) {
        rings[
            static_cast<std::size_t>(
                axisIndex
            )
        ] =
            projectedRing(
                state,
                viewportPosition,
                static_cast<GizmoAxis>(
                    axisIndex
                ),
                radius
            );
    }

    const ImVec2 mouse =
        ImGui::GetIO().MousePos;

    GizmoAxis hovered =
        GizmoAxis::None;

    float closestDistance =
        9.0f;

    for (
        int axisIndex = 0;
        axisIndex < 3;
        ++axisIndex
    ) {
        const float distance =
            distanceToPolyline(
                mouse,
                rings[
                    static_cast<std::size_t>(
                        axisIndex
                    )
                ]
            );

        if (distance < closestDistance) {
            closestDistance =
                distance;

            hovered =
                static_cast<GizmoAxis>(
                    axisIndex
                );
        }
    }

    for (
        int axisIndex = 0;
        axisIndex < 3;
        ++axisIndex
    ) {
        const GizmoAxis axis =
            static_cast<GizmoAxis>(
                axisIndex
            );

        const bool active =
            editorInteraction.activeAxis ==
                axis ||
            (
                !editorInteraction.draggingGizmo &&
                hovered == axis
            );

        const std::vector<ImVec2>& ring =
            rings[
                static_cast<std::size_t>(
                    axisIndex
                )
            ];

        for (
            std::size_t index = 1;
            index < ring.size();
            ++index
        ) {
            drawList->AddLine(
                ring[index - 1],
                ring[index],
                axisColor(
                    axis,
                    active
                ),
                active
                    ? 4.0f
                    : 2.5f
            );
        }
    }

    if (
        !editorInteraction.draggingGizmo &&
        ImGui::IsMouseClicked(
            ImGuiMouseButton_Left
        ) &&
        hovered != GizmoAxis::None
    ) {
        editorInteraction.activeAxis =
            hovered;

        editorInteraction.draggingGizmo =
            true;
    }

    if (
        editorInteraction.draggingGizmo &&
        !ImGui::IsMouseDown(
            ImGuiMouseButton_Left
        )
    ) {
        editorInteraction.draggingGizmo =
            false;

        editorInteraction.activeAxis =
            GizmoAxis::None;

        return;
    }

    if (
        !editorInteraction.draggingGizmo ||
        editorInteraction.activeAxis ==
            GizmoAxis::None
    ) {
        return;
    }

    const ImVec2 delta =
        ImGui::GetIO().MouseDelta;

    const float angleDelta =
        (
            delta.x -
            delta.y
        ) *
        0.012f;

    switch (editorInteraction.activeAxis) {
        case GizmoAxis::X:
            state.modelTransform.rotation.x +=
                angleDelta;
            break;

        case GizmoAxis::Y:
            state.modelTransform.rotation.y +=
                angleDelta;
            break;

        case GizmoAxis::Z:
            state.modelTransform.rotation.z +=
                angleDelta;
            break;

        case GizmoAxis::None:
            break;
    }
}

eld::math::Vec3 editorCameraForward(
    const CacheExplorerState& state
) {
    const float pitch =
        state.camera.rotation.x;

    const float yaw =
        state.camera.rotation.y;

    return {
        std::cos(pitch) *
            std::sin(yaw),
        -std::sin(pitch),
        std::cos(pitch) *
            std::cos(yaw)
    };
}

eld::math::Vec3 editorCameraRight(
    const CacheExplorerState& state
) {
    const float yaw =
        state.camera.rotation.y;

    return {
        std::cos(yaw),
        0.0f,
        -std::sin(yaw)
    };
}

eld::math::Vec3 editorCameraUp(
    const CacheExplorerState& state
) {
    return
        editorCameraForward(state)
            .cross(
                editorCameraRight(state)
            )
            .normalized();
}

void updateEditorCameraPosition(
    CacheExplorerState& state
) {
    state.camera.position =
        state.viewportCameraPivot -
        editorCameraForward(state) *
            state.viewportCameraDistance;
}

void resetEditorCamera(
    CacheExplorerState& state
) {
    state.viewportCameraPivot = {
        0.0f,
        0.0f,
        0.0f
    };

    state.viewportCameraDistance =
        650.0f;

    state.camera.rotation = {
        0.42f,
        -0.55f,
        0.0f
    };

    updateEditorCameraPosition(
        state
    );
}

void updateEditorNavigation(
    CacheExplorerState& state,
    const ImVec2& viewportPosition,
    const ImVec2& viewportSize
) {
    if (!state.activeModelHandle.has_value()) {
        return;
    }

    ImGuiIO& io =
        ImGui::GetIO();

    if (
        !mouseInside(
            io.MousePos,
            viewportPosition,
            viewportSize
        )
    ) {
        return;
    }

    if (
        io.WantTextInput ||
        editorInteraction.draggingGizmo
    ) {
        return;
    }

    // RMB = orbit the VIEW, not the selected model.
    if (
        ImGui::IsMouseDragging(
            ImGuiMouseButton_Right,
            1.0f
        )
    ) {
        state.camera.rotation.y +=
            io.MouseDelta.x *
            0.01f;

        state.camera.rotation.x +=
            io.MouseDelta.y *
            0.01f;

        state.camera.rotation.x =
            std::clamp(
                state.camera.rotation.x,
                -1.45f,
                1.45f
            );

        updateEditorCameraPosition(
            state
        );
    }

    // MMB = pan the editor view in camera-local X/Y while preserving orbit.
    if (
        ImGui::IsMouseDragging(
            ImGuiMouseButton_Middle,
            1.0f
        )
    ) {
        const ProjectedPoint pivot =
            projectWorld(
                state,
                viewportPosition,
                state.viewportCameraPivot
            );

        const float unitsPerPixel =
            pivot.valid
                ? worldUnitsPerPixel(
                      state,
                      pivot.depth
                  )
                : std::max(
                      state.viewportCameraDistance /
                          600.0f,
                      0.1f
                  );

        const eld::math::Vec3 right =
            editorCameraRight(state);

        const eld::math::Vec3 up =
            editorCameraUp(state);

        const eld::math::Vec3 pan =
            right *
                (
                    -io.MouseDelta.x *
                    unitsPerPixel
                ) +
            up *
                (
                    io.MouseDelta.y *
                    unitsPerPixel
                );

        state.viewportCameraPivot =
            state.viewportCameraPivot +
            pan;

        state.camera.position =
            state.camera.position +
            pan;
    }

    // Wheel = dolly camera toward/away from the orbit pivot.
    if (std::abs(io.MouseWheel) > 0.001f) {
        state.viewportCameraDistance *=
            std::pow(
                0.88f,
                io.MouseWheel
            );

        state.viewportCameraDistance =
            std::clamp(
                state.viewportCameraDistance,
                25.0f,
                5000.0f
            );

        updateEditorCameraPosition(
            state
        );
    }

    // F = focus the orbit pivot on the selected model without moving it.
    if (ImGui::IsKeyPressed(ImGuiKey_F)) {
        state.viewportCameraPivot =
            state.modelTransform.position;

        updateEditorCameraPosition(
            state
        );
    }
}

void renderViewportToolbar(
    CacheExplorerState& state,
    const ImVec2&
) {
    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameRounding,
        4.0f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_ItemSpacing,
        ImVec2(4.0f, 4.0f)
    );

    const auto modeButton =
        [&state](
            const char* label,
            ViewportGizmoMode mode
        ) {
            const bool active =
                state.viewportGizmoMode ==
                    mode;

            if (active) {
                ImGui::PushStyleColor(
                    ImGuiCol_Button,
                    ImGui::GetStyleColorVec4(
                        ImGuiCol_ButtonActive
                    )
                );
            }

            if (ImGui::SmallButton(label)) {
                state.viewportGizmoMode =
                    mode;
            }

            if (active) {
                ImGui::PopStyleColor();
            }
        };

    modeButton(
        "Move [W]",
        ViewportGizmoMode::Move
    );

    ImGui::SameLine();

    modeButton(
        "Rotate [E]",
        ViewportGizmoMode::Rotate
    );

    ImGui::SameLine();

    modeButton(
        "Scale [R]",
        ViewportGizmoMode::Scale
    );

    ImGui::SameLine();

    if (
        ImGui::SmallButton(
            state.showEditorGrid
                ? "Grid: On"
                : "Grid: Off"
        )
    ) {
        state.showEditorGrid =
            !state.showEditorGrid;
    }

    ImGui::SameLine();

    if (ImGui::SmallButton("Reset Model")) {
        state.modelTransform = {};
    }

    ImGui::SameLine();

    if (ImGui::SmallButton("Focus [F]")) {
        state.viewportCameraPivot =
            state.modelTransform.position;

        updateEditorCameraPosition(
            state
        );
    }

    ImGui::SameLine();

    if (ImGui::SmallButton("Reset View")) {
        resetEditorCamera(
            state
        );
    }

    ImGui::PopStyleVar(2);
}

void renderEditorOverlay(
    CacheExplorerState& state,
    const ImVec2& viewportPosition,
    const ImVec2& viewportSize
) {
    if (!state.activeModelHandle.has_value()) {
        return;
    }

    ImGuiIO& io =
        ImGui::GetIO();

    if (!io.WantTextInput) {
        if (ImGui::IsKeyPressed(ImGuiKey_W)) {
            state.viewportGizmoMode =
                ViewportGizmoMode::Move;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_E)) {
            state.viewportGizmoMode =
                ViewportGizmoMode::Rotate;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_R)) {
            state.viewportGizmoMode =
                ViewportGizmoMode::Scale;
        }
    }

    updateEditorNavigation(
        state,
        viewportPosition,
        viewportSize
    );

    ImDrawList* drawList =
        ImGui::GetWindowDrawList();

    drawEditorGrid(
        state,
        viewportPosition,
        drawList
    );

    if (state.showViewportGizmo) {
        switch (state.viewportGizmoMode) {
            case ViewportGizmoMode::Move:
                drawLinearGizmo(
                    state,
                    viewportPosition,
                    drawList,
                    false
                );
                break;

            case ViewportGizmoMode::Rotate:
                drawRotateGizmo(
                    state,
                    viewportPosition,
                    drawList
                );
                break;

            case ViewportGizmoMode::Scale:
                drawLinearGizmo(
                    state,
                    viewportPosition,
                    drawList,
                    true
                );
                break;
        }
    }

}


void setAxisDrawColor(
    SDL_Renderer* renderer,
    GizmoAxis axis,
    bool active
) {
    if (active) {
        SDL_SetRenderDrawColor(
            renderer,
            255,
            220,
            80,
            255
        );
        return;
    }

    switch (axis) {
        case GizmoAxis::X:
            SDL_SetRenderDrawColor(
                renderer,
                235,
                80,
                80,
                255
            );
            break;

        case GizmoAxis::Y:
            SDL_SetRenderDrawColor(
                renderer,
                90,
                220,
                100,
                255
            );
            break;

        case GizmoAxis::Z:
            SDL_SetRenderDrawColor(
                renderer,
                80,
                150,
                245,
                255
            );
            break;

        case GizmoAxis::None:
            SDL_SetRenderDrawColor(
                renderer,
                255,
                255,
                255,
                255
            );
            break;
    }
}

void drawEditorGridSdl(
    SDL_Renderer* renderer,
    CacheExplorerState& state,
    const ImVec2& viewportPosition
) {
    if (
        renderer == nullptr ||
        !state.showEditorGrid ||
        !state.activeModelHandle.has_value()
    ) {
        return;
    }

    // The editor grid is world space, not part of the selected model.
    // Moving/rotating/scaling the model must not drag the floor with it.
    const eld::math::Mat4 model =
        eld::math::Mat4::identity();

    constexpr int HalfLines = 10;
    constexpr float Spacing = 50.0f;
    constexpr float Extent =
        HalfLines *
        Spacing;

    for (
        int line = -HalfLines;
        line <= HalfLines;
        ++line
    ) {
        const float offset =
            static_cast<float>(line) *
            Spacing;

        const eld::math::Vec3 xStart =
            model.transformPoint({
                -Extent,
                0.0f,
                offset
            });

        const eld::math::Vec3 xEnd =
            model.transformPoint({
                Extent,
                0.0f,
                offset
            });

        const eld::math::Vec3 zStart =
            model.transformPoint({
                offset,
                0.0f,
                -Extent
            });

        const eld::math::Vec3 zEnd =
            model.transformPoint({
                offset,
                0.0f,
                Extent
            });

        const ProjectedPoint px0 =
            projectWorld(
                state,
                viewportPosition,
                xStart
            );

        const ProjectedPoint px1 =
            projectWorld(
                state,
                viewportPosition,
                xEnd
            );

        const ProjectedPoint pz0 =
            projectWorld(
                state,
                viewportPosition,
                zStart
            );

        const ProjectedPoint pz1 =
            projectWorld(
                state,
                viewportPosition,
                zEnd
            );

        const auto& gridColor =
            line == 0
                ? ui::themePalette().
                    gridMajor
                : ui::themePalette().
                    gridMinor;

        SDL_SetRenderDrawColor(
            renderer,
            gridColor[0],
            gridColor[1],
            gridColor[2],
            gridColor[3]
        );

        if (px0.valid && px1.valid) {
            SDL_RenderLine(
                renderer,
                px0.screen.x,
                px0.screen.y,
                px1.screen.x,
                px1.screen.y
            );
        }

        if (pz0.valid && pz1.valid) {
            SDL_RenderLine(
                renderer,
                pz0.screen.x,
                pz0.screen.y,
                pz1.screen.x,
                pz1.screen.y
            );
        }
    }
}

void drawLinearGizmoSdl(
    SDL_Renderer* renderer,
    CacheExplorerState& state,
    const ImVec2& viewportPosition,
    bool scaleMode
) {
    if (
        renderer == nullptr ||
        !state.showViewportGizmo
    ) {
        return;
    }

    const ProjectedPoint originProbe =
        projectWorld(
            state,
            viewportPosition,
            state.modelTransform.position
        );

    if (!originProbe.valid) {
        return;
    }

    const float axisLengthWorld =
        worldUnitsPerPixel(
            state,
            originProbe.depth
        ) *
        78.0f;

    std::array<ProjectedPoint, 3>
        endpoints;

    ProjectedPoint origin;

    const ImVec2 mouse =
        ImGui::GetIO().MousePos;

    const GizmoAxis hovered =
        closestLinearAxis(
            state,
            viewportPosition,
            mouse,
            axisLengthWorld,
            endpoints,
            origin
        );

    for (
        int axisIndex = 0;
        axisIndex < 3;
        ++axisIndex
    ) {
        const GizmoAxis axis =
            static_cast<GizmoAxis>(
                axisIndex
            );

        const ProjectedPoint& endpoint =
            endpoints[
                static_cast<std::size_t>(
                    axisIndex
                )
            ];

        if (!endpoint.valid) {
            continue;
        }

        const bool active =
            editorInteraction.activeAxis ==
                axis ||
            (
                !editorInteraction.draggingGizmo &&
                hovered == axis
            );

        setAxisDrawColor(
            renderer,
            axis,
            active
        );

        SDL_RenderLine(
            renderer,
            origin.screen.x,
            origin.screen.y,
            endpoint.screen.x,
            endpoint.screen.y
        );

        const float halfSize =
            scaleMode
                ? 5.0f
                : 4.0f;

        const SDL_FRect handle{
            endpoint.screen.x - halfSize,
            endpoint.screen.y - halfSize,
            halfSize * 2.0f,
            halfSize * 2.0f
        };

        SDL_RenderFillRect(
            renderer,
            &handle
        );
    }
}

void drawRotateGizmoSdl(
    SDL_Renderer* renderer,
    CacheExplorerState& state,
    const ImVec2& viewportPosition
) {
    if (
        renderer == nullptr ||
        !state.showViewportGizmo
    ) {
        return;
    }

    const ProjectedPoint origin =
        projectWorld(
            state,
            viewportPosition,
            state.modelTransform.position
        );

    if (!origin.valid) {
        return;
    }

    const float radius =
        worldUnitsPerPixel(
            state,
            origin.depth
        ) *
        68.0f;

    std::array<
        std::vector<ImVec2>,
        3
    > rings;

    for (
        int axisIndex = 0;
        axisIndex < 3;
        ++axisIndex
    ) {
        rings[
            static_cast<std::size_t>(
                axisIndex
            )
        ] =
            projectedRing(
                state,
                viewportPosition,
                static_cast<GizmoAxis>(
                    axisIndex
                ),
                radius
            );
    }

    const ImVec2 mouse =
        ImGui::GetIO().MousePos;

    GizmoAxis hovered =
        GizmoAxis::None;

    float closestDistance =
        9.0f;

    for (
        int axisIndex = 0;
        axisIndex < 3;
        ++axisIndex
    ) {
        const float distance =
            distanceToPolyline(
                mouse,
                rings[
                    static_cast<std::size_t>(
                        axisIndex
                    )
                ]
            );

        if (distance < closestDistance) {
            closestDistance =
                distance;

            hovered =
                static_cast<GizmoAxis>(
                    axisIndex
                );
        }
    }

    for (
        int axisIndex = 0;
        axisIndex < 3;
        ++axisIndex
    ) {
        const GizmoAxis axis =
            static_cast<GizmoAxis>(
                axisIndex
            );

        const bool active =
            editorInteraction.activeAxis ==
                axis ||
            (
                !editorInteraction.draggingGizmo &&
                hovered == axis
            );

        setAxisDrawColor(
            renderer,
            axis,
            active
        );

        const std::vector<ImVec2>& ring =
            rings[
                static_cast<std::size_t>(
                    axisIndex
                )
            ];

        for (
            std::size_t index = 1;
            index < ring.size();
            ++index
        ) {
            SDL_RenderLine(
                renderer,
                ring[index - 1].x,
                ring[index - 1].y,
                ring[index].x,
                ring[index].y
            );
        }
    }
}

void drawEditorOverlaySdl(
    SDL_Renderer* renderer,
    CacheExplorerState& state
) {
    if (
        renderer == nullptr ||
        !state.activeModelHandle.has_value()
    ) {
        return;
    }

    const SDL_Rect clip{
        state.viewportX,
        state.viewportY,
        state.viewportWidth,
        state.viewportHeight
    };

    SDL_SetRenderClipRect(
        renderer,
        &clip
    );

    const ImVec2 viewportPosition{
        static_cast<float>(
            state.viewportX
        ),
        static_cast<float>(
            state.viewportY
        )
    };

    drawEditorGridSdl(
        renderer,
        state,
        viewportPosition
    );

    if (state.showViewportGizmo) {
        switch (state.viewportGizmoMode) {
            case ViewportGizmoMode::Move:
                drawLinearGizmoSdl(
                    renderer,
                    state,
                    viewportPosition,
                    false
                );
                break;

            case ViewportGizmoMode::Rotate:
                drawRotateGizmoSdl(
                    renderer,
                    state,
                    viewportPosition
                );
                break;

            case ViewportGizmoMode::Scale:
                drawLinearGizmoSdl(
                    renderer,
                    state,
                    viewportPosition,
                    true
                );
                break;
        }
    }

    SDL_SetRenderClipRect(
        renderer,
        nullptr
    );
}

void renderCheckerboard(
    SDL_Renderer* renderer,
    const CacheExplorerState& state,
    int cellSize = 16
) {
    cellSize =
        std::max(
            cellSize,
            1
        );

    for (
        int y = 0;
        y < state.viewportHeight;
        y += cellSize
    ) {
        for (
            int x = 0;
            x < state.viewportWidth;
            x += cellSize
        ) {
            const bool light =
                (
                    x / cellSize +
                    y / cellSize
                ) %
                2 ==
                0;

            const std::uint8_t color =
                light
                    ? 180
                    : 130;

            SDL_SetRenderDrawColor(
                renderer,
                color,
                color,
                color,
                255
            );

            const SDL_FRect cell{
                static_cast<float>(
                    state.viewportX + x
                ),
                static_cast<float>(
                    state.viewportY + y
                ),
                static_cast<float>(
                    std::min(
                        cellSize,
                        state.viewportWidth - x
                    )
                ),
                static_cast<float>(
                    std::min(
                        cellSize,
                        state.viewportHeight - y
                    )
                )
            };

            SDL_RenderFillRect(
                renderer,
                &cell
            );
        }
    }
}

void renderImage(
    SDL_Renderer* renderer,
    const CacheExplorerState& state,
    const eld::image::Image& image,
    const TextureViewOptions* textureOptions = nullptr
) {
    if (
        image.width == 0 ||
        image.height == 0 ||
        image.pixels.empty()
    ) {
        return;
    }

    const SDL_Rect clip{
        state.viewportX,
        state.viewportY,
        state.viewportWidth,
        state.viewportHeight
    };

    SDL_SetRenderClipRect(
        renderer,
        &clip
    );

    if (
        textureOptions == nullptr ||
        textureOptions->showCheckerboard
    ) {
        renderCheckerboard(
            renderer,
            state,
            textureOptions == nullptr
                ? 16
                : textureOptions->checkerSize
        );
    }
    else {
        const auto& canvasColor =
            ui::themePalette().
                viewportBackground;

        SDL_SetRenderDrawColor(
            renderer,
            canvasColor[0],
            canvasColor[1],
            canvasColor[2],
            canvasColor[3]
        );

        const SDL_FRect background{
            static_cast<float>(
                state.viewportX
            ),
            static_cast<float>(
                state.viewportY
            ),
            static_cast<float>(
                state.viewportWidth
            ),
            static_cast<float>(
                state.viewportHeight
            )
        };

        SDL_RenderFillRect(
            renderer,
            &background
        );
    }

    const eld::image::RgbaPixel* pixelData =
        image.pixels.data();

    std::vector<eld::image::RgbaPixel>
        filteredPixels;

    if (textureOptions != nullptr) {
        filteredPixels =
            image.pixels;

        for (
            eld::image::RgbaPixel& pixel :
            filteredPixels
        ) {
            if (textureOptions->alphaOnly) {
                const std::uint8_t alpha =
                    pixel.alpha;

                pixel.red = alpha;
                pixel.green = alpha;
                pixel.blue = alpha;
                pixel.alpha = 255;

                continue;
            }

            if (!textureOptions->showRed) {
                pixel.red = 0;
            }

            if (!textureOptions->showGreen) {
                pixel.green = 0;
            }

            if (!textureOptions->showBlue) {
                pixel.blue = 0;
            }

            if (!textureOptions->showAlpha) {
                pixel.alpha = 255;
            }
        }

        pixelData =
            filteredPixels.data();
    }

    SDL_Texture* texture =
        SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STATIC,
            image.width,
            image.height
        );

    if (texture == nullptr) {
        SDL_SetRenderClipRect(
            renderer,
            nullptr
        );

        return;
    }

    SDL_UpdateTexture(
        texture,
        nullptr,
        pixelData,
        static_cast<int>(
            image.width *
            sizeof(
                eld::image::RgbaPixel
            )
        )
    );

    SDL_SetTextureBlendMode(
        texture,
        SDL_BLENDMODE_BLEND
    );

    SDL_SetTextureScaleMode(
        texture,
        textureOptions != nullptr &&
            !textureOptions->nearestSampling
            ? SDL_SCALEMODE_LINEAR
            : SDL_SCALEMODE_NEAREST
    );

    float scale = 1.0f;

    if (
        textureOptions == nullptr ||
        textureOptions->zoomMode ==
            TextureZoomMode::Fit
    ) {
        scale =
            std::min(
                static_cast<float>(
                    state.viewportWidth
                ) /
                    image.width,
                static_cast<float>(
                    state.viewportHeight
                ) /
                    image.height
            );

        if (scale >= 1.0f) {
            scale =
                std::floor(
                    scale
                );
        }

        scale =
            std::max(
                scale,
                0.01f
            );
    }
    else {
        scale =
            textureOptions->fixedScale();
    }

    const float width =
        image.width *
        scale;

    const float height =
        image.height *
        scale;

    const SDL_FRect destination{
        state.viewportX +
            (
                state.viewportWidth -
                width
            ) /
            2.0f,
        state.viewportY +
            (
                state.viewportHeight -
                height
            ) /
            2.0f,
        width,
        height
    };

    SDL_RenderTexture(
        renderer,
        texture,
        nullptr,
        &destination
    );

    if (
        textureOptions != nullptr &&
        textureOptions->showPixelGrid &&
        scale >= 6.0f
    ) {
        SDL_SetRenderDrawColor(
            renderer,
            255,
            255,
            255,
            55
        );

        SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_BLEND
        );

        for (
            std::uint32_t x = 0;
            x <= image.width;
            ++x
        ) {
            const float lineX =
                destination.x +
                static_cast<float>(x) *
                scale;

            SDL_RenderLine(
                renderer,
                lineX,
                destination.y,
                lineX,
                destination.y +
                    destination.h
            );
        }

        for (
            std::uint32_t y = 0;
            y <= image.height;
            ++y
        ) {
            const float lineY =
                destination.y +
                static_cast<float>(y) *
                scale;

            SDL_RenderLine(
                renderer,
                destination.x,
                lineY,
                destination.x +
                    destination.w,
                lineY
            );
        }

        SDL_SetRenderDrawBlendMode(
            renderer,
            SDL_BLENDMODE_NONE
        );
    }

    if (
        textureOptions != nullptr &&
        textureOptions->showBorder
    ) {
        SDL_SetRenderDrawColor(
            renderer,
            235,
            235,
            235,
            210
        );

        SDL_RenderRect(
            renderer,
            &destination
        );
    }

    SDL_DestroyTexture(
        texture
    );

    SDL_SetRenderClipRect(
        renderer,
        nullptr
    );
}

struct ModelDebugPoint {
    float x = 0.0f;
    float y = 0.0f;
    float depth = 0.0f;
    bool visible = false;
};

eld::math::Vec3 toGraphicsPosition(
    const eld::model::Vertex& vertex
) {
    return {
        vertex.x,
        -vertex.y,
        vertex.z
    };
}

ModelDebugPoint projectModelPoint(
    const eld::math::Vec3& point,
    const CacheExplorerState& state,
    const eld::math::Mat4& modelMatrix,
    const eld::math::Mat4& viewMatrix,
    const eld::math::Mat4& projectionMatrix
) {
    const eld::math::Vec3 worldPoint =
        modelMatrix.transformPoint(
            point
        );

    const eld::render::ScreenPoint projected =
        eld::render::projectPoint(
            worldPoint,
            viewMatrix,
            projectionMatrix,
            state.camera
        );

    return {
        static_cast<float>(
            state.viewportX
        ) +
            projected.x,
        static_cast<float>(
            state.viewportY
        ) +
            projected.y,
        projected.depth,
        std::isfinite(
            projected.x
        ) &&
            std::isfinite(
                projected.y
            ) &&
            projected.depth >
                state.camera.nearPlane &&
            projected.depth <
                state.camera.farPlane
    };
}

std::vector<ModelDebugPoint>
projectModelVertices(
    const eld::model::ModelMesh& mesh,
    const CacheExplorerState& state
) {
    const eld::math::Mat4 modelMatrix =
        eld::render::buildModelMatrix(
            state.modelTransform
        );

    const eld::math::Mat4 viewMatrix =
        eld::render::buildViewMatrix(
            state.camera
        );

    const eld::math::Mat4 projectionMatrix =
        eld::render::buildProjectionMatrix(
            state.camera
        );

    std::vector<ModelDebugPoint> result;

    result.reserve(
        mesh.vertices.size()
    );

    for (
        const eld::model::Vertex& vertex :
        mesh.vertices
    ) {
        result.push_back(
            projectModelPoint(
                toGraphicsPosition(
                    vertex
                ),
                state,
                modelMatrix,
                viewMatrix,
                projectionMatrix
            )
        );
    }

    return result;
}

void renderModelWireframe(
    SDL_Renderer* renderer,
    const eld::model::ModelMesh& mesh,
    const CacheExplorerState& state
) {
    const std::vector<ModelDebugPoint> points =
        projectModelVertices(
            mesh,
            state
        );

    SDL_SetRenderDrawBlendMode(
        renderer,
        SDL_BLENDMODE_BLEND
    );

    SDL_SetRenderDrawColor(
        renderer,
        245,
        245,
        245,
        155
    );

    std::size_t drawn = 0;

    for (
        const eld::model::Face& face :
        mesh.faces
    ) {
        if (
            face.a >= points.size() ||
            face.b >= points.size() ||
            face.c >= points.size()
        ) {
            continue;
        }

        const ModelDebugPoint& a =
            points[face.a];

        const ModelDebugPoint& b =
            points[face.b];

        const ModelDebugPoint& c =
            points[face.c];

        if (
            !a.visible ||
            !b.visible ||
            !c.visible
        ) {
            continue;
        }

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

        if (++drawn >= 6000) {
            break;
        }
    }

    SDL_SetRenderDrawBlendMode(
        renderer,
        SDL_BLENDMODE_NONE
    );
}

void renderModelVertices(
    SDL_Renderer* renderer,
    const eld::model::ModelMesh& mesh,
    const CacheExplorerState& state
) {
    const std::vector<ModelDebugPoint> points =
        projectModelVertices(
            mesh,
            state
        );

    SDL_SetRenderDrawColor(
        renderer,
        255,
        210,
        70,
        255
    );

    for (
        const ModelDebugPoint& point :
        points
    ) {
        if (!point.visible) {
            continue;
        }

        const SDL_FRect marker{
            point.x - 1.5f,
            point.y - 1.5f,
            3.0f,
            3.0f
        };

        SDL_RenderFillRect(
            renderer,
            &marker
        );
    }
}

void renderModelBounds(
    SDL_Renderer* renderer,
    const eld::model::ModelMesh& mesh,
    const CacheExplorerState& state
) {
    if (mesh.vertices.empty()) {
        return;
    }

    eld::math::Vec3 minimum =
        toGraphicsPosition(
            mesh.vertices.front()
        );

    eld::math::Vec3 maximum =
        minimum;

    for (
        const eld::model::Vertex& vertex :
        mesh.vertices
    ) {
        const eld::math::Vec3 point =
            toGraphicsPosition(
                vertex
            );

        minimum.x =
            std::min(
                minimum.x,
                point.x
            );

        minimum.y =
            std::min(
                minimum.y,
                point.y
            );

        minimum.z =
            std::min(
                minimum.z,
                point.z
            );

        maximum.x =
            std::max(
                maximum.x,
                point.x
            );

        maximum.y =
            std::max(
                maximum.y,
                point.y
            );

        maximum.z =
            std::max(
                maximum.z,
                point.z
            );
    }

    const eld::math::Mat4 modelMatrix =
        eld::render::buildModelMatrix(
            state.modelTransform
        );

    const eld::math::Mat4 viewMatrix =
        eld::render::buildViewMatrix(
            state.camera
        );

    const eld::math::Mat4 projectionMatrix =
        eld::render::buildProjectionMatrix(
            state.camera
        );

    const std::array<eld::math::Vec3, 8> corners{
        eld::math::Vec3{
            minimum.x,
            minimum.y,
            minimum.z
        },
        eld::math::Vec3{
            maximum.x,
            minimum.y,
            minimum.z
        },
        eld::math::Vec3{
            maximum.x,
            maximum.y,
            minimum.z
        },
        eld::math::Vec3{
            minimum.x,
            maximum.y,
            minimum.z
        },
        eld::math::Vec3{
            minimum.x,
            minimum.y,
            maximum.z
        },
        eld::math::Vec3{
            maximum.x,
            minimum.y,
            maximum.z
        },
        eld::math::Vec3{
            maximum.x,
            maximum.y,
            maximum.z
        },
        eld::math::Vec3{
            minimum.x,
            maximum.y,
            maximum.z
        }
    };

    std::array<ModelDebugPoint, 8> projected;

    for (
        std::size_t index = 0;
        index < corners.size();
        ++index
    ) {
        projected[index] =
            projectModelPoint(
                corners[index],
                state,
                modelMatrix,
                viewMatrix,
                projectionMatrix
            );
    }

    static const std::array<
        std::array<int, 2>,
        12
    > Edges{{
        {{0, 1}},
        {{1, 2}},
        {{2, 3}},
        {{3, 0}},
        {{4, 5}},
        {{5, 6}},
        {{6, 7}},
        {{7, 4}},
        {{0, 4}},
        {{1, 5}},
        {{2, 6}},
        {{3, 7}}
    }};

    SDL_SetRenderDrawColor(
        renderer,
        80,
        220,
        255,
        255
    );

    for (
        const auto& edge :
        Edges
    ) {
        const ModelDebugPoint& a =
            projected[
                static_cast<std::size_t>(
                    edge[0]
                )
            ];

        const ModelDebugPoint& b =
            projected[
                static_cast<std::size_t>(
                    edge[1]
                )
            ];

        if (
            !a.visible ||
            !b.visible
        ) {
            continue;
        }

        SDL_RenderLine(
            renderer,
            a.x,
            a.y,
            b.x,
            b.y
        );
    }
}

void renderModelAxes(
    SDL_Renderer* renderer,
    const eld::model::ModelMesh& mesh,
    const CacheExplorerState& state
) {
    if (mesh.vertices.empty()) {
        return;
    }

    float maximumExtent = 1.0f;

    for (
        const eld::model::Vertex& vertex :
        mesh.vertices
    ) {
        maximumExtent =
            std::max(
                maximumExtent,
                std::abs(
                    vertex.x
                )
            );

        maximumExtent =
            std::max(
                maximumExtent,
                std::abs(
                    vertex.y
                )
            );

        maximumExtent =
            std::max(
                maximumExtent,
                std::abs(
                    vertex.z
                )
            );
    }

    const float axisLength =
        maximumExtent *
        0.45f;

    const eld::math::Mat4 modelMatrix =
        eld::render::buildModelMatrix(
            state.modelTransform
        );

    const eld::math::Mat4 viewMatrix =
        eld::render::buildViewMatrix(
            state.camera
        );

    const eld::math::Mat4 projectionMatrix =
        eld::render::buildProjectionMatrix(
            state.camera
        );

    const ModelDebugPoint origin =
        projectModelPoint(
            {0.0f, 0.0f, 0.0f},
            state,
            modelMatrix,
            viewMatrix,
            projectionMatrix
        );

    if (!origin.visible) {
        return;
    }

    struct Axis {
        eld::math::Vec3 point;
        std::array<std::uint8_t, 3>
            color;
    };

    const std::array<Axis, 3> axes{{
        {
            {
                axisLength,
                0.0f,
                0.0f
            },
            {
                240,
                90,
                90
            }
        },
        {
            {
                0.0f,
                axisLength,
                0.0f
            },
            {
                90,
                235,
                120
            }
        },
        {
            {
                0.0f,
                0.0f,
                axisLength
            },
            {
                100,
                155,
                245
            }
        }
    }};

    for (
        const Axis& axis :
        axes
    ) {
        const ModelDebugPoint end =
            projectModelPoint(
                axis.point,
                state,
                modelMatrix,
                viewMatrix,
                projectionMatrix
            );

        if (!end.visible) {
            continue;
        }

        SDL_SetRenderDrawColor(
            renderer,
            axis.color[0],
            axis.color[1],
            axis.color[2],
            255
        );

        SDL_RenderLine(
            renderer,
            origin.x,
            origin.y,
            end.x,
            end.y
        );
    }
}

void renderModelOverlays(
    SDL_Renderer* renderer,
    const eld::model::ModelMesh& mesh,
    const CacheExplorerState& state,
    const ModelViewOptions& options
) {
    if (options.showWireframe) {
        renderModelWireframe(
            renderer,
            mesh,
            state
        );
    }

    if (options.showVertices) {
        renderModelVertices(
            renderer,
            mesh,
            state
        );
    }

    if (options.showBounds) {
        renderModelBounds(
            renderer,
            mesh,
            state
        );
    }

    if (options.showAxes) {
        renderModelAxes(
            renderer,
            mesh,
            state
        );
    }
}

void renderInterfaceText(
    SDL_Renderer* renderer,
    const eld::interface::InterfaceFileWidget& widget,
    int x,
    int y,
    int width
) {
    const std::string& text =
        widget.text.empty()
            ? widget.secondaryText
            : widget.text;

    if (text.empty()) {
        return;
    }

    int lineY = y;

    for (
        const std::string& line :
        splitText(text)
    ) {
        int lineX = x;

        if (widget.centeredText) {
            const int textWidth =
                static_cast<int>(
                    line.size()
                ) *
                DebugFontWidth;

            lineX =
                x +
                (
                    width -
                    textWidth
                ) /
                2;
        }

        if (widget.textShadow) {
            SDL_SetRenderDrawColor(
                renderer,
                0,
                0,
                0,
                255
            );

            SDL_RenderDebugText(
                renderer,
                static_cast<float>(
                    lineX + 1
                ),
                static_cast<float>(
                    lineY + 1
                ),
                line.c_str()
            );
        }

        setInterfaceDrawColor(
            renderer,
            widget.color
        );

        SDL_RenderDebugText(
            renderer,
            static_cast<float>(
                lineX
            ),
            static_cast<float>(
                lineY
            ),
            line.c_str()
        );

        lineY +=
            DebugFontHeight;
    }
}

void renderInterfaceSprite(
    SDL_Renderer* renderer,
    const eld::interface::InterfaceFileWidget& widget,
    int x,
    int y,
    InterfaceSpriteCache& spriteCache
) {
    const std::string& spriteText =
        widget.sprite.empty()
            ? widget.secondarySprite
            : widget.sprite;

    SpriteTexture* texture =
        spriteCache.find(
            spriteText
        );

    if (
        texture == nullptr ||
        texture->texture == nullptr
    ) {
        return;
    }

    const SDL_FRect destination{
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(
            texture->width
        ),
        static_cast<float>(
            texture->height
        )
    };

    SDL_RenderTexture(
        renderer,
        texture->texture,
        nullptr,
        &destination
    );
}

void renderInterfaceInventory(
    SDL_Renderer* renderer,
    const eld::interface::InterfaceFileWidget& widget,
    int x,
    int y,
    InterfaceSpriteCache& spriteCache
) {
    for (
        const eld::interface::InterfaceFileSpriteSlot& slot :
        widget.inventorySprites
    ) {
        SpriteTexture* texture =
            spriteCache.find(
                slot.sprite
            );

        if (
            texture == nullptr ||
            texture->texture == nullptr
        ) {
            continue;
        }

        const SDL_FRect destination{
            static_cast<float>(
                x + slot.x
            ),
            static_cast<float>(
                y + slot.y
            ),
            static_cast<float>(
                texture->width
            ),
            static_cast<float>(
                texture->height
            )
        };

        SDL_RenderTexture(
            renderer,
            texture->texture,
            nullptr,
            &destination
        );
    }

    const int columns =
        static_cast<int>(
            widget.width
        );

    const int rows =
        static_cast<int>(
            widget.height
        );

    for (
        int row = 0;
        row < rows;
        ++row
    ) {
        for (
            int column = 0;
            column < columns;
            ++column
        ) {
            const int slot =
                row *
                    columns +
                column;

            if (
                slot < 0 ||
                static_cast<std::size_t>(
                    slot
                ) >=
                    widget.itemIds.size() ||
                widget.itemIds[
                    static_cast<std::size_t>(
                        slot
                    )
                ] ==
                    0
            ) {
                continue;
            }

            const int slotX =
                x +
                column *
                    (
                        InterfaceSlotSize +
                        static_cast<int>(
                            widget.inventoryPaddingX
                        )
                    );

            const int slotY =
                y +
                row *
                    (
                        InterfaceSlotSize +
                        static_cast<int>(
                            widget.inventoryPaddingY
                        )
                    );

            SDL_SetRenderDrawColor(
                renderer,
                255,
                170,
                55,
                80
            );

            const SDL_FRect rect{
                static_cast<float>(
                    slotX
                ),
                static_cast<float>(
                    slotY
                ),
                static_cast<float>(
                    InterfaceSlotSize
                ),
                static_cast<float>(
                    InterfaceSlotSize
                )
            };

            SDL_RenderRect(
                renderer,
                &rect
            );

            const std::string itemText =
                std::to_string(
                    widget.itemIds[
                        static_cast<std::size_t>(
                            slot
                        )
                    ]
                );

            SDL_SetRenderDrawColor(
                renderer,
                255,
                255,
                255,
                255
            );

            SDL_RenderDebugText(
                renderer,
                static_cast<float>(
                    slotX + 2
                ),
                static_cast<float>(
                    slotY + 2
                ),
                itemText.c_str()
            );
        }
    }
}

void renderInterfaceItemList(
    SDL_Renderer* renderer,
    const eld::interface::InterfaceFileWidget& widget,
    int x,
    int y
) {
    const int columns =
        static_cast<int>(
            widget.width
        );

    const int rows =
        static_cast<int>(
            widget.height
        );

    for (
        int row = 0;
        row < rows;
        ++row
    ) {
        for (
            int column = 0;
            column < columns;
            ++column
        ) {
            const int slot =
                row *
                    columns +
                column;

            if (
                slot < 0 ||
                static_cast<std::size_t>(
                    slot
                ) >=
                    widget.itemIds.size() ||
                widget.itemIds[
                    static_cast<std::size_t>(
                        slot
                    )
                ] ==
                    0
            ) {
                continue;
            }

            const int itemX =
                x +
                column *
                    (
                        InterfaceSlotSize +
                        static_cast<int>(
                            widget.itemPaddingX
                        )
                    );

            const int itemY =
                y +
                row *
                    (
                        InterfaceSlotSize +
                        static_cast<int>(
                            widget.itemPaddingY
                        )
                    );

            const std::string text =
                std::to_string(
                    widget.itemIds[
                        static_cast<std::size_t>(
                            slot
                        )
                    ]
                );

            setInterfaceDrawColor(
                renderer,
                widget.color
            );

            SDL_RenderDebugText(
                renderer,
                static_cast<float>(
                    itemX
                ),
                static_cast<float>(
                    itemY
                ),
                text.c_str()
            );
        }
    }
}

float interfaceVerticalFov(
    float focalLength
) {
    return
        2.0f *
        std::atan(
            static_cast<float>(
                InterfaceModelRenderTargetSize
            ) /
            (
                2.0f *
                focalLength
            )
        );
}

void renderInterfaceModel(
    SDL_Renderer* renderer,
    const InterfaceViewState& viewState,
    const InterfaceViewNode& node,
    int x,
    int y,
    eld::graphics::GraphicsResources& resources
) {
    if (!node.model.has_value()) {
        return;
    }

    try {
        const eld::graphics::ModelHandle handle =
            resources.resolveModel(
                node.model->modelId
            );

        eld::render::RenderObject object;

        object.model =
            handle;

        object.transform.rotation =
            node.model->rotation;

        object.transform.position = {
            0.0f,
            0.0f,
            node.model->depth
        };

        eld::render::RenderScene scene;

        scene.camera.position = {
            0.0f,
            0.0f,
            0.0f
        };

        scene.camera.rotation = {
            0.0f,
            0.0f,
            0.0f
        };

        scene.camera.verticalFov =
            interfaceVerticalFov(
                viewState.modelProjectionFocalLength
            );

        scene.camera.nearPlane = 1.0f;
        scene.camera.farPlane = 10000.0f;

        scene.camera.viewportWidth =
            InterfaceModelRenderTargetSize;

        scene.camera.viewportHeight =
            InterfaceModelRenderTargetSize;

        scene.objects.push_back(
            object
        );

        const PixelSize size =
            widgetPixelSize(
                node.widget
            );

        eld::render::SoftwareRenderBackend backend(
            renderer
        );

        backend.setOutputPosition(
            x +
                size.width / 2 -
                InterfaceModelRenderTargetSize / 2,
            y +
                size.height / 2 -
                InterfaceModelRenderTargetSize / 2
        );

        backend.setClearColor({
            0,
            0,
            0,
            0
        });

        eld::render::RenderPipeline pipeline;

        pipeline.render(
            scene,
            resources,
            backend
        );
    }
    catch (const std::exception&) {
        return;
    }
}

const char* interfaceWidgetTypeName(
    std::uint8_t type
) {
    switch (type) {
        case 0:
            return "container";

        case 1:
            return "unused";

        case 2:
            return "inventory";

        case 3:
            return "rectangle";

        case 4:
            return "text";

        case 5:
            return "sprite";

        case 6:
            return "model";

        case 7:
            return "item-list";

        case 8:
            return "tooltip";

        default:
            return "unknown";
    }
}

void renderInterfaceNode(
    SDL_Renderer* renderer,
    const InterfaceViewState& viewState,
    const InterfaceViewNode& node,
    int parentX,
    int parentY,
    const SDL_Rect& clip,
    InterfaceSpriteCache& spriteCache,
    eld::graphics::GraphicsResources& resources,
    const InterfaceViewOptions& options
) {
    if (
        node.widget.hidden &&
        !options.showHiddenWidgets
    ) {
        return;
    }

    const int x =
        parentX +
        node.x;

    const int y =
        parentY +
        node.y;

    const PixelSize size =
        widgetPixelSize(
            node.widget
        );

    SDL_SetRenderClipRect(
        renderer,
        &clip
    );

    switch (node.widget.type) {
        case 2:
            if (options.showInventories) {
                renderInterfaceInventory(
                    renderer,
                    node.widget,
                    x,
                    y,
                    spriteCache
                );
            }
            break;

        case 3: {
            if (!options.showRectangles) {
                break;
            }

            setInterfaceDrawColor(
                renderer,
                node.widget.color,
                node.widget.opacity
            );

            const SDL_FRect rect{
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(
                    size.width
                ),
                static_cast<float>(
                    size.height
                )
            };

            if (node.widget.filled) {
                SDL_RenderFillRect(
                    renderer,
                    &rect
                );
            }
            else {
                SDL_RenderRect(
                    renderer,
                    &rect
                );
            }

            break;
        }

        case 4:
        case 8:
            if (options.showText) {
                renderInterfaceText(
                    renderer,
                    node.widget,
                    x,
                    y,
                    size.width
                );
            }
            break;

        case 5:
            if (options.showSprites) {
                renderInterfaceSprite(
                    renderer,
                    node.widget,
                    x,
                    y,
                    spriteCache
                );
            }
            break;

        case 6:
            if (options.showModels) {
                renderInterfaceModel(
                    renderer,
                    viewState,
                    node,
                    x,
                    y,
                    resources
                );
            }
            break;

        case 7:
            if (options.showItemLists) {
                renderInterfaceItemList(
                    renderer,
                    node.widget,
                    x,
                    y
                );
            }
            break;

        default:
            break;
    }

    SDL_Rect childClip =
        clip;

    if (
        node.widget.type == 0 &&
        node.widget.width > 0 &&
        node.widget.height > 0
    ) {
        const SDL_Rect containerRect{
            x,
            y,
            static_cast<int>(
                node.widget.width
            ),
            static_cast<int>(
                node.widget.height
            )
        };

        const std::optional<SDL_Rect> clipped =
            intersectRects(
                clip,
                containerRect
            );

        if (!clipped.has_value()) {
            return;
        }

        childClip =
            *clipped;
    }

    for (
        const InterfaceViewNode& child :
        node.children
    ) {
        if (
            options.showParentLinks &&
            (
                !child.widget.hidden ||
                options.showHiddenWidgets
            )
        ) {
            SDL_SetRenderClipRect(
                renderer,
                &childClip
            );

            SDL_SetRenderDrawColor(
                renderer,
                120,
                160,
                255,
                170
            );

            SDL_RenderLine(
                renderer,
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(x + child.x),
                static_cast<float>(y + child.y)
            );
        }

        renderInterfaceNode(
            renderer,
            viewState,
            child,
            x,
            y,
            childClip,
            spriteCache,
            resources,
            options
        );

        SDL_SetRenderClipRect(
            renderer,
            &childClip
        );
    }

    SDL_SetRenderClipRect(
        renderer,
        &clip
    );

    const bool drawWidgetBounds =
        options.showWidgetBounds ||
        (
            options.showContainerBounds &&
            node.widget.type == 0
        );

    if (drawWidgetBounds) {
        SDL_SetRenderDrawColor(
            renderer,
            node.widget.hidden ? 255 : 80,
            node.widget.hidden ? 120 : 210,
            255,
            220
        );

        const SDL_FRect bounds{
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(size.width),
            static_cast<float>(size.height)
        };

        SDL_RenderRect(
            renderer,
            &bounds
        );
    }

    if (
        options.showClipRegions &&
        node.widget.type == 0 &&
        node.widget.width > 0 &&
        node.widget.height > 0
    ) {
        SDL_SetRenderDrawColor(
            renderer,
            255,
            180,
            80,
            210
        );

        const SDL_FRect clipBounds{
            static_cast<float>(childClip.x),
            static_cast<float>(childClip.y),
            static_cast<float>(childClip.w),
            static_cast<float>(childClip.h)
        };

        SDL_RenderRect(
            renderer,
            &clipBounds
        );
    }

    if (
        options.showScrollExtents &&
        node.widget.type == 0 &&
        node.widget.scrollHeight > node.widget.height
    ) {
        SDL_SetRenderDrawColor(
            renderer,
            190,
            110,
            255,
            200
        );

        const SDL_FRect scrollBounds{
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(
                std::max(
                    1,
                    size.width
                )
            ),
            static_cast<float>(
                node.widget.scrollHeight
            )
        };

        SDL_RenderRect(
            renderer,
            &scrollBounds
        );
    }

    if (options.showWidgetOrigins) {
        SDL_SetRenderDrawColor(
            renderer,
            255,
            100,
            100,
            230
        );

        SDL_RenderLine(
            renderer,
            static_cast<float>(x - 3),
            static_cast<float>(y),
            static_cast<float>(x + 3),
            static_cast<float>(y)
        );

        SDL_RenderLine(
            renderer,
            static_cast<float>(x),
            static_cast<float>(y - 3),
            static_cast<float>(x),
            static_cast<float>(y + 3)
        );
    }

    if (
        options.showWidgetIds ||
        options.showWidgetTypes
    ) {
        std::string label;

        if (options.showWidgetIds) {
            label =
                std::to_string(
                    node.widget.id
                );
        }

        if (options.showWidgetTypes) {
            if (!label.empty()) {
                label += " ";
            }

            label +=
                interfaceWidgetTypeName(
                    node.widget.type
                );
        }

        SDL_SetRenderDrawColor(
            renderer,
            255,
            255,
            255,
            255
        );

        SDL_RenderDebugText(
            renderer,
            static_cast<float>(x + 2),
            static_cast<float>(y + 2),
            label.c_str()
        );
    }
}

void renderInterfaceView(
    SDL_Renderer* renderer,
    const CacheExplorerState& state,
    const InterfaceViewState& viewState,
    eld::sprite::SpriteRepository& spriteRepository,
    eld::graphics::GraphicsResources& resources,
    const InterfaceViewOptions& options
) {
    const SDL_Rect viewportClip{
        state.viewportX,
        state.viewportY,
        state.viewportWidth,
        state.viewportHeight
    };

    SDL_SetRenderClipRect(
        renderer,
        &viewportClip
    );

    SDL_SetRenderDrawBlendMode(
        renderer,
        SDL_BLENDMODE_BLEND
    );

    const int originX =
        state.viewportX +
        (
            state.viewportWidth -
            viewState.width
        ) /
        2;

    const int originY =
        state.viewportY +
        (
            state.viewportHeight -
            viewState.height
        ) /
        2;

    const SDL_FRect canvas{
        static_cast<float>(
            originX
        ),
        static_cast<float>(
            originY
        ),
        static_cast<float>(
            viewState.width
        ),
        static_cast<float>(
            viewState.height
        )
    };

    if (options.showGrid) {
        const int spacing =
            std::max(
                options.gridSpacing,
                1
            );

        SDL_SetRenderDrawColor(
            renderer,
            90,
            90,
            90,
            120
        );

        for (
            int gridX = spacing;
            gridX < viewState.width;
            gridX += spacing
        ) {
            SDL_RenderLine(
                renderer,
                static_cast<float>(originX + gridX),
                static_cast<float>(originY),
                static_cast<float>(originX + gridX),
                static_cast<float>(originY + viewState.height)
            );
        }

        for (
            int gridY = spacing;
            gridY < viewState.height;
            gridY += spacing
        ) {
            SDL_RenderLine(
                renderer,
                static_cast<float>(originX),
                static_cast<float>(originY + gridY),
                static_cast<float>(originX + viewState.width),
                static_cast<float>(originY + gridY)
            );
        }
    }

    if (options.showCanvasBounds) {
        SDL_SetRenderDrawColor(
            renderer,
            110,
            110,
            110,
            255
        );

        SDL_RenderRect(
            renderer,
            &canvas
        );
    }

    InterfaceSpriteCache spriteCache(
        renderer,
        spriteRepository
    );

    renderInterfaceNode(
        renderer,
        viewState,
        viewState.root,
        originX,
        originY,
        viewportClip,
        spriteCache,
        resources,
        options
    );

    SDL_SetRenderClipRect(
        renderer,
        nullptr
    );

    SDL_SetRenderDrawBlendMode(
        renderer,
        SDL_BLENDMODE_NONE
    );
}

void renderMapToolbar(
    CacheExplorerState& state
) {
    ImGui::TextUnformatted("PLANE");
    ImGui::SameLine();

    for (
        std::size_t plane = 0;
        plane < eld::map::PlaneCount;
        ++plane
    ) {
        if (plane != 0) {
            ImGui::SameLine();
        }

        const bool selected =
            state.mapPlane == plane;

        ImGui::PushID(
            static_cast<int>(plane)
        );

        if (
            ImGui::Selectable(
                std::to_string(plane).c_str(),
                selected,
                0,
                ImVec2(32.0f, 0.0f)
            )
        ) {
            state.mapPlane = plane;
            state.selectedMapTile.reset();
            state.selectedMapLocIndex.reset();
            state.mapViewportDirty = true;
        }

        ImGui::PopID();
    }

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    if (
        ImGui::Checkbox(
            "Terrain",
            &state.mapShowTerrain
        )
    ) {
        if (!state.mapShowTerrain) {
            state.selectedMapTile.reset();
        }

        state.mapViewportDirty = true;
    }

    ImGui::SameLine();

    if (
        ImGui::Checkbox(
            "Objects",
            &state.mapShowLocs
        )
    ) {
        if (!state.mapShowLocs) {
            state.selectedMapLocIndex.reset();
        }

        state.mapViewportDirty = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Reset view")) {
        resetMapView(
            state.mapPlane,
            state.mapYaw,
            state.mapPitch,
            state.mapDistance
        );

        state.selectedMapTile.reset();
        state.selectedMapLocIndex.reset();
        state.mapViewportDirty = true;
    }
}

struct MapProjectedPoint {
    ImVec2 screen{};
    float depth = 0.0f;
    bool valid = false;
};

struct MapProjectionContext {
    eld::math::Mat4 model;
    eld::math::Mat4 view;
    eld::math::Mat4 projection;
    eld::render::Camera camera;
    float viewportX = 0.0f;
    float viewportY = 0.0f;
    bool valid = false;
};

MapProjectionContext makeMapProjectionContext(
    const CacheExplorerState& state
) {
    if (!state.activeMap.has_value()) {
        return {};
    }

    const MapViewState& viewState =
        *state.activeMap;

    const std::size_t terrainIndex =
        viewState.terrainObjectIndices[0];

    if (terrainIndex >= viewState.scene.objects.size()) {
        return {};
    }

    return {
        eld::render::buildModelMatrix(
            viewState.scene.objects[terrainIndex].transform
        ),
        eld::render::buildViewMatrix(
            viewState.scene.camera
        ),
        eld::render::buildProjectionMatrix(
            viewState.scene.camera
        ),
        viewState.scene.camera,
        static_cast<float>(state.viewportX),
        static_cast<float>(state.viewportY),
        true
    };
}

MapProjectedPoint projectMapLocalPoint(
    const MapProjectionContext& context,
    const eld::math::Vec3& localPoint
) {
    if (!context.valid) {
        return {};
    }

    const eld::math::Vec3 world =
        context.model.transformPoint(
            localPoint
        );

    const eld::render::ScreenPoint point =
        eld::render::projectPoint(
            world,
            context.view,
            context.projection,
            context.camera
        );

    return {
        ImVec2(
            context.viewportX + point.x,
            context.viewportY + point.y
        ),
        point.depth,
        point.depth >= context.camera.nearPlane &&
            point.depth <= context.camera.farPlane
    };
}

float mapScreenDistanceSquared(
    const ImVec2& a,
    const ImVec2& b
) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

void pickMapElement(
    CacheExplorerState& state,
    const ImVec2& mouse
) {
    if (!state.activeMap.has_value()) {
        return;
    }

    MapViewState& viewState =
        *state.activeMap;

    updateMapViewScene(
        viewState,
        state.mapPlane,
        state.mapShowTerrain,
        state.mapShowLocs,
        state.mapYaw,
        state.mapPitch,
        state.mapDistance,
        static_cast<std::uint32_t>(
            std::max(state.viewportWidth, 1)
        ),
        static_cast<std::uint32_t>(
            std::max(state.viewportHeight, 1)
        )
    );

    const MapProjectionContext projection =
        makeMapProjectionContext(
            state
        );

    if (!projection.valid) {
        return;
    }

    constexpr float LocRadius = 18.0f;
    constexpr float TileRadius = 24.0f;

    if (state.mapShowLocs) {
        std::optional<std::size_t> bestLoc;
        float bestLocDistance =
            LocRadius * LocRadius;

        for (
            std::size_t index = 0;
            index < viewState.sceneLocs.size();
            ++index
        ) {
            const eld::graphics::map::SceneLocationPlacement& loc =
                viewState.sceneLocs[index];

            if (loc.scenePlane != state.mapPlane) {
                continue;
            }

            const MapProjectedPoint projected =
                projectMapLocalPoint(
                    projection,
                    {
                        static_cast<float>(loc.sceneX),
                        -static_cast<float>(loc.sceneY),
                        static_cast<float>(loc.sceneZ)
                    }
                );

            if (!projected.valid) {
                continue;
            }

            const float distance =
                mapScreenDistanceSquared(
                    mouse,
                    projected.screen
                );

            if (distance < bestLocDistance) {
                bestLocDistance = distance;
                bestLoc = index;
            }
        }

        if (bestLoc.has_value()) {
            state.selectedMapLocIndex =
                *bestLoc;
            state.selectedMapTile.reset();
            return;
        }
    }

    if (!state.mapShowTerrain) {
        state.selectedMapLocIndex.reset();
        state.selectedMapTile.reset();
        return;
    }

    std::optional<MapTileSelection> bestTile;
    float bestTileDistance =
        TileRadius * TileRadius;

    for (
        int x = 0;
        x < static_cast<int>(eld::map::RegionSize);
        ++x
    ) {
        for (
            int y = 0;
            y < static_cast<int>(eld::map::RegionSize);
            ++y
        ) {
            const eld::map::MapTile& tile =
                viewState.centerRegion.tile(
                    state.mapPlane,
                    static_cast<std::size_t>(x),
                    static_cast<std::size_t>(y)
                );

            const MapProjectedPoint projected =
                projectMapLocalPoint(
                    projection,
                    {
                        static_cast<float>(x * 128 + 64),
                        -static_cast<float>(tile.height),
                        static_cast<float>(y * 128 + 64)
                    }
                );

            if (!projected.valid) {
                continue;
            }

            const float distance =
                mapScreenDistanceSquared(
                    mouse,
                    projected.screen
                );

            if (distance < bestTileDistance) {
                bestTileDistance = distance;
                bestTile = MapTileSelection{
                    state.mapPlane,
                    x,
                    y
                };
            }
        }
    }

    state.selectedMapLocIndex.reset();
    state.selectedMapTile = bestTile;
}

void renderMapInteraction(
    CacheExplorerState& state,
    const ImVec2& viewportSize
) {
    ImGui::InvisibleButton(
        "##MapViewportCanvas",
        viewportSize
    );

    if (!ImGui::IsItemHovered()) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();

    if (io.MouseWheel != 0.0f) {
        state.mapDistance =
            std::clamp(
                state.mapDistance -
                    io.MouseWheel * 4.0f,
                24.0f,
                180.0f
            );

        state.mapViewportDirty = true;
    }

    if (
        ImGui::IsMouseClicked(
            ImGuiMouseButton_Left
        )
    ) {
        pickMapElement(
            state,
            io.MousePos
        );
    }

    if (
        ImGui::IsMouseDown(
            ImGuiMouseButton_Right
        )
    ) {
        state.mapYaw +=
            io.MouseDelta.x * 0.010f;

        state.mapPitch =
            std::clamp(
                state.mapPitch -
                    io.MouseDelta.y * 0.010f,
                0.12f,
                1.35f
            );

        state.mapViewportDirty = true;
    }
}

}

void ViewportPanel::shutdown() {
    viewportSurface_.shutdown();
    viewportRenderer_ = nullptr;

    mapViewSurface_.shutdown();
}



namespace {

bool isModelWorkspace(
    ViewportViewKind kind
) {
    switch (kind) {
        case ViewportViewKind::Animation:
        case ViewportViewKind::Npc:
        case ViewportViewKind::Location:
        case ViewportViewKind::SpotAnimation:
        case ViewportViewKind::Model:
            return true;

        case ViewportViewKind::None:
        case ViewportViewKind::Map:
        case ViewportViewKind::Midi:
        case ViewportViewKind::Interface:
        case ViewportViewKind::Texture:
        case ViewportViewKind::Image:
        case ViewportViewKind::Sprite:
            return false;
    }

    return false;
}


bool isDockedWorkspace(
    ViewportViewKind kind
) {
    switch (kind) {
        case ViewportViewKind::Midi:
        case ViewportViewKind::Interface:
        case ViewportViewKind::Texture:
        case ViewportViewKind::Image:
        case ViewportViewKind::Sprite:
            return true;

        case ViewportViewKind::None:
        case ViewportViewKind::Map:
        case ViewportViewKind::Animation:
        case ViewportViewKind::Npc:
        case ViewportViewKind::Location:
        case ViewportViewKind::SpotAnimation:
        case ViewportViewKind::Model:
            return false;
    }

    return false;
}



void renderViewportToolRail(
    CacheExplorerState& state
) {
    if (
        ui::workspace::toolButton(
            "Move",
            ui::Icon::Move,
            "Move [W]",
            state.viewportGizmoMode ==
                ViewportGizmoMode::Move
        )
    ) {
        state.viewportGizmoMode =
            ViewportGizmoMode::Move;
    }

    if (
        ui::workspace::toolButton(
            "Rotate",
            ui::Icon::Rotate,
            "Rotate [E]",
            state.viewportGizmoMode ==
                ViewportGizmoMode::Rotate
        )
    ) {
        state.viewportGizmoMode =
            ViewportGizmoMode::Rotate;
    }

    if (
        ui::workspace::toolButton(
            "Scale",
            ui::Icon::Scale,
            "Scale [R]",
            state.viewportGizmoMode ==
                ViewportGizmoMode::Scale
        )
    ) {
        state.viewportGizmoMode =
            ViewportGizmoMode::Scale;
    }

    ui::workspace::toolDivider();

    if (
        ui::workspace::toolButton(
            "Grid",
            ui::Icon::Grid,
            state.showEditorGrid
                ? "Hide grid"
                : "Show grid",
            state.showEditorGrid
        )
    ) {
        state.showEditorGrid =
            !state.showEditorGrid;
    }

    if (
        ui::workspace::toolButton(
            "Focus",
            ui::Icon::Focus,
            "Focus model [F]"
        )
    ) {
        state.viewportCameraPivot =
            state.modelTransform.position;

        updateEditorCameraPosition(
            state
        );
    }

    ui::workspace::toolDivider();

    if (
        ui::workspace::toolButton(
            "ResetView",
            ui::Icon::Restart,
            "Reset viewport"
        )
    ) {
        resetEditorCamera(
            state
        );
    }
}


std::string selectedNumericId(
    const CacheExplorerState& state
) {
    if (
        state.selection.definitionId >=
        0
    ) {
        return
            std::to_string(
                state.selection.definitionId
            );
    }

    if (
        state.selection.fileId >=
        0
    ) {
        return
            std::to_string(
                state.selection.fileId
            );
    }

    if (
        state.selection.regionId >=
        0
    ) {
        return
            std::to_string(
                state.selection.regionId
            );
    }

    return {};
}


std::string workspaceIdentity(
    const CacheExplorerState& state,
    ViewportViewKind kind
) {
    const std::string id =
        selectedNumericId(
            state
        );

    const auto withId =
        [&id](
            const char* label
        ) {
            return
                id.empty()
                    ? std::string(label)
                    : std::string(label) +
                        " " +
                        id;
        };

    switch (kind) {
        case ViewportViewKind::Map:
            if (state.activeMap.has_value()) {
                return
                    "MAP " +
                    std::to_string(
                        state.activeMap->
                            indexEntry.regionId
                    );
            }

            return withId(
                "MAP"
            );

        case ViewportViewKind::Midi:
            return withId(
                "MIDI"
            );

        case ViewportViewKind::Interface:
            return withId(
                "INTERFACE"
            );

        case ViewportViewKind::Npc:
            return withId(
                "NPC"
            );

        case ViewportViewKind::Location:
            return withId(
                "OBJECT"
            );

        case ViewportViewKind::SpotAnimation:
            return withId(
                "SPOT ANIMATION"
            );

        case ViewportViewKind::Model:
            if (state.activeModel.has_value()) {
                return
                    "MODEL " +
                    std::to_string(
                        state.activeModel->id
                    );
            }

            return withId(
                "MODEL"
            );

        case ViewportViewKind::Texture:
            return withId(
                "TEXTURE"
            );

        case ViewportViewKind::Image:
            return
                state.selection.name.empty()
                    ? std::string(
                          "IMAGE"
                      )
                    : std::string(
                          "IMAGE "
                      ) +
                          state.selection.name;

        case ViewportViewKind::Sprite:
            if (
                !state.selection.name.empty()
            ) {
                return
                    std::string(
                        "SPRITE "
                    ) +
                    state.selection.name;
            }

            return withId(
                "SPRITE"
            );

        case ViewportViewKind::Animation:
            if (state.activeAnimation.has_value()) {
                return
                    "ANIMATION " +
                    std::to_string(
                        state.activeAnimation->
                            animation.id
                    );
            }

            return withId(
                "ANIMATION"
            );

        case ViewportViewKind::None:
        default:
            return "VIEW";
    }
}


std::string workspaceSummary(
    const CacheExplorerState& state,
    ViewportViewKind kind
) {
    switch (kind) {
        case ViewportViewKind::Map:
            if (state.activeMap.has_value()) {
                return
                    std::to_string(
                        state.activeMap->
                            indexEntry.regionX()
                    ) +
                    ", " +
                    std::to_string(
                        state.activeMap->
                            indexEntry.regionY()
                    ) +
                    "  ·  " +
                    std::to_string(
                        state.activeMap->
                            stats.locPlacements
                    ) +
                    " objects";
            }
            break;

        case ViewportViewKind::Midi:
            return
                state.midiView.totalTicks > 0
                    ? std::to_string(
                          state.midiView.totalTicks
                      ) +
                          " ticks"
                    : state.midiView.
                        playbackStatus;

        case ViewportViewKind::Interface:
            return
                state.selection.label;

        case ViewportViewKind::Npc:
        case ViewportViewKind::Location:
        case ViewportViewKind::SpotAnimation:
        case ViewportViewKind::Model:
            if (state.activeModel.has_value()) {
                return
                    std::to_string(
                        state.activeModel->
                            mesh.vertices.size()
                    ) +
                    " vertices  ·  " +
                    std::to_string(
                        state.activeModel->
                            mesh.faces.size()
                    ) +
                    " faces";
            }
            break;

        case ViewportViewKind::Texture:
            if (state.activeTexture.has_value()) {
                return
                    std::to_string(
                        state.activeTexture->
                            image.width
                    ) +
                    " x " +
                    std::to_string(
                        state.activeTexture->
                            image.height
                    );
            }
            break;

        case ViewportViewKind::Image:
            if (state.activeImage.has_value()) {
                return
                    std::to_string(
                        state.activeImage->
                            width
                    ) +
                    " x " +
                    std::to_string(
                        state.activeImage->
                            height
                    );
            }
            break;

        case ViewportViewKind::Sprite:
            if (state.activeSprite.has_value()) {
                return
                    std::to_string(
                        state.activeSprite->
                            image.width
                    ) +
                    " x " +
                    std::to_string(
                        state.activeSprite->
                            image.height
                    ) +
                    "  ·  frame " +
                    std::to_string(
                        std::max(
                            state.selection.frameId,
                            0
                        )
                    );
            }
            break;

        case ViewportViewKind::Animation:
            if (state.activeAnimation.has_value()) {
                return
                    std::to_string(
                        state.activeAnimation->
                            animation.asset.frames.size()
                    ) +
                    " frames  ·  " +
                    std::to_string(
                        state.activeAnimation->
                            animation.asset.skeleton.slots.size()
                    ) +
                    " slots";
            }
            break;

        case ViewportViewKind::None:
            break;
    }

    return {};
}


void renderWorkspaceHeader(
    const CacheExplorerState& state,
    ViewportViewKind kind
) {
    ui::workspace::renderIdentityHeader(
        workspaceIdentity(
            state,
            kind
        ),
        workspaceSummary(
            state,
            kind
        )
    );
}


void renderImageWorkspace(
    CacheExplorerState& state,
    ViewportViewKind kind,
    const ImVec2& controlsPosition,
    const ImVec2& controlsSize
) {
    const bool sprite =
        kind ==
        ViewportViewKind::Sprite;

    const std::uint32_t width =
        sprite
            ? (
                  state.activeSprite.has_value()
                      ? state.activeSprite->
                            image.width
                      : 0
              )
            : (
                  state.activeImage.has_value()
                      ? state.activeImage->
                            width
                      : 0
              );

    const std::uint32_t height =
        sprite
            ? (
                  state.activeSprite.has_value()
                      ? state.activeSprite->
                            image.height
                      : 0
              )
            : (
                  state.activeImage.has_value()
                      ? state.activeImage->
                            height
                      : 0
              );


    const viewport_workspace::BottomRow layout =
        viewport_workspace::bottomRow(
            controlsSize.x
        );

    ui::workspace::beginDockedHud(
        sprite
            ? "##SpriteBottomHud"
            : "##ImageBottomHud",
        controlsPosition,
        controlsSize
    );

    const float cardY =
        ui::workspace::dockedCardY(
            controlsPosition,
            controlsSize
        );


    ui::workspace::beginCard(
        sprite
            ? "##SpriteViewCard"
            : "##ImageViewCard",
        ImVec2(
            controlsPosition.x +
                layout.leftX,
            cardY
        ),
        ImVec2(
            viewport_workspace::
                LeftWidth,
            viewport_workspace::
                CardHeight
        )
    );

    ui::workspace::centeredText(
        "VIEW",
        7.0f,
        true
    );

    ui::workspace::centeredText(
        "FIT",
        34.0f
    );

    ui::workspace::centeredText(
        "native pixels",
        61.0f,
        true
    );

    ui::workspace::endCard();


    if (
        layout.centerWidth >=
        150.0f
    ) {
        ui::workspace::beginCard(
            sprite
                ? "##SpriteInfoCard"
                : "##ImageInfoCard",
            ImVec2(
                controlsPosition.x +
                    layout.centerX,
                cardY
            ),
            ImVec2(
                layout.centerWidth,
                viewport_workspace::
                    CardHeight
            )
        );

        ui::workspace::centeredText(
            sprite
                ? "SPRITE"
                : "IMAGE",
            7.0f,
            true
        );

        ui::workspace::centeredText(
            std::to_string(
                width
            ) +
                " x " +
                std::to_string(
                    height
                ),
            34.0f
        );

        if (sprite) {
            ui::workspace::centeredText(
                "frame " +
                    std::to_string(
                        std::max(
                            state.selection.frameId,
                            0
                        )
                    ),
                61.0f,
                true
            );
        }

        ui::workspace::endCard();
    }


    ui::workspace::beginCard(
        sprite
            ? "##SpriteSourceCard"
            : "##ImageSourceCard",
        ImVec2(
            controlsPosition.x +
                layout.rightX,
            cardY
        ),
        ImVec2(
            layout.rightWidth,
            viewport_workspace::
                CardHeight
        )
    );

    ui::workspace::centeredText(
        "SOURCE",
        7.0f,
        true
    );

    std::string source =
        state.selection.label;

    if (source.empty()) {
        source =
            state.selection.name;
    }

    if (source.empty()) {
        source =
            "cache asset";
    }

    if (source.size() > 38) {
        source =
            source.substr(
                0,
                35
            ) +
            "...";
    }

    ui::workspace::centeredText(
        source,
        34.0f
    );

    std::string sourceIds =
        "archive " +
        std::to_string(
            state.selection.archiveId
        ) +
        "  ·  file " +
        std::to_string(
            state.selection.fileId
        );

    if (sprite) {
        sourceIds +=
            "  ·  frame " +
            std::to_string(
                std::max(
                    state.selection.frameId,
                    0
                )
            );
    }

    ui::workspace::centeredText(
        sourceIds,
        61.0f,
        true
    );

    ui::workspace::endCard();

    ui::workspace::endDockedHud();
}

}



void ViewportPanel::render(
    CacheExplorerState& state,
    float width,
    float height,
    eld::audio::MidiPlayer& midiPlayer,
    const std::function<void()>&
        renderAnimationControls
) {
    ImGui::BeginChild(
        "ViewportPanel",
        ImVec2(
            width,
            height
        ),
        true
    );


    const ImVec2 available =
        ImGui::GetContentRegionAvail();

    const ViewportViewKind viewKind =
        workspaceRouter_.kindFor(
            state
        );

    workspaceRouter_.update(
        state,
        viewKind
    );

    midiView_.update(
        state.activeMidi.has_value()
            ? &*state.activeMidi
            : nullptr,
        state.midiView
    );


    const bool workspaceActive =
        viewKind !=
        ViewportViewKind::None;

    const bool modelWorkspace =
        isModelWorkspace(
            viewKind
        );

    const bool dockedWorkspace =
        isDockedWorkspace(
            viewKind
        );

    const bool immersiveWorkspace =
        modelWorkspace ||
        viewKind ==
            ViewportViewKind::Map;


    const float spacing =
        ImGui::GetStyle().
            ItemSpacing.y;

    const float headerHeight =
        workspaceActive
            ? ui::workspace::
                  HeaderHeight +
                  spacing
            : 0.0f;

    const float bodyHeight =
        std::max(
            available.y -
                headerHeight,
            1.0f
        );


    // --------------------------------------------------------
    // SHARED IDENTITY HEADER
    // --------------------------------------------------------

    if (workspaceActive) {
        ImGui::BeginChild(
            "##ViewportWorkspaceHeader",
            ImVec2(
                0.0f,
                ui::workspace::
                    HeaderHeight
            ),
            false,
            ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse
        );

        renderWorkspaceHeader(
            state,
            viewKind
        );

        ImGui::EndChild();
    }


    // --------------------------------------------------------
    // WORKSPACE COMPOSITION
    //
    // 3D / map:
    //   canvas fills the body and HUD floats inside it.
    //
    // 2D / MIDI:
    //   canvas is display-faithful and stops BEFORE controls.
    //   controls live in a real dock underneath it.
    // --------------------------------------------------------

    constexpr float MinimumCanvasHeight =
        80.0f;

    const float dockHeight =
        dockedWorkspace
            ? std::min(
                  ui::workspace::DockHeight,
                  std::max(
                      bodyHeight -
                          MinimumCanvasHeight -
                          spacing,
                      0.0f
                  )
              )
            : 0.0f;

    const float canvasHeight =
        std::max(
            bodyHeight -
                (
                    dockHeight > 0.0f
                        ? dockHeight +
                            spacing
                        : 0.0f
                ),
            1.0f
        );


    if (workspaceActive) {
        ImGui::PushStyleVar(
            ImGuiStyleVar_WindowPadding,
            ImVec2(
                0.0f,
                0.0f
            )
        );
    }

    ImGui::BeginChild(
        "ViewportCanvasWindow",
        ImVec2(
            0.0f,
            canvasHeight
        ),
        dockedWorkspace ||
            !workspaceActive,
        ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
    );

    if (workspaceActive) {
        ImGui::PopStyleVar();
    }


    const ImVec2 viewportPosition =
        ImGui::GetCursorScreenPos();

    const ImVec2 viewportSize =
        ImGui::GetContentRegionAvail();


    // state.viewport* always means the TRUE render canvas.
    // It never includes a 2D control dock.
    state.viewportX =
        static_cast<int>(
            viewportPosition.x
        );

    state.viewportY =
        static_cast<int>(
            viewportPosition.y
        );

    state.viewportWidth =
        std::max(
            static_cast<int>(
                viewportSize.x
            ),
            1
        );

    state.viewportHeight =
        std::max(
            static_cast<int>(
                viewportSize.y
            ),
            1
        );


    if (viewportRenderer_ != nullptr) {
        viewportSurface_.ensure(
            viewportRenderer_,
            state.viewportWidth,
            state.viewportHeight
        );
    }


    // --------------------------------------------------------
    // BASE CANVAS
    // --------------------------------------------------------

    if (
        viewKind !=
            ViewportViewKind::Midi &&
        viewportSurface_.texture() !=
            nullptr
    ) {
        const ImVec2 viewportEnd{
            viewportPosition.x +
                viewportSize.x,
            viewportPosition.y +
                viewportSize.y
        };

        ImGui::GetWindowDrawList()->
            AddImage(
                (ImTextureID)(
                    std::intptr_t
                ) viewportSurface_.
                    texture(),
                viewportPosition,
                viewportEnd
            );
    }


    if (
        viewKind ==
        ViewportViewKind::Midi
    ) {
        midiViewPanel_.
            renderVisualization(
                state.midiView,
                midiPlayer,
                viewportSize
            );
    }


    // The soft light/vignette is presentation for 3D scenes.
    // Never tint a texture, sprite, image, interface, or MIDI
    // visualization while somebody is inspecting its pixels.
    if (immersiveWorkspace) {
        ui::workspace::
            drawViewportAtmosphere(
                ImGui::GetWindowDrawList(),
                viewportPosition,
                viewportSize
            );
    }


    // --------------------------------------------------------
    // VIEWPORT INTERACTION
    // --------------------------------------------------------

    if (
        viewKind ==
        ViewportViewKind::Map
    ) {
        ImGui::SetCursorScreenPos(
            viewportPosition
        );

        renderMapInteraction(
            state,
            viewportSize
        );
    }
    else if (modelWorkspace) {
        renderEditorOverlay(
            state,
            viewportPosition,
            viewportSize
        );
    }


    // --------------------------------------------------------
    // SHARED 3D TOOL RAIL
    // --------------------------------------------------------

    if (
        modelWorkspace &&
        state.activeModelHandle.
            has_value()
    ) {
        ui::workspace::beginToolRail(
            "##ViewportToolRail",
            viewportPosition
        );

        renderViewportToolRail(
            state
        );

        ui::workspace::endToolRail();
    }


    // --------------------------------------------------------
    // IMMERSIVE CONTENT
    // --------------------------------------------------------

    switch (viewKind) {
        case ViewportViewKind::Animation:
            AnimationViewPanel{}.render(
                state,
                renderAnimationControls
            );
            break;

        case ViewportViewKind::Map:
            mapViewPanel_.render(
                state
            );
            break;

        case ViewportViewKind::Npc:
        case ViewportViewKind::Location:
        case ViewportViewKind::SpotAnimation:
        case ViewportViewKind::Model:
            workspaceRouter_.
                renderModelWorkspace(
                    state
                );
            break;

        case ViewportViewKind::Midi:
        case ViewportViewKind::Interface:
        case ViewportViewKind::Texture:
        case ViewportViewKind::Image:
        case ViewportViewKind::Sprite:
        case ViewportViewKind::None:
        default:
            break;
    }


    // Absolute overlay children move ImGui's cursor. Restore the
    // canvas item so this child consumes the exact canvas height.
    ImGui::SetCursorScreenPos(
        viewportPosition
    );

    ImGui::Dummy(
        viewportSize
    );

    ImGui::EndChild();


    // --------------------------------------------------------
    // DOCKED 2D / MEDIA CONTROLS
    // --------------------------------------------------------

    if (
        dockedWorkspace &&
        dockHeight > 0.0f
    ) {
        const ImVec2 controlsPosition =
            ImGui::GetCursorScreenPos();

        const ImVec2 controlsSize{
            ImGui::GetContentRegionAvail().x,
            dockHeight
        };

        switch (viewKind) {
            case ViewportViewKind::Midi:
                midiViewPanel_.
                    renderWorkspace(
                        state,
                        state.activeMidi.has_value()
                            ? &*state.activeMidi
                            : nullptr,
                        state.midiView,
                        midiPlayer,
                        controlsPosition,
                        controlsSize
                    );
                break;

            case ViewportViewKind::Interface:
                workspaceRouter_.
                    renderInterfaceWorkspace(
                        state,
                        controlsPosition,
                        controlsSize
                    );
                break;

            case ViewportViewKind::Texture:
                workspaceRouter_.
                    renderTextureWorkspace(
                        state,
                        controlsPosition,
                        controlsSize
                    );
                break;

            case ViewportViewKind::Image:
            case ViewportViewKind::Sprite:
                renderImageWorkspace(
                    state,
                    viewKind,
                    controlsPosition,
                    controlsSize
                );
                break;

            case ViewportViewKind::None:
            case ViewportViewKind::Map:
            case ViewportViewKind::Animation:
            case ViewportViewKind::Npc:
            case ViewportViewKind::Location:
            case ViewportViewKind::SpotAnimation:
            case ViewportViewKind::Model:
            default:
                break;
        }

        // Keep the parent layout honest even though the dock uses
        // absolute placement internally.
        ImGui::SetCursorScreenPos(
            controlsPosition
        );

        ImGui::Dummy(
            controlsSize
        );
    }


    ImGui::EndChild();
}


void ViewportPanel::prepareViewport(
    SDL_Renderer* renderer,
    CacheExplorerState& state,
    eld::graphics::GraphicsResources& resources
) {
    if (!state.activeMap.has_value()) {
        return;
    }

    if (
        !mapViewSurface_.prepare(
            renderer,
            state,
            resources
        )
    ) {
        state.mapViewError =
            mapViewSurface_.error();
    }
    else {
        state.mapViewError.clear();
    }
}

void ViewportPanel::renderViewport(
    SDL_Renderer* renderer,
    CacheExplorerState& state,
    eld::graphics::GraphicsResources& resources,
    const eld::interface::InterfaceRepository& interfaces,
    eld::sprite::SpriteRepository& interfaceSprites
) {
    viewportRenderer_ =
        renderer;

    if (
        !viewportSurface_.ensure(
            renderer,
            state.viewportWidth,
            state.viewportHeight
        )
    ) {
        return;
    }

    if (
        !viewportSurface_.begin(
            renderer
        )
    ) {
        return;
    }

    // Existing viewport render helpers use state.viewportX/Y as
    // their output origin. On the offscreen target the origin is
    // always local (0, 0), not application-window coordinates.
    const int screenViewportX =
        state.viewportX;

    const int screenViewportY =
        state.viewportY;

    state.viewportX =
        0;

    state.viewportY =
        0;

    struct ViewportSurfaceRestore {
        ViewportSurface& surface;
        SDL_Renderer* renderer;
        CacheExplorerState& state;

        int screenX;
        int screenY;

        ~ViewportSurfaceRestore() {
            state.viewportX =
                screenX;

            state.viewportY =
                screenY;

            surface.end(
                renderer
            );
        }
    };

    const ViewportSurfaceRestore restore{
        viewportSurface_,
        renderer,
        state,
        screenViewportX,
        screenViewportY
    };

    // Give empty/failed views a deterministic canvas instead of
    // leaking the previous frame's contents.
    const auto& viewportBackground =
        ui::themePalette().
            viewportBackground;

    SDL_SetRenderDrawColor(
        renderer,
        viewportBackground[0],
        viewportBackground[1],
        viewportBackground[2],
        viewportBackground[3]
    );

    SDL_RenderClear(
        renderer
    );


    if (state.activeMap.has_value()) {
        mapViewSurface_.draw(
            renderer,
            state
        );

        return;
    }

    if (state.activeInterface.has_value()) {
        const InterfaceView view;

        const std::optional<InterfaceViewState> viewState =
            view.build(
                *state.activeInterface,
                interfaces
            );

        if (viewState.has_value()) {
            renderInterfaceView(
                renderer,
                state,
                *viewState,
                interfaceSprites,
                resources,
                workspaceRouter_.interfaceOptions()
            );
        }

        return;
    }

    if (state.activeImage.has_value()) {
        renderImage(
            renderer,
            state,
            *state.activeImage
        );

        return;
    }

    if (state.activeSprite.has_value()) {
        renderImage(
            renderer,
            state,
            state.activeSprite->image
        );

        return;
    }

    if (state.activeTexture.has_value()) {
        renderImage(
            renderer,
            state,
            state.activeTexture->image,
            &workspaceRouter_.textureOptions()
        );

        return;
    }

    if (!state.activeModelHandle.has_value()) {
        return;
    }

    state.camera.viewportWidth =
        static_cast<std::uint32_t>(
            state.viewportWidth
        );

    state.camera.viewportHeight =
        static_cast<std::uint32_t>(
            state.viewportHeight
        );

    const SDL_Rect clip{
        state.viewportX,
        state.viewportY,
        state.viewportWidth,
        state.viewportHeight
    };

    SDL_SetRenderClipRect(
        renderer,
        &clip
    );

    eld::render::RenderObject object;

    object.model =
        *state.activeModelHandle;

    object.transform =
        state.modelTransform;

    eld::render::RenderScene scene;

    scene.camera =
        state.camera;

    scene.objects.push_back(
        object
    );


    for (
        const PresentationRenderObject& presentationObject :
        state.presentationObjects
    ) {
        eld::render::RenderObject extra;
        extra.model = presentationObject.model;
        extra.transform = presentationObject.transform;
        scene.objects.push_back(extra);
    }

    const ModelViewOptions& modelOptions =
        workspaceRouter_.modelOptions();

    const std::array<std::uint8_t, 4>
        background =
            ui::themePalette().
                viewportBackground;

    if (modelOptions.showSolid) {
        eld::render::SoftwareRenderBackend backend(
            renderer
        );

        backend.setOutputPosition(
            state.viewportX,
            state.viewportY
        );

        backend.setClearColor({
            background[0],
            background[1],
            background[2],
            background[3]
        });

        eld::render::RenderPipeline pipeline;

        pipeline.render(
            scene,
            resources,
            backend
        );
    }
    else {
        SDL_SetRenderDrawColor(
            renderer,
            background[0],
            background[1],
            background[2],
            background[3]
        );

        const SDL_FRect backgroundRect{
            static_cast<float>(
                state.viewportX
            ),
            static_cast<float>(
                state.viewportY
            ),
            static_cast<float>(
                state.viewportWidth
            ),
            static_cast<float>(
                state.viewportHeight
            )
        };

        SDL_RenderFillRect(
            renderer,
            &backgroundRect
        );
    }

    if (state.activeModel.has_value()) {
        renderModelOverlays(
            renderer,
            state.activeModel->mesh,
            state,
            modelOptions
        );
    }

    drawEditorOverlaySdl(
        renderer,
        state
    );

    SDL_SetRenderClipRect(
        renderer,
        nullptr
    );
}

}
