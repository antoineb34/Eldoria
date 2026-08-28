#include "CacheViewportPanel.h"

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

#include "../CacheExplorerState.h"
#include "../InterfacePreviewBuilder.h"

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
        SDL_SetRenderDrawColor(
            renderer,
            36,
            38,
            42,
            255
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
    const InterfacePreview& preview,
    const InterfacePreviewNode& node,
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
                preview.modelProjectionFocalLength
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
    const InterfacePreview& preview,
    const InterfacePreviewNode& node,
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
                    preview,
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
        const InterfacePreviewNode& child :
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
            preview,
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

void renderInterfacePreview(
    SDL_Renderer* renderer,
    const CacheExplorerState& state,
    const InterfacePreview& preview,
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
            preview.width
        ) /
        2;

    const int originY =
        state.viewportY +
        (
            state.viewportHeight -
            preview.height
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
            preview.width
        ),
        static_cast<float>(
            preview.height
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
            gridX < preview.width;
            gridX += spacing
        ) {
            SDL_RenderLine(
                renderer,
                static_cast<float>(originX + gridX),
                static_cast<float>(originY),
                static_cast<float>(originX + gridX),
                static_cast<float>(originY + preview.height)
            );
        }

        for (
            int gridY = spacing;
            gridY < preview.height;
            gridY += spacing
        ) {
            SDL_RenderLine(
                renderer,
                static_cast<float>(originX),
                static_cast<float>(originY + gridY),
                static_cast<float>(originX + preview.width),
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
        preview,
        preview.root,
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

}

// ELFORGE_NPC_ANIMATION_DRAWER_V1
void CacheViewportPanel::render(
    CacheExplorerState& state,
    float width,
    float height,
    const std::function<void()>&
        renderAnimationControls
) {
    ImGui::BeginChild(
        "CacheViewportPanel",
        ImVec2(width, height),
        true
    );

    ImGui::TextUnformatted("VIEWPORT");
    ImGui::Separator();

    const ImVec2 available =
        ImGui::GetContentRegionAvail();

    const ViewportViewKind viewKind =
        viewDrawer_.kindFor(
            state
        );

    viewDrawer_.update(
        state,
        viewKind
    );

    const ViewportDrawerLayout drawerLayout =
        viewDrawer_.updateLayout(
            available.y
        );

    const float spacing =
        ImGui::GetStyle().ItemSpacing.y;

    const float viewportHeight =
        std::max(
            available.y -
                drawerLayout.drawerHeight -
                drawerLayout.resizeHandleHeight -
                spacing,
            1.0f
        );

    // The drawable preview stays a real child window.
    // The contextual view drawer is a sibling below it.
    ImGui::BeginChild(
        "ViewportCanvasWindow",
        ImVec2(
            0.0f,
            viewportHeight
        ),
        true,
        ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
    );

    const ImVec2 viewportPosition =
        ImGui::GetCursorScreenPos();

    const ImVec2 viewportSize =
        ImGui::GetContentRegionAvail();

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

    ImGui::Dummy(
        viewportSize
    );

    ImGui::EndChild();

    viewDrawer_.renderResizeHandle(
        drawerLayout
    );

    viewDrawer_.render(
        state,
        viewKind,
        drawerLayout.drawerHeight,
        renderAnimationControls
    );

    ImGui::EndChild();
}

void CacheViewportPanel::renderViewport(
    SDL_Renderer* renderer,
    CacheExplorerState& state,
    eld::graphics::GraphicsResources& resources,
    const eld::interface::InterfaceRepository& interfaces,
    eld::sprite::SpriteRepository& interfaceSprites
) {
    if (state.activeInterface.has_value()) {
        const InterfacePreviewBuilder previewBuilder;

        const std::optional<InterfacePreview> preview =
            previewBuilder.build(
                *state.activeInterface,
                interfaces
            );

        if (preview.has_value()) {
            renderInterfacePreview(
                renderer,
                state,
                *preview,
                interfaceSprites,
                resources,
                viewDrawer_.interfaceOptions()
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
            &viewDrawer_.textureOptions()
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


    // ELFORGE_COMPOSITE_ACTION_PREVIEW_V1
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
        viewDrawer_.modelOptions();

    const std::array<std::uint8_t, 4>
        background =
            modelOptions.backgroundColor();

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

    SDL_SetRenderClipRect(
        renderer,
        nullptr
    );
}

}
