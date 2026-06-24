#include "CacheExplorer.h"
#include "IdentityKitPreviewBuilder.h"
#include "FontPreviewBuilder.h"

#include <exception>
#include <limits>
#include <optional>
#include <string>
#include <utility>

#include <imgui.h>

namespace eld::elforge {

namespace {

eld::image::RgbaPixel makeColorPixel(
    std::uint32_t rgb
) {
    return eld::image::RgbaPixel{
        static_cast<std::uint8_t>(
            (rgb >> 16) & 0xFF
        ),
        static_cast<std::uint8_t>(
            (rgb >> 8) & 0xFF
        ),
        static_cast<std::uint8_t>(
            rgb & 0xFF
        ),
        255
    };
}

eld::image::Image buildFloorPreview(
    const eld::definition::FloorDefinition& floor
) {
    constexpr std::uint16_t Size = 256;

    eld::image::Image image;

    image.width = Size;
    image.height = Size;
    image.pixels.resize(
        static_cast<std::size_t>(Size) *
        Size
    );

    const eld::image::RgbaPixel primary =
        makeColorPixel(
            floor.rgb.value_or(0)
        );

    const eld::image::RgbaPixel secondary =
        makeColorPixel(
            floor.secondaryRgb.value_or(
                floor.rgb.value_or(0)
            )
        );

    for (
        std::size_t y = 0;
        y < Size;
        y++
    ) {
        for (
            std::size_t x = 0;
            x < Size;
            x++
        ) {
            image.pixels[
                y *
                    Size +
                x
            ] =
                x < Size / 2
                    ? primary
                    : secondary;
        }
    }

    return image;
}

}


bool CacheExplorer::hasAlphaFaces(
    const eld::model::ModelMesh& model
) const {
    for (const eld::model::Face& face : model.faces) {
        if (face.alpha > 0) {
            return true;
        }
    }

    return false;
}

void CacheExplorer::findNextAlphaModel() {
    int startId = 0;

    if (
        state_.activeModel &&
        state_.selection.fileId >= 0
    ) {
        startId =
            state_.selection.fileId + 1;
    }

    const std::vector<std::uint16_t> modelIds =
        modelRepository_.listIds();

    for (const std::uint16_t modelId : modelIds) {
        if (static_cast<int>(modelId) < startId) {
            continue;
        }

        try {
            eld::model::Model model =
                modelRepository_.get(modelId);

            if (!hasAlphaFaces(model.mesh)) {
                continue;
            }

            const eld::graphics::ModelHandle handle =
                graphicsResources_.resolveModel(
                    modelId
                );

            state_.activeModel =
                std::move(model);

            state_.activeModelHandle =
                handle;

            state_.activeTexture.reset();

            state_.selection.type =
                CacheTreeNodeType::Model;

            state_.selection.fileId =
                static_cast<int>(modelId);

            state_.selection.label =
                "Model " +
                std::to_string(modelId);

            state_.selection.key =
                "index/1/file/" +
                std::to_string(modelId);

            lastSelectedKey_ =
                state_.selection.key;

            return;
        }
        catch (const std::exception&) {
            continue;
        }
    }
}

CacheExplorer::CacheExplorer()
    : cache_("cache"),
      textureRepository_(
          cache_.open(
              eld::cache::IndexId::Config
          )
      ),
      modelRepository_(
          cache_.open(
              eld::cache::IndexId::Models
          )
      ),
      titleSpriteRepository_(
          cache_.open(
              eld::cache::IndexId::Config
          ),
          1
      ),

      titleJpegRepository_(
          cache_.open(
              eld::cache::IndexId::Config
          ),
          1
      ),
      titleFontRepository_(
          cache_.open(
              eld::cache::IndexId::Config
          ),
          1
      ),
      definitionRepository_(
          cache_.open(
              eld::cache::IndexId::Config
          ),
          2
      ),
      floorRepository_(
          definitionRepository_.get(
              "flo"
          )
      ),
      identityKitRepository_(
          definitionRepository_.get(
              "idk"
          )
      ),
      graphicsResources_(
          modelRepository_,
          textureRepository_
      ) {
}

bool CacheExplorer::initialize() {
    state_.camera.position = {
        0.0f,
        0.0f,
        -500.0f
    };

    state_.camera.rotation = {
        0.0f,
        0.0f,
        0.0f
    };

    state_.camera.verticalFov =
        1.04719755f;

    state_.camera.nearPlane = 1.0f;
    state_.camera.farPlane = 10000.0f;

    state_.camera.viewportWidth = 1;
    state_.camera.viewportHeight = 1;

    state_.rootNode =
        treeBuilder_.build(
            cache_
        );

    return true;
}

void CacheExplorer::handleEvent(
    const SDL_Event& event
) {
    (void) event;
}

void CacheExplorer::update() {
    if (
        state_.selection.key !=
        lastSelectedKey_
    ) {
        lastSelectedKey_ =
            state_.selection.key;

        handleSelectionChanged();
    }
}

void CacheExplorer::handleSelectionChanged() {
    state_.activeModel.reset();
    state_.activeModelHandle.reset();
    state_.activeTexture.reset();
    state_.activeSprite.reset();
    state_.activeImage.reset();
    state_.activeFont.reset();
    state_.activeFloor.reset();
    state_.activeIdentityKit.reset();

    switch (state_.selection.type) {
        case CacheTreeNodeType::Root:
        case CacheTreeNodeType::Index:
        case CacheTreeNodeType::Archive:
        case CacheTreeNodeType::File:
        case CacheTreeNodeType::ArchiveFile:
            break;

        case CacheTreeNodeType::Model: {
            if (
                state_.selection.fileId < 0 ||
                state_.selection.fileId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const std::uint16_t modelId =
                static_cast<std::uint16_t>(
                    state_.selection.fileId
                );

            try {
                std::optional<eld::model::Model> model =
                    modelRepository_.find(
                        modelId
                    );

                if (model.has_value()) {
                    const eld::graphics::ModelHandle handle =
                        graphicsResources_.resolveModel(
                            modelId
                        );

                    state_.activeModel =
                        std::move(*model);

                    state_.activeModelHandle =
                        handle;
                }
            }
            catch (const std::exception&) {
                state_.activeModel.reset();
                state_.activeModelHandle.reset();
            }

            break;
        }

        case CacheTreeNodeType::Texture:
            break;

        case CacheTreeNodeType::Font: {
            if (
                state_.selection.archiveId != 1 ||
                state_.selection.name.empty()
            ) {
                break;
            }

            state_.activeFont =
                titleFontRepository_.find(
                    state_.selection.name
                );

            if (state_.activeFont.has_value()) {
                const FontPreviewBuilder previewBuilder;

                state_.activeImage =
                    previewBuilder.build(
                        *state_.activeFont
                    );
            }

            break;
        }

        case CacheTreeNodeType::DefinitionGroup:
            break;

        case CacheTreeNodeType::IdentityKitDefinition: {
            if (
                state_.selection.definitionId < 0 ||
                state_.selection.definitionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const eld::definition::IdentityKitDefinition* definition =
                identityKitRepository_.find(
                    static_cast<std::uint16_t>(
                        state_.selection.definitionId
                    )
                );

            if (definition != nullptr) {
                state_.activeIdentityKit =
                    *definition;

                const IdentityKitPreviewBuilder previewBuilder;

                std::optional<eld::model::Model> preview =
                    previewBuilder.build(
                        *definition,
                        modelRepository_
                    );

                if (preview.has_value()) {
                    state_.activeModelHandle =
                        graphicsResources_.resolveModel(
                            preview->mesh
                        );

                    state_.activeModel =
                        std::move(*preview);
                }
            }

            break;
        }

        case CacheTreeNodeType::FloorDefinition: {
            if (
                state_.selection.definitionId < 0 ||
                state_.selection.definitionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const eld::definition::FloorDefinition* floor =
                floorRepository_.find(
                    static_cast<std::uint16_t>(
                        state_.selection.definitionId
                    )
                );

            if (floor != nullptr) {
                state_.activeFloor =
                    *floor;

                if (floor->textureId.has_value()) {
                    state_.activeTexture =
                        textureRepository_.find(
                            *floor->textureId
                        );
                }

                if (
                    !state_.activeTexture.has_value() &&
                    (
                        floor->rgb.has_value() ||
                        floor->secondaryRgb.has_value()
                    )
                ) {
                    state_.activeImage =
                        buildFloorPreview(
                            *floor
                        );
                }
            }

            break;
        }

        case CacheTreeNodeType::Image: {
            if (
                state_.selection.archiveId == 1 &&
                !state_.selection.name.empty()
            ) {
                state_.activeImage =
                    titleJpegRepository_.find(
                        state_.selection.name
                    );
            }

            break;
        }

        case CacheTreeNodeType::Sprite:
        case CacheTreeNodeType::SpriteFrame: {
            if (
                state_.selection.archiveId != 1 ||
                state_.selection.name.empty()
                ) {
                    break;
                }

                const int selectedFrame =
                    state_.selection.frameId >= 0
                        ? state_.selection.frameId
                        : 0;

                if (
                    selectedFrame >
                    std::numeric_limits<std::uint16_t>::max()
                ) {
                    break;
                }

                state_.activeSprite =
                    titleSpriteRepository_.find(
                        state_.selection.name,
                        static_cast<std::uint16_t>(
                            selectedFrame
                        )
                    );

                break;
            }
    }
}

void CacheExplorer::renderViewport(
    SDL_Renderer* renderer
) {
    viewportPanel_.renderViewport(
        renderer,
        state_,
        graphicsResources_
    );
}

void CacheExplorer::renderUi() {
    ImGuiViewport* viewport =
        ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(
        viewport->WorkPos
    );

    ImGui::SetNextWindowSize(
        viewport->WorkSize
    );

    const ImGuiWindowFlags shellFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin(
        "CacheExplorer",
        nullptr,
        shellFlags
    );

    ImGui::TextUnformatted(
        "RuneForge Cache Explorer"
    );

    ImGui::Separator();

    if (ImGui::Button("Find alpha model")) {
        findNextAlphaModel();
    }

    const float treeWidth = 300.0f;
    const float inspectorWidth = 320.0f;
    const float spacing =
        ImGui::GetStyle().ItemSpacing.x;

    const ImVec2 available =
        ImGui::GetContentRegionAvail();

    const float height =
        available.y;

    float viewportWidth =
        available.x -
        treeWidth -
        inspectorWidth -
        spacing * 2.0f;

    if (viewportWidth < 100.0f) {
        viewportWidth = 100.0f;
    }

    treePanel_.render(
        state_,
        treeWidth,
        height
    );

    ImGui::SameLine();

    viewportPanel_.render(
        state_,
        viewportWidth,
        height
    );

    ImGui::SameLine();

    inspectorPanel_.render(
        state_,
        inspectorWidth,
        height
    );

    ImGui::End();
}

}
