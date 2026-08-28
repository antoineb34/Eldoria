#include "CacheExplorer.h"

#include <algorithm>
#include <cmath>
#include "IdentityKitPreviewBuilder.h"
#include "LocationPreviewBuilder.h"
#include "NpcPreviewBuilder.h"
#include "ItemPreviewBuilder.h"
#include "SpotAnimationPreviewBuilder.h"
#include "FontPreviewBuilder.h"

#include <exception>
#include <limits>
#include <optional>
#include <sstream>
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

void appendInterfaceDump(
    std::ostringstream& stream,
    const eld::interface::InterfaceWidget& widget,
    const eld::interface::InterfaceRepository& repository,
    int depth,
    int x,
    int y
) {
    const std::string indent(
        static_cast<std::size_t>(depth) * 2,
        ' '
    );

    stream
        << indent
        << "id=" << widget.id
        << " type=" << static_cast<int>(widget.type)
        << " action=" << static_cast<int>(widget.actionType)
        << " content=" << widget.contentType
        << " pos=" << x << "," << y
        << " size=" << widget.width << "x" << widget.height
        << " scroll=" << widget.scrollHeight
        << " hidden=" << (widget.hidden ? "yes" : "no")
        << " children=" << widget.children.size()
        << "\n";

    if (!widget.text.empty()) {
        stream << indent << "  text=\"" << widget.text << "\"\n";
    }

    if (!widget.secondaryText.empty()) {
        stream << indent << "  secondaryText=\"" << widget.secondaryText << "\"\n";
    }

    if (!widget.sprite.empty()) {
        stream << indent << "  sprite=\"" << widget.sprite << "\"\n";
    }

    if (!widget.secondarySprite.empty()) {
        stream << indent << "  secondarySprite=\"" << widget.secondarySprite << "\"\n";
    }

    if (widget.modelId.has_value()) {
        stream
            << indent
            << "  model=" << *widget.modelId
            << " zoom=" << widget.modelZoom
            << " rot=" << widget.modelRotationX
            << "," << widget.modelRotationY
            << "\n";
    }

    if (widget.secondaryModelId.has_value()) {
        stream << indent << "  secondaryModel=" << *widget.secondaryModelId << "\n";
    }

    if (widget.animationId.has_value()) {
        stream << indent << "  animation=" << *widget.animationId << "\n";
    }

    if (widget.secondaryAnimationId.has_value()) {
        stream << indent << "  secondaryAnimation=" << *widget.secondaryAnimationId << "\n";
    }

    if (!widget.itemIds.empty()) {
        std::size_t nonEmpty = 0;

        for (const std::uint16_t itemId : widget.itemIds) {
            if (itemId != 0) {
                ++nonEmpty;
            }
        }

        stream
            << indent
            << "  items=" << widget.itemIds.size()
            << " nonEmpty=" << nonEmpty
            << " padding=" << static_cast<int>(widget.inventoryPaddingX)
            << "," << static_cast<int>(widget.inventoryPaddingY)
            << "\n";
    }

    if (!widget.inventorySprites.empty()) {
        stream
            << indent
            << "  inventorySprites=" << widget.inventorySprites.size()
            << "\n";

        for (const eld::interface::InterfaceFileSpriteSlot& slot : widget.inventorySprites) {
            stream
                << indent
                << "    slot=" << static_cast<int>(slot.slot)
                << " pos=" << slot.x << "," << slot.y
                << " sprite=\"" << slot.sprite << "\"\n";
        }
    }

    for (const eld::interface::InterfaceFileChild& child : widget.children) {
        const eld::interface::InterfaceWidget* childWidget =
            repository.find(child.id);

        if (childWidget == nullptr) {
            stream
                << indent
                << "  missing-child id=" << child.id
                << " pos=" << child.x << "," << child.y
                << "\n";

            continue;
        }

        appendInterfaceDump(
            stream,
            *childWidget,
            repository,
            depth + 1,
            x + child.x,
            y + child.y
        );
    }
}

std::string buildInterfaceDump(
    const eld::interface::InterfaceWidget& root,
    const eld::interface::InterfaceRepository& repository
) {
    std::ostringstream stream;

    stream << "Interface subtree dump\n";
    stream << "======================\n";

    appendInterfaceDump(
        stream,
        root,
        repository,
        0,
        0,
        0
    );

    return stream.str();
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

bool populateTextureArchive(
    CacheTreeNode& node,
    const std::vector<std::uint16_t>& textureIds
) {
    constexpr int ConfigIndexId = 0;
    constexpr int TextureArchiveId = 6;

    if (
        node.type == CacheTreeNodeType::Archive &&
        node.indexId == ConfigIndexId &&
        node.archiveId == TextureArchiveId
    ) {
        node.label =
            "Archive 6 - Textures";

        node.children.clear();

        node.children.reserve(
            textureIds.size()
        );

        for (
            const std::uint16_t textureId :
            textureIds
        ) {
            CacheTreeNode textureNode;

            textureNode.type =
                CacheTreeNodeType::Texture;

            textureNode.label =
                "Texture " +
                std::to_string(
                    textureId
                );

            textureNode.key =
                node.key +
                "/texture/" +
                std::to_string(
                    textureId
                );

            textureNode.indexId =
                node.indexId;

            textureNode.archiveId =
                node.archiveId;

            textureNode.fileId =
                static_cast<int>(
                    textureId
                );

            node.children.push_back(
                std::move(
                    textureNode
                )
            );
        }

        return true;
    }

    for (
        CacheTreeNode& child :
        node.children
    ) {
        if (
            populateTextureArchive(
                child,
                textureIds
            )
        ) {
            return true;
        }
    }

    return false;
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


// ELFORGE_NPC_ANIMATION_PREVIEW_V1
void CacheExplorer::resetAnimationPreview() {
    clearNpcActionPreview();

    animationPlayer_.clear();

    animationPreviewKind_ =
        AnimationPreviewKind::None;

    animationSource_.reset();
    animationHandles_.clear();
}

void CacheExplorer::startAnimationPreview(
    const std::optional<std::uint16_t>& sequenceId
) {
    if (
        !sequenceId.has_value() ||
        !animationSource_.has_value()
    ) {
        return;
    }

    const eld::definition::SequenceDefinition* sequence =
        sequenceRepository_.find(
            *sequenceId
        );

    if (
        sequence == nullptr ||
        sequence->frames.empty()
    ) {
        return;
    }

    animationPlayer_.setSequence(
        *sequence
    );

    animationPlayer_.setLooping(
        true
    );

    animationPlayer_.play();

    rebuildAnimationFrame();
}

void CacheExplorer::rebuildAnimationFrame() {
    if (
        !animationSource_.has_value() ||
        !state_.activeModel.has_value()
    ) {
        return;
    }

    const eld::definition::SequenceDefinition* sequence =
        animationPlayer_.sequence();

    if (sequence == nullptr) {
        return;
    }

    const eld::animation::ResolvedAnimationFrame resolved =
        animationPlayer_.currentResolvedFrame();

    if (!resolved) {
        return;
    }

    const eld::graphics::AnimatedModelFrame animated =
        modelAnimator_.apply(
            *animationSource_,
            *resolved.frame,
            *resolved.skeleton
        );

    eld::model::ModelMesh displayMesh =
        animated.mesh;

    if (
        animationPreviewKind_ == AnimationPreviewKind::Location &&
        state_.activeLocation.has_value()
    ) {
        const LocationPreviewBuilder builder;
        builder.prepareAnimatedMesh(
            *state_.activeLocation,
            displayMesh
        );
    }
    else if (
        animationPreviewKind_ == AnimationPreviewKind::SpotAnimation &&
        state_.activeSpotAnimation.has_value()
    ) {
        const SpotAnimationPreviewBuilder builder;
        builder.prepareAnimatedMesh(
            *state_.activeSpotAnimation,
            displayMesh
        );
    }

    state_.activeModel->mesh =
        displayMesh;

    const std::pair<std::uint16_t, std::size_t> key{
        sequence->id,
        animationPlayer_.frameIndex()
    };

    const auto cached =
        animationHandles_.find(key);

    if (cached != animationHandles_.end()) {
        state_.activeModelHandle =
            cached->second;
        return;
    }

    const eld::graphics::ModelHandle handle =
        graphicsResources_.resolveModel(
            displayMesh
        );

    animationHandles_.emplace(
        key,
        handle
    );

    state_.activeModelHandle =
        handle;
}


// ELFORGE_COMPOSITE_ACTION_PREVIEW_V1
void CacheExplorer::clearNpcActionPreview() {
    npcActionEffects_.clear();
    activeNpcAction_.reset();
    state_.presentationObjects.clear();
}


// ELFORGE_CLICK_TARGET_GRID_V1
void CacheExplorer::ensureActionGrid() {
    if (actionGridHandle_.has_value()) {
        return;
    }

    eld::model::ModelMesh grid;

    constexpr int HalfLines = 10;
    constexpr float Spacing = 50.0f;
    constexpr float HalfThickness = 0.75f;

    const float extent =
        static_cast<float>(HalfLines) *
        Spacing;

    const auto addStrip =
        [&grid](
            float x0,
            float z0,
            float x1,
            float z1,
            bool alongX,
            std::uint16_t color
        ) {
            const std::uint32_t base =
                static_cast<std::uint32_t>(
                    grid.vertices.size()
                );

            if (alongX) {
                grid.vertices.push_back({
                    x0,
                    0.0f,
                    z0 - HalfThickness,
                    std::nullopt
                });

                grid.vertices.push_back({
                    x1,
                    0.0f,
                    z1 - HalfThickness,
                    std::nullopt
                });

                grid.vertices.push_back({
                    x1,
                    0.0f,
                    z1 + HalfThickness,
                    std::nullopt
                });

                grid.vertices.push_back({
                    x0,
                    0.0f,
                    z0 + HalfThickness,
                    std::nullopt
                });
            }
            else {
                grid.vertices.push_back({
                    x0 - HalfThickness,
                    0.0f,
                    z0,
                    std::nullopt
                });

                grid.vertices.push_back({
                    x1 - HalfThickness,
                    0.0f,
                    z1,
                    std::nullopt
                });

                grid.vertices.push_back({
                    x1 + HalfThickness,
                    0.0f,
                    z1,
                    std::nullopt
                });

                grid.vertices.push_back({
                    x0 + HalfThickness,
                    0.0f,
                    z0,
                    std::nullopt
                });
            }

            eld::model::Face first;
            first.a = base;
            first.b = base + 1;
            first.c = base + 2;
            first.color = color;

            eld::model::Face second;
            second.a = base;
            second.b = base + 2;
            second.c = base + 3;
            second.color = color;

            grid.faces.push_back(first);
            grid.faces.push_back(second);
        };

    for (
        int line = -HalfLines;
        line <= HalfLines;
        ++line
    ) {
        const float offset =
            static_cast<float>(line) *
            Spacing;

        const std::uint16_t color =
            line == 0
                ? static_cast<std::uint16_t>(127)
                : static_cast<std::uint16_t>(95);

        addStrip(
            -extent,
            offset,
            extent,
            offset,
            true,
            color
        );

        addStrip(
            offset,
            -extent,
            offset,
            extent,
            false,
            color
        );
    }

    actionGridHandle_ =
        graphicsResources_.resolveModel(grid);
}

bool CacheExplorer::placeActionTargetFromViewport(
    float mouseX,
    float mouseY
) {
    if (
        state_.viewportWidth <= 0 ||
        state_.viewportHeight <= 0
    ) {
        return false;
    }

    const float localX =
        mouseX -
        static_cast<float>(state_.viewportX);

    const float localY =
        mouseY -
        static_cast<float>(state_.viewportY);

    const float ndcX =
        localX /
            static_cast<float>(state_.viewportWidth) *
            2.0f -
        1.0f;

    const float ndcY =
        1.0f -
        localY /
            static_cast<float>(state_.viewportHeight) *
            2.0f;

    const float aspect =
        static_cast<float>(state_.viewportWidth) /
        static_cast<float>(state_.viewportHeight);

    const float tanHalfFov =
        std::tan(
            state_.camera.verticalFov *
            0.5f
        );

    const eld::math::Vec3 viewDirection{
        ndcX * aspect * tanHalfFov,
        ndcY * tanHalfFov,
        1.0f
    };

    const eld::math::Mat4 cameraRotation =
        eld::math::Mat4::rotationX(
            state_.camera.rotation.x
        ) *
        eld::math::Mat4::rotationY(
            state_.camera.rotation.y
        ) *
        eld::math::Mat4::rotationZ(
            state_.camera.rotation.z
        );

    const eld::math::Vec4 worldDirection4 =
        cameraRotation.transform({
            viewDirection.x,
            viewDirection.y,
            viewDirection.z,
            0.0f
        });

    const eld::math::Vec3 rayDirection =
        eld::math::Vec3{
            worldDirection4.x,
            worldDirection4.y,
            worldDirection4.z
        }.normalized();

    const eld::math::Vec3 rayOrigin =
        state_.camera.position;

    const eld::math::Mat4 modelMatrix =
        eld::render::buildModelMatrix(
            state_.modelTransform
        );

    const eld::math::Vec3 planeOrigin =
        modelMatrix.transformPoint({
            0.0f,
            0.0f,
            0.0f
        });

    const eld::math::Vec3 planeX =
        modelMatrix.transformPoint({
            1.0f,
            0.0f,
            0.0f
        });

    const eld::math::Vec3 planeZ =
        modelMatrix.transformPoint({
            0.0f,
            0.0f,
            1.0f
        });

    const eld::math::Vec3 xAxis =
        planeX -
        planeOrigin;

    const eld::math::Vec3 zAxis =
        planeZ -
        planeOrigin;

    const eld::math::Vec3 normal =
        xAxis.cross(zAxis).normalized();

    const float denominator =
        normal.dot(rayDirection);

    if (std::abs(denominator) < 0.0001f) {
        return false;
    }

    const float distance =
        normal.dot(
            planeOrigin -
            rayOrigin
        ) /
        denominator;

    if (distance <= 0.0f) {
        return false;
    }

    const eld::math::Vec3 hit =
        rayOrigin +
        rayDirection * distance;

    const eld::math::Vec3 relative =
        hit -
        planeOrigin;

    const float xLengthSquared =
        xAxis.dot(xAxis);

    const float zLengthSquared =
        zAxis.dot(zAxis);

    if (
        xLengthSquared <= 0.000001f ||
        zLengthSquared <= 0.000001f
    ) {
        return false;
    }

    actionTargetLocal_.x =
        relative.dot(xAxis) /
        xLengthSquared;

    actionTargetLocal_.y = 0.0f;

    actionTargetLocal_.z =
        relative.dot(zAxis) /
        zLengthSquared;

    return true;
}

void CacheExplorer::ensureActionTargetMarker() {
    if (actionTargetHandle_.has_value()) {
        return;
    }

    eld::model::ModelMesh marker;

    marker.vertices.resize(5);
    marker.vertices[0].x = 0.0f;
    marker.vertices[0].y = 32.0f;
    marker.vertices[0].z = 0.0f;

    marker.vertices[1].x = -12.0f;
    marker.vertices[1].y = 0.0f;
    marker.vertices[1].z = -12.0f;

    marker.vertices[2].x = 12.0f;
    marker.vertices[2].y = 0.0f;
    marker.vertices[2].z = -12.0f;

    marker.vertices[3].x = 12.0f;
    marker.vertices[3].y = 0.0f;
    marker.vertices[3].z = 12.0f;

    marker.vertices[4].x = -12.0f;
    marker.vertices[4].y = 0.0f;
    marker.vertices[4].z = 12.0f;

    const auto addFace =
        [&marker](
            std::uint32_t a,
            std::uint32_t b,
            std::uint32_t c
        ) {
            eld::model::Face face;
            face.a = a;
            face.b = b;
            face.c = c;
            face.color = 960;
            marker.faces.push_back(face);
        };

    addFace(0, 1, 2);
    addFace(0, 2, 3);
    addFace(0, 3, 4);
    addFace(0, 4, 1);
    addFace(1, 4, 3);
    addFace(1, 3, 2);

    actionTargetHandle_ =
        graphicsResources_.resolveModel(marker);
}

void CacheExplorer::rebuildNpcActionEffect(
    std::size_t effectIndex
) {
    if (effectIndex >= npcActionEffects_.size()) {
        return;
    }

    NpcActionEffectPreview& effect =
        npcActionEffects_.at(effectIndex);

    eld::model::ModelMesh displayMesh =
        effect.sourceMesh;

    if (effect.player) {
        const eld::animation::ResolvedAnimationFrame resolved =
            effect.player->currentResolvedFrame();

        if (resolved) {
            displayMesh =
                modelAnimator_.apply(
                    effect.sourceMesh,
                    *resolved.frame,
                    *resolved.skeleton
                ).mesh;
        }
    }

    const SpotAnimationPreviewBuilder builder;
    builder.prepareAnimatedMesh(
        effect.definition,
        displayMesh
    );

    effect.modelHandle =
        graphicsResources_.resolveModel(displayMesh);
}

void CacheExplorer::startNpcActionPreview(
    const eld::animation::presentation::AnimationBinding& binding
) {
    clearNpcActionPreview();
    activeNpcAction_ = binding;

    if (binding.sequenceId.has_value()) {
        startAnimationPreview(binding.sequenceId);
        animationPlayer_.setLooping(false);
    }

    const SpotAnimationPreviewBuilder previewBuilder;

    for (
        const eld::animation::presentation::AnimationEffectBinding& effectBinding :
        binding.effects
    ) {
        const eld::definition::SpotAnimationDefinition* definition =
            spotAnimationRepository_.find(effectBinding.spotAnimationId);

        if (definition == nullptr) {
            continue;
        }

        std::optional<eld::model::Model> source =
            previewBuilder.buildAnimationSource(
                *definition,
                modelRepository_
            );

        if (!source.has_value()) {
            continue;
        }

        NpcActionEffectPreview effect;
        effect.binding = effectBinding;
        effect.definition = *definition;
        effect.sourceMesh = std::move(source->mesh);

        if (definition->sequenceId.has_value()) {
            const eld::definition::SequenceDefinition* sequence =
                sequenceRepository_.find(*definition->sequenceId);

            if (
                sequence != nullptr &&
                !sequence->frames.empty()
            ) {
                effect.player =
                    std::make_unique<eld::graphics::AnimationPlayer>(
                        animationFrameIndex_
                    );
                effect.player->setSequence(*sequence);
                effect.player->setLooping(effectBinding.projectile);
                effect.player->play();
            }
        }

        npcActionEffects_.push_back(std::move(effect));
        rebuildNpcActionEffect(npcActionEffects_.size() - 1);
    }
}

void CacheExplorer::updateNpcActionEffects(
    std::uint64_t deltaMilliseconds
) {
    state_.presentationObjects.clear();

    if (
        state_.activeNpc.has_value() &&
        showActionGrid_
    ) {
        ensureActionGrid();

        if (actionGridHandle_.has_value()) {
            state_.presentationObjects.push_back({
                *actionGridHandle_,
                state_.modelTransform
            });
        }
    }

    bool showTargetMarker =
        state_.activeNpc.has_value() &&
        showActionGrid_;

    for (
        std::size_t index = 0;
        index < npcActionEffects_.size();
        ++index
    ) {
        NpcActionEffectPreview& effect =
            npcActionEffects_.at(index);

        effect.elapsedMilliseconds += deltaMilliseconds;

        if (
            effect.elapsedMilliseconds <
            effect.binding.delayMilliseconds
        ) {
            continue;
        }

        const std::uint64_t activeMilliseconds =
            effect.elapsedMilliseconds -
            effect.binding.delayMilliseconds;

        if (
            effect.binding.projectile &&
            activeMilliseconds > effect.binding.durationMilliseconds
        ) {
            continue;
        }

        if (
            effect.player &&
            effect.player->update(deltaMilliseconds)
        ) {
            rebuildNpcActionEffect(index);
        }

        if (!effect.modelHandle.has_value()) {
            rebuildNpcActionEffect(index);
        }

        if (!effect.modelHandle.has_value()) {
            continue;
        }

        eld::render::Transform transform =
            state_.modelTransform;

        if (effect.binding.projectile) {
            showTargetMarker = true;

            const float duration =
                static_cast<float>(
                    std::max<std::uint32_t>(
                        effect.binding.durationMilliseconds,
                        1u
                    )
                );

            const float progress =
                std::clamp(
                    static_cast<float>(activeMilliseconds) / duration,
                    0.0f,
                    1.0f
                );

            const eld::math::Mat4 modelMatrix =
                eld::render::buildModelMatrix(
                    state_.modelTransform
                );

            const eld::math::Vec3 source =
                modelMatrix.transformPoint({
                    0.0f,
                    actionPreviewSourceHeight_,
                    0.0f
                });

            const eld::math::Vec3 target =
                modelMatrix.transformPoint(
                    actionTargetLocal_
                );

            const eld::math::Vec3 upPoint =
                modelMatrix.transformPoint({
                    0.0f,
                    1.0f,
                    0.0f
                });

            const eld::math::Vec3 origin =
                modelMatrix.transformPoint({
                    0.0f,
                    0.0f,
                    0.0f
                });

            const eld::math::Vec3 up =
                (upPoint - origin).normalized();

            const eld::math::Vec3 linear =
                source * (1.0f - progress) +
                target * progress;

            const float arc =
                4.0f *
                actionPreviewArcHeight_ *
                progress *
                (1.0f - progress);

            const eld::math::Vec3 position =
                linear +
                up * arc;

            transform.position =
                position;
        }
        else {
            const eld::math::Mat4 modelMatrix =
                eld::render::buildModelMatrix(
                    state_.modelTransform
                );

            transform.position =
                modelMatrix.transformPoint({
                    0.0f,
                    actionPreviewSourceHeight_,
                    0.0f
                });
        }

        state_.presentationObjects.push_back({
            *effect.modelHandle,
            transform
        });
    }

    if (showTargetMarker) {
        ensureActionTargetMarker();

        if (actionTargetHandle_.has_value()) {
            eld::render::Transform target =
                state_.modelTransform;

            const eld::math::Mat4 modelMatrix =
                eld::render::buildModelMatrix(
                    state_.modelTransform
                );

            target.position =
                modelMatrix.transformPoint(
                    actionTargetLocal_
                );

            state_.presentationObjects.push_back({
                *actionTargetHandle_,
                target
            });
        }
    }
}

void CacheExplorer::renderManualNpcActionComposer() {
    if (!state_.activeNpc.has_value()) {
        return;
    }

    if (!ImGui::CollapsingHeader("Action composer")) {
        return;
    }

    ImGui::TextUnformatted(
        "Research/debug: compose a body sequence + SpotAnim without "
        "claiming the relationship came from npc.dat."
    );

    ImGui::InputInt("Body sequence", &manualActionSequenceId_);
    ImGui::InputInt("SpotAnim", &manualActionSpotAnimationId_);
    ImGui::Checkbox("Projectile", &manualActionProjectile_);
    ImGui::InputInt("Effect delay ms", &manualActionDelayMilliseconds_);
    ImGui::InputInt("Projectile duration ms", &manualActionDurationMilliseconds_);

    ImGui::SeparatorText("3D target");

    ImGui::Checkbox(
        "Show ground grid",
        &showActionGrid_
    );

    ImGui::Checkbox(
        "Click viewport to set target",
        &placeActionTargetOnClick_
    );

    if (placeActionTargetOnClick_) {
        ImGui::TextUnformatted(
            "Click anywhere on the visible grid."
        );
    }

    float targetPosition[2]{
        actionTargetLocal_.x,
        actionTargetLocal_.z
    };

    if (
        ImGui::DragFloat2(
            "Target X / Z",
            targetPosition,
            1.0f,
            -2000.0f,
            2000.0f,
            "%.1f"
        )
    ) {
        actionTargetLocal_.x =
            targetPosition[0];

        actionTargetLocal_.z =
            targetPosition[1];
    }

    if (ImGui::Button("Reset target")) {
        actionTargetLocal_ = {
            220.0f,
            0.0f,
            0.0f
        };
    }

    ImGui::SliderFloat(
        "Arc height",
        &actionPreviewArcHeight_,
        0.0f,
        300.0f,
        "%.0f"
    );

    ImGui::SliderFloat(
        "Effect/source height",
        &actionPreviewSourceHeight_,
        -200.0f,
        400.0f,
        "%.0f"
    );

    if (ImGui::Button("Play composed action")) {
        eld::animation::presentation::AnimationBinding binding;
        binding.action =
            eld::animation::presentation::AnimationAction::Attack;

        if (
            manualActionSequenceId_ >= 0 &&
            manualActionSequenceId_ <= 65535
        ) {
            binding.sequenceId =
                static_cast<std::uint16_t>(manualActionSequenceId_);
        }

        if (
            manualActionSpotAnimationId_ >= 0 &&
            manualActionSpotAnimationId_ <= 65535
        ) {
            eld::animation::presentation::AnimationEffectBinding effect;
            effect.spotAnimationId =
                static_cast<std::uint16_t>(manualActionSpotAnimationId_);
            effect.projectile = manualActionProjectile_;
            effect.delayMilliseconds =
                static_cast<std::uint32_t>(
                    std::max(manualActionDelayMilliseconds_, 0)
                );
            effect.durationMilliseconds =
                static_cast<std::uint32_t>(
                    std::max(manualActionDurationMilliseconds_, 1)
                );
            binding.effects.push_back(effect);
        }

        startNpcActionPreview(binding);
    }
}

void CacheExplorer::renderAnimationPlaybackControls() {
    const eld::definition::SequenceDefinition* sequence =
        animationPlayer_.sequence();

    if (sequence == nullptr) {
        ImGui::TextUnformatted("No animation sequence");
        return;
    }

    if (
        ImGui::Button(
            animationPlayer_.isPlaying()
                ? "Pause"
                : "Play"
        )
    ) {
        animationPlayer_.setPlaying(
            !animationPlayer_.isPlaying()
        );
    }

    ImGui::SameLine();

    if (ImGui::Button("< Frame")) {
        animationPlayer_.pause();
        if (animationPlayer_.stepBackward()) {
            rebuildAnimationFrame();
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Frame >")) {
        animationPlayer_.pause();
        if (animationPlayer_.stepForward()) {
            rebuildAnimationFrame();
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Restart")) {
        animationPlayer_.restart();
        rebuildAnimationFrame();
    }

    float speed =
        animationPlayer_.speed();

    ImGui::SetNextItemWidth(180.0f);

    if (
        ImGui::SliderFloat(
            "Speed",
            &speed,
            0.10f,
            3.00f,
            "%.2fx"
        )
    ) {
        animationPlayer_.setSpeed(speed);
    }

    ImGui::SameLine();

    ImGui::Text(
        "seq=%u  frame=%zu/%zu  %ums",
        static_cast<unsigned int>(sequence->id),
        animationPlayer_.frameIndex(),
        animationPlayer_.frameCount() > 0
            ? animationPlayer_.frameCount() - 1
            : 0,
        static_cast<unsigned int>(
            animationPlayer_.currentFrameDurationMilliseconds()
        )
    );
}

void CacheExplorer::renderNpcAnimationControls() {
    if (
        !state_.activeNpc.has_value() ||
        !animationSource_.has_value()
    ) {
        return;
    }

    const eld::animation::presentation::NpcAnimationProfile profile =
        animationPresentationCatalog_.resolveNpc(*state_.activeNpc);

    const auto isMovementAction =
        [](eld::animation::presentation::AnimationAction action) {
            using Action =
                eld::animation::presentation::AnimationAction;

            return
                action == Action::Idle ||
                action == Action::Walk ||
                action == Action::TurnAround ||
                action == Action::TurnLeft ||
                action == Action::TurnRight;
        };

    ImGui::TextUnformatted("MOVEMENT");
    bool firstMovement = true;

    for (
        const eld::animation::presentation::AnimationBinding& binding :
        profile.bindings
    ) {
        if (
            !isMovementAction(binding.action) ||
            !binding.sequenceId.has_value()
        ) {
            continue;
        }

        if (!firstMovement) {
            ImGui::SameLine();
        }

        const std::string label =
            std::string(
                eld::animation::presentation::toString(binding.action)
            );

        if (ImGui::Button(label.c_str())) {
            clearNpcActionPreview();
            startAnimationPreview(binding.sequenceId);
            animationPlayer_.setLooping(true);
        }

        firstMovement = false;
    }

    ImGui::Spacing();
    ImGui::TextUnformatted("ACTIONS");

    bool hasActions = false;
    bool firstAction = true;

    for (
        const eld::animation::presentation::AnimationBinding& binding :
        profile.bindings
    ) {
        if (isMovementAction(binding.action)) {
            continue;
        }

        if (
            !binding.sequenceId.has_value() &&
            binding.effects.empty()
        ) {
            continue;
        }

        hasActions = true;

        if (!firstAction) {
            ImGui::SameLine();
        }

        const std::string label =
            std::string(
                eld::animation::presentation::toString(binding.action)
            );

        if (ImGui::Button(label.c_str())) {
            startNpcActionPreview(binding);
        }

        firstAction = false;
    }

    if (!hasActions) {
        ImGui::TextUnformatted(
            "No authored gameplay actions for this NPC yet."
        );
    }

    if (activeNpcAction_.has_value()) {
        const std::string actionLabel =
            std::string(
                eld::animation::presentation::toString(
                    activeNpcAction_->action
                )
            );

        ImGui::Text(
            "Active action: %s",
            actionLabel.c_str()
        );

        for (
            const eld::animation::presentation::AnimationEffectBinding& effect :
            activeNpcAction_->effects
        ) {
            ImGui::BulletText(
                "SpotAnim %u%s  delay=%ums  duration=%ums",
                static_cast<unsigned int>(effect.spotAnimationId),
                effect.projectile ? " projectile" : " attached",
                static_cast<unsigned int>(effect.delayMilliseconds),
                static_cast<unsigned int>(effect.durationMilliseconds)
            );
        }
    }

    renderAnimationPlaybackControls();
    renderManualNpcActionComposer();
    ImGui::Separator();
}

void CacheExplorer::renderLocationAnimationControls() {
    if (!state_.activeLocation.has_value()) {
        return;
    }

    ImGui::TextUnformatted("OBJECT ANIMATION");

    if (state_.activeLocation->animationId.has_value()) {
        ImGui::Text(
            "Definition sequence: %u",
            static_cast<unsigned int>(
                *state_.activeLocation->animationId
            )
        );
    }
    else {
        ImGui::TextUnformatted("Definition sequence: (none)");
    }

    renderAnimationPlaybackControls();
    ImGui::Separator();
}

void CacheExplorer::renderSpotAnimationControls() {
    if (!state_.activeSpotAnimation.has_value()) {
        return;
    }

    ImGui::TextUnformatted("SPOT ANIMATION");

    if (state_.activeSpotAnimation->sequenceId.has_value()) {
        ImGui::Text(
            "Sequence: %u",
            static_cast<unsigned int>(
                *state_.activeSpotAnimation->sequenceId
            )
        );
    }
    else {
        ImGui::TextUnformatted("Sequence: (none)");
    }

    if (state_.activeSpotAnimation->modelId.has_value()) {
        ImGui::SameLine();
        ImGui::Text(
            "model=%u",
            static_cast<unsigned int>(
                *state_.activeSpotAnimation->modelId
            )
        );
    }

    renderAnimationPlaybackControls();
    ImGui::Separator();
}

void CacheExplorer::renderAnimationControls() {
    if (state_.activeNpc.has_value()) {
        renderNpcAnimationControls();
        return;
    }

    if (state_.activeLocation.has_value()) {
        renderLocationAnimationControls();
        return;
    }

    if (state_.activeSpotAnimation.has_value()) {
        renderSpotAnimationControls();
    }
}

CacheExplorer::CacheExplorer()
    : cache_("cache"),
      animationRepository_(
          cache_.open(
              eld::cache::IndexId::Animations
          )
      ),
      animationFrameIndex_(
          animationRepository_
      ),
      animationPlayer_(
          animationFrameIndex_
      ),
      animationPresentationCatalog_(
          "content/animation_bindings.csv"
      ),
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
      mediaSpriteRepository_(
          cache_.open(
              eld::cache::IndexId::Config
          ),
          4
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
      locationRepository_(
          definitionRepository_.get(
              "loc"
          )
      ),
      npcRepository_(
          definitionRepository_.get(
              "npc"
          )
      ),
      itemRepository_(
          definitionRepository_.get(
              "obj"
          )
      ),
      sequenceRepository_(
          definitionRepository_.get(
              "seq"
          )
      ),
      spotAnimationRepository_(
          definitionRepository_.get(
              "spotanim"
          )
      ),
      varpRepository_(
          definitionRepository_.get("varp")
      ),
      varbitRepository_(
          definitionRepository_.get("varbit")
      ),
      parameterRepository_(
          definitionRepository_.get("param")
      ),
      messageRepository_(
          definitionRepository_.get("mes")
      ),
      messageAnimationRepository_(
          definitionRepository_.get("mesanim")
      ),
      interfaceRepository_(
          cache_.open(
              eld::cache::IndexId::Config
          ),
          3
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

    // Archive 6 in the config index is not useful as a list
    // of hashed raw archive entries.  TextureRepository already
    // knows the semantic texture IDs (0.dat, 1.dat, ...), so expose
    // those IDs directly in ElForge.
    populateTextureArchive(
        state_.rootNode,
        textureRepository_.listIds()
    );

    lastAnimationUpdateMs_ =
        SDL_GetTicks();

    return true;
}

void CacheExplorer::handleEvent(
    const SDL_Event& event
) {
    if (
        !placeActionTargetOnClick_ ||
        !state_.activeNpc.has_value() ||
        event.type != SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event.button.button != SDL_BUTTON_LEFT
    ) {
        return;
    }

    const float mouseX = event.button.x;
    const float mouseY = event.button.y;

    const float viewportLeft =
        static_cast<float>(state_.viewportX);

    const float viewportTop =
        static_cast<float>(state_.viewportY);

    const float viewportRight =
        viewportLeft +
        static_cast<float>(state_.viewportWidth);

    const float viewportBottom =
        viewportTop +
        static_cast<float>(state_.viewportHeight);

    if (
        mouseX < viewportLeft ||
        mouseX >= viewportRight ||
        mouseY < viewportTop ||
        mouseY >= viewportBottom
    ) {
        return;
    }

    placeActionTargetFromViewport(
        mouseX,
        mouseY
    );
}

void CacheExplorer::update() {
    const std::uint64_t now =
        SDL_GetTicks();

    const std::uint64_t delta =
        lastAnimationUpdateMs_ == 0
            ? 0
            : now - lastAnimationUpdateMs_;

    lastAnimationUpdateMs_ =
        now;

    if (
        state_.selection.key !=
        lastSelectedKey_
    ) {
        lastSelectedKey_ =
            state_.selection.key;

        handleSelectionChanged();

        lastAnimationUpdateMs_ =
            now;

        return;
    }

    if (
        animationSource_.has_value() &&
        animationPlayer_.update(
            delta
        )
    ) {
        rebuildAnimationFrame();
    }

    updateNpcActionEffects(
        delta
    );
}

void CacheExplorer::handleSelectionChanged() {
    resetAnimationPreview();

    state_.activeModel.reset();
    state_.activeModelHandle.reset();
    state_.activeTexture.reset();
    state_.activeSprite.reset();
    state_.activeImage.reset();
    state_.activeFont.reset();
    state_.activeFloor.reset();
    state_.activeIdentityKit.reset();
    state_.activeLocation.reset();
    state_.activeNpc.reset();
    state_.activeItem.reset();
    state_.activeSequence.reset();
    state_.activeSpotAnimation.reset();
    state_.activeVarp.reset();
    state_.activeVarbit.reset();
    state_.activeParameter.reset();
    state_.activeMessage.reset();
    state_.activeMessageAnimation.reset();
    state_.activeInterface.reset();
    state_.activeInterfaceDump.clear();

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

        case CacheTreeNodeType::Texture: {
            if (
                state_.selection.fileId < 0 ||
                state_.selection.fileId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const std::uint16_t textureId =
                static_cast<std::uint16_t>(
                    state_.selection.fileId
                );

            try {
                state_.activeTexture =
                    textureRepository_.find(
                        textureId
                    );
            }
            catch (const std::exception&) {
                state_.activeTexture.reset();
            }

            break;
        }

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

        case CacheTreeNodeType::InterfaceDefinition: {
            if (state_.selection.definitionId >= 0) {
                const auto* definition =
                    interfaceRepository_.find(
                        static_cast<std::uint16_t>(
                            state_.selection.definitionId
                        )
                    );

                if (definition != nullptr) {
                    state_.activeInterface =
                        *definition;

                    state_.activeInterfaceDump =
                        buildInterfaceDump(
                            *definition,
                            interfaceRepository_
                        );

                    if (
                        definition->type == 6 &&
                        definition->modelId.has_value()
                    ) {
                        try {
                            state_.activeModel =
                                modelRepository_.find(
                                    *definition->modelId
                                );

                            if (state_.activeModel.has_value()) {
                                state_.activeModelHandle =
                                    graphicsResources_.resolveModel(
                                        *definition->modelId
                                    );
                            }
                        }
                        catch (const std::exception&) {
                            state_.activeModel.reset();
                            state_.activeModelHandle.reset();
                        }
                    }

                    if (!state_.activeModelHandle.has_value()) {
                        state_.activeImage.reset();
                    }
                }
            }

            break;
        }

        case CacheTreeNodeType::MessageDefinition: {
            if (state_.selection.definitionId >= 0) {
                const auto* definition =
                    messageRepository_.find(
                        static_cast<std::uint16_t>(
                            state_.selection.definitionId
                        )
                    );

                if (definition != nullptr) {
                    state_.activeMessage = *definition;
                }
            }

            break;
        }

        case CacheTreeNodeType::MessageAnimationDefinition: {
            if (state_.selection.definitionId >= 0) {
                const auto* definition =
                    messageAnimationRepository_.find(
                        static_cast<std::uint16_t>(
                            state_.selection.definitionId
                        )
                    );

                if (definition != nullptr) {
                    state_.activeMessageAnimation =
                        *definition;
                }
            }

            break;
        }

        case CacheTreeNodeType::ParameterDefinition: {
            if (state_.selection.definitionId >= 0) {
                const auto* definition =
                    parameterRepository_.find(
                        static_cast<std::uint16_t>(
                            state_.selection.definitionId
                        )
                    );

                if (definition != nullptr) {
                    state_.activeParameter =
                        *definition;
                }
            }

            break;
        }

        case CacheTreeNodeType::VarpDefinition: {
            if (state_.selection.definitionId >= 0) {
                const auto* definition =
                    varpRepository_.find(
                        static_cast<std::uint16_t>(
                            state_.selection.definitionId
                        )
                    );

                if (definition != nullptr) {
                    state_.activeVarp = *definition;
                }
            }

            break;
        }

        case CacheTreeNodeType::VarbitDefinition: {
            if (state_.selection.definitionId >= 0) {
                const auto* definition =
                    varbitRepository_.find(
                        static_cast<std::uint16_t>(
                            state_.selection.definitionId
                        )
                    );

                if (definition != nullptr) {
                    state_.activeVarbit = *definition;
                }
            }

            break;
        }

        case CacheTreeNodeType::SpotAnimationDefinition: {
            if (
                state_.selection.definitionId < 0 ||
                state_.selection.definitionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const eld::definition::SpotAnimationDefinition* definition =
                spotAnimationRepository_.find(
                    static_cast<std::uint16_t>(
                        state_.selection.definitionId
                    )
                );

            if (definition != nullptr) {
                state_.activeSpotAnimation =
                    *definition;

                const SpotAnimationPreviewBuilder previewBuilder;

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

                    std::optional<eld::model::Model> animationSource =
                        previewBuilder.buildAnimationSource(*definition, modelRepository_);

                    if (animationSource.has_value()) {
                        animationSource_ = animationSource->mesh;
                        animationPreviewKind_ =
                            AnimationPreviewKind::SpotAnimation;

                        startAnimationPreview(
                            definition->sequenceId
                        );
                    }
                }
            }

            break;
        }

        case CacheTreeNodeType::SequenceDefinition: {
            if (
                state_.selection.definitionId < 0 ||
                state_.selection.definitionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const eld::definition::SequenceDefinition* definition =
                sequenceRepository_.find(
                    static_cast<std::uint16_t>(
                        state_.selection.definitionId
                    )
                );

            if (definition != nullptr) {
                state_.activeSequence =
                    *definition;
            }

            break;
        }

        case CacheTreeNodeType::ItemDefinition: {
            if (
                state_.selection.definitionId < 0 ||
                state_.selection.definitionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const eld::definition::ItemDefinition* definition =
                itemRepository_.find(
                    static_cast<std::uint16_t>(
                        state_.selection.definitionId
                    )
                );

            if (definition != nullptr) {
                state_.activeItem =
                    *definition;

                const ItemPreviewBuilder previewBuilder;

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

        case CacheTreeNodeType::NpcDefinition: {
            if (
                state_.selection.definitionId < 0 ||
                state_.selection.definitionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const eld::definition::NpcDefinition* definition =
                npcRepository_.find(
                    static_cast<std::uint16_t>(
                        state_.selection.definitionId
                    )
                );

            if (definition != nullptr) {
                state_.activeNpc =
                    *definition;

                const NpcPreviewBuilder previewBuilder;

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

                    animationSource_ =
                        state_.activeModel->mesh;

                    animationPreviewKind_ =
                        AnimationPreviewKind::Npc;

                    const std::optional<std::uint16_t>
                        initialSequence =
                            definition->idleAnimationId.has_value()
                                ? definition->idleAnimationId
                                : definition->walkAnimationId;

                    startAnimationPreview(
                        initialSequence
                    );
                }
            }

            break;
        }

        case CacheTreeNodeType::LocationDefinition: {
            if (
                state_.selection.definitionId < 0 ||
                state_.selection.definitionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            const eld::definition::LocationDefinition* definition =
                locationRepository_.find(
                    static_cast<std::uint16_t>(
                        state_.selection.definitionId
                    )
                );

            if (definition != nullptr) {
                state_.activeLocation =
                    *definition;

                const LocationPreviewBuilder previewBuilder;

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

                    std::optional<eld::model::Model> animationSource =
                        previewBuilder.buildAnimationSource(*definition, modelRepository_);

                    if (animationSource.has_value()) {
                        animationSource_ = animationSource->mesh;
                        animationPreviewKind_ =
                            AnimationPreviewKind::Location;

                        startAnimationPreview(
                            definition->animationId
                        );
                    }
                }
            }

            break;
        }

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

            const auto frameId =
                static_cast<std::uint16_t>(
                    selectedFrame
                );

            if (
                state_.selection.archiveId == 1 &&
                !state_.selection.name.empty()
            ) {
                state_.activeSprite =
                    titleSpriteRepository_.find(
                        state_.selection.name,
                        frameId
                    );
            }
            else if (
                state_.selection.archiveId == 4 &&
                state_.selection.fileId >= 0 &&
                state_.selection.fileId <=
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                state_.activeSprite =
                    mediaSpriteRepository_.find(
                        static_cast<std::uint16_t>(
                            state_.selection.fileId
                        ),
                        frameId
                    );
            }

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
        graphicsResources_,
        interfaceRepository_,
        mediaSpriteRepository_
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

    // ELFORGE_NPC_ANIMATION_DRAWER_V1
    viewportPanel_.render(
        state_,
        viewportWidth,
        height,
        [this]() {
            renderAnimationControls();
        }
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
