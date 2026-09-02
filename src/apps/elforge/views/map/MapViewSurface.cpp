#include "views/map/MapViewSurface.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <exception>
#include <utility>

#include "explorer/CacheExplorerState.h"

#include "render/camera/Projection.h"
#include "render/scene/Transform.h"

namespace eld::elforge {

namespace {

struct CurrentGlContext {
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
};

CurrentGlContext captureCurrentGlContext() {
    return {
        SDL_GL_GetCurrentWindow(),
        SDL_GL_GetCurrentContext()
    };
}

void restoreGlContext(
    const CurrentGlContext& previous
) {
    if (
        previous.window != nullptr &&
        previous.context != nullptr
    ) {
        SDL_GL_MakeCurrent(
            previous.window,
            previous.context
        );
    }
}

struct MapOverlayPoint {
    float x = 0.0f;
    float y = 0.0f;
    bool valid = false;
};

MapOverlayPoint projectMapOverlayPoint(
    const CacheExplorerState& state,
    const eld::math::Vec3& localPoint
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

    const eld::math::Vec3 world =
        eld::render::buildModelMatrix(
            viewState.scene.objects[terrainIndex].transform
        ).transformPoint(
            localPoint
        );

    const eld::math::Mat4 view =
        eld::render::buildViewMatrix(
            viewState.scene.camera
        );

    const eld::math::Mat4 projection =
        eld::render::buildProjectionMatrix(
            viewState.scene.camera
        );

    const eld::render::ScreenPoint point =
        eld::render::projectPoint(
            world,
            view,
            projection,
            viewState.scene.camera
        );

    return {
        static_cast<float>(state.viewportX) + point.x,
        static_cast<float>(state.viewportY) + point.y,
        point.depth >= viewState.scene.camera.nearPlane &&
            point.depth <= viewState.scene.camera.farPlane
    };
}

void drawMapSelectionCross(
    SDL_Renderer* renderer,
    const MapOverlayPoint& point,
    float radius
) {
    if (!point.valid) {
        return;
    }

    SDL_SetRenderDrawColor(
        renderer,
        255,
        220,
        72,
        255
    );

    SDL_RenderLine(
        renderer,
        point.x - radius,
        point.y,
        point.x + radius,
        point.y
    );

    SDL_RenderLine(
        renderer,
        point.x,
        point.y - radius,
        point.x,
        point.y + radius
    );
}

void drawMapSelectionOverlay(
    SDL_Renderer* renderer,
    const CacheExplorerState& state
) {
    if (!state.activeMap.has_value()) {
        return;
    }

    const MapViewState& viewState =
        *state.activeMap;

    if (
        state.selectedMapLocIndex.has_value() &&
        *state.selectedMapLocIndex < viewState.sceneLocs.size()
    ) {
        const eld::graphics::map::SceneLocationPlacement& loc =
            viewState.sceneLocs[*state.selectedMapLocIndex];

        drawMapSelectionCross(
            renderer,
            projectMapOverlayPoint(
                state,
                {
                    static_cast<float>(loc.sceneX),
                    -static_cast<float>(loc.sceneY),
                    static_cast<float>(loc.sceneZ)
                }
            ),
            9.0f
        );

        return;
    }

    if (!state.selectedMapTile.has_value()) {
        return;
    }

    const MapTileSelection& selection =
        *state.selectedMapTile;

    if (
        selection.plane >= eld::map::PlaneCount ||
        selection.x < 0 ||
        selection.x >= static_cast<int>(eld::map::RegionSize) ||
        selection.y < 0 ||
        selection.y >= static_cast<int>(eld::map::RegionSize)
    ) {
        return;
    }

    const eld::map::MapTile& tile =
        viewState.centerRegion.tile(
            selection.plane,
            static_cast<std::size_t>(selection.x),
            static_cast<std::size_t>(selection.y)
        );

    drawMapSelectionCross(
        renderer,
        projectMapOverlayPoint(
            state,
            {
                static_cast<float>(selection.x * 128 + 64),
                -static_cast<float>(tile.height),
                static_cast<float>(selection.y * 128 + 64)
            }
        ),
        7.0f
    );
}

}

MapViewSurface::~MapViewSurface() {
    shutdown();
}

void MapViewSurface::shutdown() {
    destroyOutputTexture();
    destroyOpenGLContext();
    pixels_.clear();
    flippedPixels_.clear();
    width_ = 0;
    height_ = 0;
}

bool MapViewSurface::ensureOpenGLContext(
    int width,
    int height
) {
    if (
        gpuWindow_ != nullptr &&
        gpuContext_ != nullptr &&
        backend_ != nullptr
    ) {
        if (
            width != width_ ||
            height != height_
        ) {
            SDL_SetWindowSize(
                gpuWindow_,
                std::max(width, 1),
                std::max(height, 1)
            );
            SDL_SyncWindow(gpuWindow_);
        }

        return true;
    }

    if (
        !SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_MAJOR_VERSION,
            3
        ) ||
        !SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_MINOR_VERSION,
            3
        ) ||
        !SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_CORE
        ) ||
        !SDL_GL_SetAttribute(
            SDL_GL_DEPTH_SIZE,
            24
        ) ||
        !SDL_GL_SetAttribute(
            SDL_GL_DOUBLEBUFFER,
            1
        )
    ) {
        error_ =
            std::string("OpenGL attributes: ") +
            SDL_GetError();
        return false;
    }

    gpuWindow_ =
        SDL_CreateWindow(
            "ElForge Map GPU Surface",
            std::max(width, 1),
            std::max(height, 1),
            SDL_WINDOW_OPENGL |
                SDL_WINDOW_HIDDEN
        );

    if (gpuWindow_ == nullptr) {
        error_ =
            std::string("OpenGL map window: ") +
            SDL_GetError();
        return false;
    }

    gpuContext_ =
        SDL_GL_CreateContext(
            gpuWindow_
        );

    if (gpuContext_ == nullptr) {
        error_ =
            std::string("OpenGL map context: ") +
            SDL_GetError();
        destroyOpenGLContext();
        return false;
    }

    if (!makeMapContextCurrent()) {
        destroyOpenGLContext();
        return false;
    }

    SDL_GL_SetSwapInterval(0);

    try {
        backend_ =
            std::make_unique<
                eld::render::OpenGLRenderBackend
            >(gpuWindow_);

        backend_->setPresentEnabled(false);
        backend_->setClearColor({
            0.055f,
            0.063f,
            0.071f,
            1.0f
        });
    }
    catch (const std::exception& exception) {
        error_ = exception.what();
        destroyOpenGLContext();
        return false;
    }

    return true;
}

bool MapViewSurface::ensureOutputTexture(
    SDL_Renderer* renderer,
    int width,
    int height
) {
    if (
        outputTexture_ != nullptr &&
        outputRenderer_ == renderer &&
        width_ == width &&
        height_ == height
    ) {
        return true;
    }

    destroyOutputTexture();

    outputTexture_ =
        SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING,
            width,
            height
        );

    if (outputTexture_ == nullptr) {
        error_ =
            std::string("map viewport texture: ") +
            SDL_GetError();
        return false;
    }

    outputRenderer_ = renderer;
    width_ = width;
    height_ = height;

    SDL_SetTextureBlendMode(
        outputTexture_,
        SDL_BLENDMODE_NONE
    );

    SDL_SetTextureScaleMode(
        outputTexture_,
        SDL_SCALEMODE_NEAREST
    );

    return true;
}

bool MapViewSurface::makeMapContextCurrent() {
    if (
        gpuWindow_ == nullptr ||
        gpuContext_ == nullptr
    ) {
        return false;
    }

    if (
        !SDL_GL_MakeCurrent(
            gpuWindow_,
            gpuContext_
        )
    ) {
        error_ =
            std::string("SDL_GL_MakeCurrent(map): ") +
            SDL_GetError();
        return false;
    }

    return true;
}

bool MapViewSurface::prepare(
    SDL_Renderer* renderer,
    CacheExplorerState& state,
    eld::graphics::GraphicsResources& resources
) {
    if (!state.activeMap.has_value()) {
        return false;
    }

    const int width =
        std::max(
            state.viewportWidth,
            1
        );

    const int height =
        std::max(
            state.viewportHeight,
            1
        );

    const bool dimensionsChanged =
        width != width_ ||
        height != height_ ||
        outputRenderer_ != renderer;

    if (
        !state.mapViewportDirty &&
        !dimensionsChanged &&
        outputTexture_ != nullptr
    ) {
        return true;
    }

    SDL_FlushRenderer(renderer);

    const CurrentGlContext previous =
        captureCurrentGlContext();

    if (!ensureOpenGLContext(width, height)) {
        restoreGlContext(previous);
        return false;
    }

    if (!makeMapContextCurrent()) {
        restoreGlContext(previous);
        return false;
    }

    if (
        width != width_ ||
        height != height_
    ) {
        SDL_SetWindowSize(
            gpuWindow_,
            width,
            height
        );
        SDL_SyncWindow(gpuWindow_);
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
        static_cast<std::uint32_t>(width),
        static_cast<std::uint32_t>(height)
    );

    try {
        pipeline_.render(
            viewState.scene,
            resources,
            *backend_
        );

        backend_->readColorBufferRgba(
            static_cast<std::uint32_t>(width),
            static_cast<std::uint32_t>(height),
            pixels_
        );
    }
    catch (const std::exception& exception) {
        error_ = exception.what();
        restoreGlContext(previous);
        return false;
    }

    restoreGlContext(previous);

    flipReadbackRows(
        width,
        height
    );

    if (
        !ensureOutputTexture(
            renderer,
            width,
            height
        )
    ) {
        return false;
    }

    if (
        !SDL_UpdateTexture(
            outputTexture_,
            nullptr,
            flippedPixels_.data(),
            width * 4
        )
    ) {
        error_ =
            std::string("map viewport upload: ") +
            SDL_GetError();
        return false;
    }

    state.mapViewportDirty = false;
    error_.clear();
    return true;
}

void MapViewSurface::draw(
    SDL_Renderer* renderer,
    const CacheExplorerState& state
) const {
    if (
        outputTexture_ == nullptr ||
        renderer == nullptr
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

    const SDL_FRect destination{
        static_cast<float>(state.viewportX),
        static_cast<float>(state.viewportY),
        static_cast<float>(state.viewportWidth),
        static_cast<float>(state.viewportHeight)
    };

    SDL_RenderTexture(
        renderer,
        outputTexture_,
        nullptr,
        &destination
    );

    drawMapSelectionOverlay(
        renderer,
        state
    );

    SDL_SetRenderClipRect(
        renderer,
        nullptr
    );
}

const std::string& MapViewSurface::error() const {
    return error_;
}

void MapViewSurface::flipReadbackRows(
    int width,
    int height
) {
    const std::size_t rowBytes =
        static_cast<std::size_t>(width) * 4u;

    flippedPixels_.resize(
        pixels_.size()
    );

    for (int y = 0; y < height; ++y) {
        const std::size_t sourceRow =
            static_cast<std::size_t>(
                height - 1 - y
            ) * rowBytes;

        const std::size_t destinationRow =
            static_cast<std::size_t>(y) *
            rowBytes;

        std::memcpy(
            flippedPixels_.data() +
                destinationRow,
            pixels_.data() +
                sourceRow,
            rowBytes
        );
    }
}

void MapViewSurface::destroyOpenGLContext() {
    if (
        backend_ != nullptr &&
        gpuWindow_ != nullptr &&
        gpuContext_ != nullptr
    ) {
        SDL_GL_MakeCurrent(
            gpuWindow_,
            gpuContext_
        );

        backend_.reset();
    }
    else {
        backend_.reset();
    }

    if (gpuContext_ != nullptr) {
        SDL_GL_DestroyContext(
            gpuContext_
        );
        gpuContext_ = nullptr;
    }

    if (gpuWindow_ != nullptr) {
        SDL_DestroyWindow(
            gpuWindow_
        );
        gpuWindow_ = nullptr;
    }
}

void MapViewSurface::destroyOutputTexture() {
    if (outputTexture_ != nullptr) {
        SDL_DestroyTexture(
            outputTexture_
        );
        outputTexture_ = nullptr;
    }

    outputRenderer_ = nullptr;
}

}
