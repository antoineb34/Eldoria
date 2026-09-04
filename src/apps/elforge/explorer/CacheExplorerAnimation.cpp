#include "ui/ElForgeTheme.h"
#include "explorer/CacheExplorer.h"
#include "views/animation/AnimationHudLayout.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <imgui.h>

#include "views/location/LocationView.h"
#include "views/npc/NpcView.h"
#include "views/spot_animation/SpotAnimationView.h"
#include "ui/IconButton.h"

namespace eld::elforge {

namespace {

std::string actionBindingLabel(
    const eld::animation::presentation::AnimationBinding& binding
) {
    std::string label(
        eld::animation::presentation::toString(binding.action)
    );

    if (!binding.variant.empty()) {
        label += " - ";
        label += binding.variant;
    }

    return label;
}

}

void CacheExplorer::resetAnimationView() {
    clearNpcActionView();

    animationPlayer_.clear();

    animationTargetKind_ =
        AnimationTargetKind::None;

    animationSource_.reset();
    animationHandles_.clear();
}

void CacheExplorer::rebuildAnimationPreviewUses() {
    AnimationViewState& view =
        state_.animationView;

    view.previewUseIndices.clear();
    view.selectedPreviewUseIndex = 0;
    view.activePreviewUseIndex =
        AnimationViewState::NoActivePreview;
    view.previewStatus.clear();

    if (!state_.activeAnimation.has_value()) {
        return;
    }

    const AnimationInspection& inspection =
        *state_.activeAnimation;

    for (
        std::size_t index = 0;
        index < inspection.uses.size();
        ++index
    ) {
        const AnimationUse& use =
            inspection.uses[index];

        // First useful version:
        // entities with an existing, proven standalone
        // animation-source construction path.
        if (
            use.source == "NPC" ||
            use.source == "Location" ||
            use.source == "SpotAnim"
        ) {
            view.previewUseIndices.push_back(
                index
            );
        }
    }

    if (view.previewUseIndices.empty()) {
        view.previewStatus =
            "No previewable 3D model usage found";
    }
    else {
        view.previewStatus =
            std::to_string(
                view.previewUseIndices.size()
            ) +
            " previewable 3D use(s)";
    }
}


bool CacheExplorer::activateAnimationPreviewUse(
    std::size_t previewIndex
) {
    if (!state_.activeAnimation.has_value()) {
        return false;
    }

    AnimationViewState& viewState =
        state_.animationView;

    if (
        previewIndex >=
        viewState.previewUseIndices.size()
    ) {
        return false;
    }

    const AnimationInspection& inspection =
        *state_.activeAnimation;

    const std::size_t useIndex =
        viewState.previewUseIndices[
            previewIndex
        ];

    if (useIndex >= inspection.uses.size()) {
        return false;
    }

    const AnimationUse& use =
        inspection.uses[useIndex];

    // Mark this attempt consumed even if its model is bad.
    // That prevents repeatedly rebuilding a broken usage
    // every frame.
    viewState.activePreviewUseIndex =
        previewIndex;

    const auto sequence =
        sequenceRepository_.find(
            use.sequenceId
        );

    if (
        !sequence ||
        sequence->frames.empty()
    ) {
        viewState.previewStatus =
            "Sequence " +
            std::to_string(use.sequenceId) +
            " is unavailable or empty";

        return false;
    }

    // Reset only the active presentation runtime.
    // The selected raw AnimationInspection stays active.
    resetAnimationView();

    state_.activeNpc.reset();
    state_.activeLocation.reset();
    state_.activeSpotAnimation.reset();
    state_.activeItem.reset();

    state_.activeModel.reset();
    state_.activeModelHandle.reset();

    if (use.source == "NPC") {
        const auto definition =
            npcRepository_.find(
                use.sourceId
            );

        if (!definition) {
            viewState.previewStatus =
                "NPC " +
                std::to_string(use.sourceId) +
                " was not found";

            return false;
        }

        const NpcView view;

        std::optional<eld::model::Model> model =
            view.build(
                *definition,
                modelRepository_
            );

        if (!model.has_value()) {
            viewState.previewStatus =
                "Could not build NPC " +
                std::to_string(use.sourceId);

            return false;
        }

        state_.activeNpc =
            *definition;

        state_.activeModel =
            std::move(*model);

        animationSource_ =
            *state_.activeModel;

        animationTargetKind_ =
            AnimationTargetKind::Npc;
    }
    else if (use.source == "Location") {
        const auto definition =
            locationRepository_.find(
                use.sourceId
            );

        if (!definition.has_value()) {
            viewState.previewStatus =
                "Location " +
                std::to_string(use.sourceId) +
                " was not found";

            return false;
        }

        const LocationView view;

        std::optional<eld::model::Model> model =
            view.build(
                *definition,
                modelRepository_
            );

        std::optional<eld::model::Model> animationSource =
            view.buildAnimationSource(
                *definition,
                modelRepository_
            );

        if (
            !model.has_value() ||
            !animationSource.has_value()
        ) {
            viewState.previewStatus =
                "Could not build location " +
                std::to_string(use.sourceId);

            return false;
        }

        state_.activeLocation =
            *definition;

        state_.activeModel =
            std::move(*model);

        animationSource_ =
            *animationSource;

        animationTargetKind_ =
            AnimationTargetKind::Location;
    }
    else if (use.source == "SpotAnim") {
        const auto definition =
            spotAnimationRepository_.find(
                use.sourceId
            );

        if (!definition) {
            viewState.previewStatus =
                "Spot animation " +
                std::to_string(use.sourceId) +
                " was not found";

            return false;
        }

        const SpotAnimationView view;

        std::optional<eld::model::Model> model =
            view.build(
                *definition,
                modelRepository_
            );

        std::optional<eld::model::Model> animationSource =
            view.buildAnimationSource(
                *definition,
                modelRepository_
            );

        if (
            !model.has_value() ||
            !animationSource.has_value()
        ) {
            viewState.previewStatus =
                "Could not build spot animation " +
                std::to_string(use.sourceId);

            return false;
        }

        state_.activeSpotAnimation =
            *definition;

        state_.activeModel =
            std::move(*model);

        animationSource_ =
            *animationSource;

        animationTargetKind_ =
            AnimationTargetKind::SpotAnimation;
    }
    else {
        viewState.previewStatus =
            "This usage does not have a 3D preview yet";

        return false;
    }

    state_.activeModelHandle =
        graphicsResources_.resolveModel(
            *state_.activeModel
        );

    startAnimationView(
        use.sequenceId
    );

    std::string label =
        use.source +
        " " +
        std::to_string(use.sourceId);

    if (!use.sourceName.empty()) {
        label += " - ";
        label += use.sourceName;
    }

    label +=
        " | " +
        use.role +
        " | seq=" +
        std::to_string(use.sequenceId);

    viewState.previewStatus =
        std::move(label);

    return true;
}


void CacheExplorer::startAnimationView(
    const std::optional<std::uint16_t>& sequenceId
) {
    if (
        !sequenceId.has_value() ||
        !animationSource_.has_value()
    ) {
        return;
    }

    const auto sequence =
        sequenceRepository_.find(
            *sequenceId
        );

    if (
        !sequence ||
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

    const auto sequence =
        animationPlayer_.sequence();

    if (!sequence) {
        return;
    }

    const auto resolved =
        animationPlayer_.currentFrame();

    if (!resolved) {
        return;
    }

    const eld::render::AnimatedModelFrame animated =
        modelAnimator_.apply(
            *animationSource_,
            resolved->frame,
            resolved->skeleton
        );

    eld::model::Model displayMesh =
        animated.mesh;

    if (
        animationTargetKind_ == AnimationTargetKind::Location &&
        state_.activeLocation.has_value()
    ) {
        const LocationView view;
        view.prepareAnimatedMesh(
            *state_.activeLocation,
            displayMesh
        );
    }
    else if (
        animationTargetKind_ == AnimationTargetKind::SpotAnimation &&
        state_.activeSpotAnimation.has_value()
    ) {
        const SpotAnimationView view;
        view.prepareAnimatedMesh(
            *state_.activeSpotAnimation,
            displayMesh
        );
    }

    *state_.activeModel =
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

    const eld::render::ModelHandle handle =
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


void CacheExplorer::clearNpcActionView() {
    npcActionEffects_.clear();
    activeNpcAction_.reset();
    activeItemAction_.reset();
    state_.presentationObjects.clear();
}


void CacheExplorer::ensureActionGrid() {
    if (actionGridHandle_.has_value()) {
        return;
    }

    eld::model::Model grid;

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

    // The visible editor grid is stationary world space, so target picking
    // must use that same world plane. This keeps the clicked point fixed while
    // the selected NPC/model turns independently.
    const eld::math::Vec3 planeOrigin{
        0.0f,
        0.0f,
        0.0f
    };

    const eld::math::Vec3 planeNormal{
        0.0f,
        1.0f,
        0.0f
    };

    const float denominator =
        planeNormal.dot(
            rayDirection
        );

    if (std::abs(denominator) < 0.0001f) {
        return false;
    }

    const float distance =
        planeNormal.dot(
            planeOrigin -
            rayOrigin
        ) /
        denominator;

    if (distance <= 0.0f) {
        return false;
    }

    actionTargetWorld_ =
        rayOrigin +
        rayDirection *
            distance;

    // Keep the tool target exactly on the editor floor.
    actionTargetWorld_.y = 0.0f;

    if (activeNpcAction_.has_value()) {
        faceNpcTowardActionTarget();
    }

    return true;
}

void CacheExplorer::ensureActionTargetMarker() {
    if (actionTargetHandle_.has_value()) {
        return;
    }

    eld::model::Model marker;

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

    NpcActionEffectState& effect =
        npcActionEffects_.at(effectIndex);

    eld::model::Model displayMesh =
        effect.sourceMesh;

    if (effect.player) {
        const auto resolved =
            effect.player->currentFrame();

        if (resolved) {
            displayMesh =
                modelAnimator_.apply(
                    effect.sourceMesh,
                    resolved->frame,
                    resolved->skeleton
                ).mesh;
        }
    }

    const SpotAnimationView view;
    view.prepareAnimatedMesh(
        effect.definition,
        displayMesh
    );

    effect.modelHandle =
        graphicsResources_.resolveModel(displayMesh);
}


void CacheExplorer::faceNpcTowardActionTarget() {
    if (!state_.activeNpc.has_value()) {
        return;
    }

    const float deltaX =
        actionTargetWorld_.x -
        state_.modelTransform.position.x;

    const float deltaZ =
        actionTargetWorld_.z -
        state_.modelTransform.position.z;

    const float horizontalDistanceSquared =
        deltaX * deltaX +
        deltaZ * deltaZ;

    if (horizontalDistanceSquared <= 0.0001f) {
        return;
    }

    // atan2 uses +X as zero heading, while classic NPC models are authored
    // facing +Z at zero yaw. Subtract a quarter-turn so the model's forward
    // axis points at the selected world-space target.
    constexpr float NpcForwardAxisOffset =
        1.57079632679f;

    state_.modelTransform.rotation.y =
        std::atan2(
            -deltaZ,
            deltaX
        ) -
        NpcForwardAxisOffset;
}

void CacheExplorer::appendActionEffects(
    const eld::animation::presentation::AnimationBinding& binding
) {
    const SpotAnimationView view;

    for (
        const eld::animation::presentation::AnimationEffectBinding& effectBinding :
        binding.effects
    ) {
        const auto definition =
            spotAnimationRepository_.find(effectBinding.spotAnimationId);

        if (!definition) {
            continue;
        }

        std::optional<eld::model::Model> source =
            view.buildAnimationSource(
                *definition,
                modelRepository_
            );

        if (!source.has_value()) {
            continue;
        }

        NpcActionEffectState effect;
        effect.binding = effectBinding;
        effect.definition = *definition;
        effect.sourceMesh = std::move(*source);

        if (definition->sequenceId.has_value()) {
            const auto sequence =
                sequenceRepository_.find(*definition->sequenceId);

            if (
                sequence &&
                !sequence->frames.empty()
            ) {
                effect.player =
                    std::make_unique<eld::render::AnimationPlayer>(
                        animationFrameTable_
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

void CacheExplorer::startNpcActionView(
    const eld::animation::presentation::AnimationBinding& binding
) {
    clearNpcActionView();
    activeNpcAction_ = binding;

    faceNpcTowardActionTarget();

    if (binding.sequenceId.has_value()) {
        startAnimationView(binding.sequenceId);
        animationPlayer_.setLooping(false);
    }

    appendActionEffects(binding);
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
        state_.activeNpc.has_value();

    for (
        std::size_t index = 0;
        index < npcActionEffects_.size();
        ++index
    ) {
        NpcActionEffectState& effect =
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

        // Classic RuneScape launch/impact SpotAnims are transient. Projectile
        // graphics live for the projectile flight, while non-projectile
        // SpotAnims disappear when their one-shot sequence completes. Static
        // SpotAnims have no sequence to signal completion, so use the authored
        // binding duration as their lifetime instead of leaving them attached
        // to the NPC forever.
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

        if (!effect.binding.projectile) {
            if (
                effect.player &&
                !effect.player->isPlaying()
            ) {
                continue;
            }

            if (
                !effect.player &&
                activeMilliseconds > effect.binding.durationMilliseconds
            ) {
                continue;
            }
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

            const eld::math::Vec3 origin =
                modelMatrix.transformPoint({
                    0.0f,
                    0.0f,
                    0.0f
                });

            const eld::math::Vec3 upPoint =
                modelMatrix.transformPoint({
                    0.0f,
                    1.0f,
                    0.0f
                });

            const eld::math::Vec3 up =
                (upPoint - origin).normalized();

            // Mirror the classic client projectile setup, but do not apply a
            // human-sized server height blindly to tiny displayed actors. RuneTek
            // projectile heights are scene-space values; some reconstructed
            // bindings only tell us the projectile identity, not actor-specific
            // launch geometry. Cap the requested launch point to the current
            // displayed actor's upper body in graphics space. Exact lower values
            // (for example troll rocks) remain unchanged when
            // they already fall below this cap.
            float sourceHeight =
                static_cast<float>(
                    effect.binding.projectileStartHeight * 4u
                );

            if (
                animationSource_.has_value() &&
                !animationSource_->vertices.empty()
            ) {
                float minGraphicsY =
                    -animationSource_->vertices.front().y;
                float maxGraphicsY =
                    minGraphicsY;

                for (
                    const eld::model::Vertex& vertex :
                    animationSource_->vertices
                ) {
                    const float graphicsY =
                        -vertex.y;

                    minGraphicsY =
                        std::min(minGraphicsY, graphicsY);
                    maxGraphicsY =
                        std::max(maxGraphicsY, graphicsY);
                }

                const float bodyHeight =
                    maxGraphicsY - minGraphicsY;

                if (bodyHeight > 1.0f) {
                    constexpr float UpperBodyFraction =
                        0.78f;

                    const float upperBodyY =
                        minGraphicsY +
                        bodyHeight * UpperBodyFraction;

                    sourceHeight =
                        std::min(
                            sourceHeight,
                            upperBodyY
                        );
                }
            }

            const eld::math::Vec3 sourceLocal{
                0.0f,
                sourceHeight,
                0.0f
            };

            eld::math::Vec3 source =
                modelMatrix.transformPoint(
                    sourceLocal
                );

            const eld::math::Vec3 target =
                actionTargetWorld_ +
                up * static_cast<float>(
                    effect.binding.projectileEndHeight * 4u
                );

            eld::math::Vec3 horizontalDelta =
                target - source;
            horizontalDelta.y = 0.0f;

            const float horizontalDistance =
                std::sqrt(
                    horizontalDelta.x * horizontalDelta.x +
                    horizontalDelta.z * horizontalDelta.z
                );

            if (horizontalDistance > 0.0001f) {
                const float startDistance =
                    static_cast<float>(
                        effect.binding.projectileStartDistance
                    );

                source.x +=
                    horizontalDelta.x / horizontalDistance *
                    startDistance;
                source.z +=
                    horizontalDelta.z / horizontalDistance *
                    startDistance;
            }

            const float time =
                progress * duration;

            const float velocityX =
                (target.x - source.x) / duration;
            const float velocityZ =
                (target.z - source.z) / duration;

            const float horizontalSpeed =
                std::sqrt(
                    velocityX * velocityX +
                    velocityZ * velocityZ
                );

            // 0.02454369 is pi / 128, matching the old client. Its vertical
            // axis points the opposite way, so the sign is flipped here.
            const float initialVelocityY =
                horizontalSpeed *
                std::tan(
                    static_cast<float>(effect.binding.projectileSlope) *
                    0.02454369f
                );

            const float accelerationY =
                2.0f *
                (
                    target.y -
                    source.y -
                    initialVelocityY * duration
                ) /
                (duration * duration);

            eld::math::Vec3 position;
            position.x = source.x + velocityX * time;
            position.z = source.z + velocityZ * time;
            position.y =
                source.y +
                initialVelocityY * time +
                0.5f * accelerationY * time * time;

            transform.position =
                position;

            // The classic 317 client does not inherit the shooter's model
            // rotation.  Each projectile owns its orientation: yaw follows
            // horizontal velocity and pitch follows the instantaneous vertical
            // velocity (which changes as the ballistic acceleration is
            // applied).  In the original client these were stored in 0..2047
            // angle units; the formulas below are the same angles in radians.
            constexpr float ProjectileForwardAxisOffset =
                1.57079632679f;

            transform.rotation.y =
                std::atan2(
                    -velocityZ,
                    velocityX
                ) -
                ProjectileForwardAxisOffset;

            const float currentVelocityY =
                initialVelocityY +
                accelerationY * time;

            // Eldoria uses an upward-positive world Y axis and its transform
            // rotation convention already matches that axis. Do not carry
            // over the old client's downward-positive vertical sign a second
            // time here: pitch follows the instantaneous flight velocity.
            transform.rotation.x =
                std::atan2(
                    currentVelocityY,
                    horizontalSpeed
                );

            transform.rotation.z = 0.0f;
        }
        else if (effect.binding.target) {
            showTargetMarker = true;
            transform.position =
                actionTargetWorld_;
        }
        else {
            const eld::math::Mat4 modelMatrix =
                eld::render::buildModelMatrix(
                    state_.modelTransform
                );

            transform.position =
                modelMatrix.transformPoint({
                    0.0f,
                    actionViewSourceHeight_,
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
                actionTargetWorld_;

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

    ImGui::SeparatorText("PRESETS");

    const eld::animation::presentation::NpcAnimationProfile presetProfile =
        animationPresentationCatalog_.resolveNpc(
            *state_.activeNpc
        );

    const auto isMovementPreset =
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

    bool firstPreset = true;
    bool hasPreset = false;

    for (
        const eld::animation::presentation::AnimationBinding& preset :
        presetProfile.bindings
    ) {
        if (
            isMovementPreset(preset.action) ||
            (
                !preset.sequenceId.has_value() &&
                preset.effects.empty()
            )
        ) {
            continue;
        }

        hasPreset = true;

        if (!firstPreset) {
            ImGui::SameLine();
        }

        const std::string presetLabel =
            std::string("Load ") +
            actionBindingLabel(preset);

        if (ImGui::Button(presetLabel.c_str())) {
            manualActionAction_ =
                preset.action;

            manualActionSequenceId_ =
                preset.sequenceId.has_value()
                    ? static_cast<int>(
                          *preset.sequenceId
                      )
                    : -1;

            manualActionSpotAnimationId_ = -1;
            manualActionProjectile_ = true;
            manualActionDelayMilliseconds_ = 0;
            manualActionDurationMilliseconds_ = 700;

            if (!preset.effects.empty()) {
                const auto& effect =
                    preset.effects.front();

                manualActionSpotAnimationId_ =
                    static_cast<int>(
                        effect.spotAnimationId
                    );

                manualActionProjectile_ =
                    effect.projectile;

                manualActionDelayMilliseconds_ =
                    static_cast<int>(
                        effect.delayMilliseconds
                    );

                manualActionDurationMilliseconds_ =
                    static_cast<int>(
                        effect.durationMilliseconds
                    );
            }
        }

        firstPreset = false;
    }

    if (!hasPreset) {
        ImGui::TextDisabled(
            "No authored action presets for this NPC."
        );
    }

    ImGui::TextDisabled(
        "Preset values are presentation bindings, not npc.dat ownership."
    );

    ImGui::InputInt("Body sequence", &manualActionSequenceId_);
    ImGui::InputInt("SpotAnim", &manualActionSpotAnimationId_);
    ImGui::Checkbox("Projectile", &manualActionProjectile_);
    ImGui::InputInt("Effect delay ms", &manualActionDelayMilliseconds_);
    ImGui::InputInt("Projectile duration ms", &manualActionDurationMilliseconds_);

    ImGui::SeparatorText("3D target");

    ImGui::Checkbox(
        "Show editor grid",
        &state_.showEditorGrid
    );

    ImGui::Checkbox(
        "Click viewport to set target",
        &placeActionTargetOnClick_
    );

    ImGui::Checkbox(
        "Lock NPC facing to target",
        &lockNpcFacingToActionTarget_
    );

    if (
        lockNpcFacingToActionTarget_ &&
        activeNpcAction_.has_value()
    ) {
        faceNpcTowardActionTarget();
    }

    if (placeActionTargetOnClick_) {
        ImGui::TextUnformatted(
            "Click anywhere on the visible grid."
        );
    }

    float targetPosition[2]{
        actionTargetWorld_.x,
        actionTargetWorld_.z
    };

    if (
        ImGui::DragFloat2(
            "Target world X / Z",
            targetPosition,
            1.0f,
            -2000.0f,
            2000.0f,
            "%.1f"
        )
    ) {
        actionTargetWorld_.x =
            targetPosition[0];

        actionTargetWorld_.z =
            targetPosition[1];

        if (activeNpcAction_.has_value()) {
            faceNpcTowardActionTarget();
        }
    }

    if (ImGui::Button("Reset target")) {
        const float yaw =
            state_.modelTransform.rotation.y;

        actionTargetWorld_ = {
            state_.modelTransform.position.x +
                std::cos(yaw) * 220.0f,
            0.0f,
            state_.modelTransform.position.z -
                std::sin(yaw) * 220.0f
        };

        if (activeNpcAction_.has_value()) {
            faceNpcTowardActionTarget();
        }
    }

    ImGui::SliderFloat(
        "Arc height",
        &actionViewArcHeight_,
        0.0f,
        300.0f,
        "%.0f"
    );

    ImGui::SliderFloat(
        "Effect/source height",
        &actionViewSourceHeight_,
        -200.0f,
        400.0f,
        "%.0f"
    );

    if (ImGui::Button("Play composed action")) {
        eld::animation::presentation::AnimationBinding binding;
        binding.action =
            manualActionAction_;

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

        startNpcActionView(binding);
    }
}

void CacheExplorer::renderAnimationPlayerHud() {
    const auto sequence =
        animationPlayer_.sequence();

    if (!sequence) {
        return;
    }

    const std::size_t frameCount =
        animationPlayer_.frameCount();

    const std::size_t frameIndex =
        animationPlayer_.frameIndex();

    const auto currentResolved =
            animationPlayer_.
                currentFrame();

    const ImVec2 viewportPosition{
        static_cast<float>(
            state_.viewportX
        ),
        static_cast<float>(
            state_.viewportY
        )
    };

    const ImVec2 viewportSize{
        static_cast<float>(
            state_.viewportWidth
        ),
        static_cast<float>(
            state_.viewportHeight
        )
    };

        constexpr float HorizontalInset =
        animation_hud::HorizontalInset;

    const float timelineWidth =
        std::max(
            viewportSize.x -
                HorizontalInset *
                    2.0f,
            100.0f
        );

    constexpr float TimelineHitHeight =
        24.0f;

    // Animation playback is rendered from inside
    // ##AnimationBottomHud, so its top edge is the timeline
    // anchor. No duplicated HUD-height math required.
    const float timelineY =
        state_.activeAnimation.has_value()
            ? ImGui::GetWindowPos().y
            : viewportPosition.y +
                viewportSize.y -
                154.0f;

    // --------------------------------------------------------
    // THIN TIMELINE
    // --------------------------------------------------------

    ImGui::SetCursorScreenPos(
        ImVec2(
            viewportPosition.x +
                HorizontalInset,
            timelineY
        )
    );

    ImGui::InvisibleButton(
        "##AnimationTimeline",
        ImVec2(
            timelineWidth,
            TimelineHitHeight
        )
    );

    const ImVec2 minimum =
        ImGui::GetItemRectMin();

    const ImVec2 maximum =
        ImGui::GetItemRectMax();

    ImDrawList* drawList =
        ImGui::GetWindowDrawList();

    const float lineStart =
        minimum.x + 3.0f;

    const float lineEnd =
        maximum.x - 3.0f;

    const float centerY =
        state_.activeAnimation.has_value()
            ? minimum.y + 5.0f
            : (
                  minimum.y +
                  maximum.y
              ) *
                  0.5f;

    drawList->AddLine(
        ImVec2(
            lineStart,
            centerY
        ),
        ImVec2(
            lineEnd,
            centerY
        ),
        ImGui::GetColorU32(
            ImGuiCol_Separator
        ),
        4.0f
    );

    if (frameCount > 0) {
        const float playheadRatio =
            (
                static_cast<float>(
                    frameIndex +
                    1
                )
            ) /
            static_cast<float>(
                frameCount
            );

        const float playheadX =
            lineStart +
            (
                lineEnd -
                lineStart
            ) *
            playheadRatio;

        drawList->AddLine(
            ImVec2(
                lineStart,
                centerY
            ),
            ImVec2(
                playheadX,
                centerY
            ),
            ImGui::GetColorU32(
                ImGuiCol_SliderGrabActive
            ),
            7.0f
        );

        if (frameCount <= 64) {
            for (
                std::size_t index = 0;
                index <= frameCount;
                ++index
            ) {
                const float x =
                    lineStart +
                    (
                        lineEnd -
                        lineStart
                    ) *
                    (
                        static_cast<float>(
                            index
                        ) /
                        static_cast<float>(
                            frameCount
                        )
                    );

                drawList->AddLine(
                    ImVec2(
                        x,
                        centerY - 4.0f
                    ),
                    ImVec2(
                        x,
                        centerY + 8.0f
                    ),
                    ImGui::GetColorU32(
                        ImGuiCol_TextDisabled
                    ),
                    1.4f
                );
            }
        }

        drawList->AddCircleFilled(
            ImVec2(
                playheadX,
                centerY
            ),
            5.0f,
            ImGui::GetColorU32(
                ImGuiCol_SliderGrabActive
            )
        );
    }

    // Hovered frame metadata.
    if (
        frameCount > 0 &&
        ImGui::IsItemHovered()
    ) {
        const float ratio =
            std::clamp(
                (
                    ImGui::GetIO().MousePos.x -
                    lineStart
                ) /
                std::max(
                    lineEnd -
                        lineStart,
                    1.0f
                ),
                0.0f,
                0.999999f
            );

        const std::size_t hoverIndex =
            std::min(
                static_cast<std::size_t>(
                    ratio *
                    static_cast<float>(
                        frameCount
                    )
                ),
                frameCount - 1
            );

        eld::render::AnimationPlayer hoverPlayer(
            animationFrameTable_
        );

        hoverPlayer.setSequence(
            *sequence
        );

        hoverPlayer.setLooping(
            false
        );

        hoverPlayer.pause();

        for (
            std::size_t index = 0;
            index < hoverIndex;
            ++index
        ) {
            if (!hoverPlayer.stepForward()) {
                break;
            }
        }

        const auto hoverResolved =
                hoverPlayer.
                    currentFrame();

        ImGui::BeginTooltip();

        ImGui::Text(
            "Frame %zu / %zu",
            hoverIndex + 1,
            frameCount
        );

        if (hoverResolved) {
            ImGui::Separator();

            ImGui::Text(
                "Global frame: %u",
                static_cast<unsigned int>(
                    hoverResolved->frame.id
                )
            );

            ImGui::Text(
                "Duration: %u ms",
                static_cast<unsigned int>(
                    hoverPlayer.
                        currentFrameDurationMilliseconds()
                )
            );

            ImGui::Text(
                "Transforms: %zu",
                hoverResolved->frame.
                    transforms.size()
            );
        }

        ImGui::EndTooltip();
    }

    // Seek by clicking timeline.
    if (
        frameCount > 0 &&
        ImGui::IsItemClicked()
    ) {
        const float ratio =
            std::clamp(
                (
                    ImGui::GetIO().MousePos.x -
                    lineStart
                ) /
                std::max(
                    lineEnd -
                        lineStart,
                    1.0f
                ),
                0.0f,
                0.999999f
            );

        const std::size_t target =
            std::min(
                static_cast<std::size_t>(
                    ratio *
                    static_cast<float>(
                        frameCount
                    )
                ),
                frameCount - 1
            );

        animationPlayer_.pause();
        animationPlayer_.restart();
        animationPlayer_.pause();

        for (
            std::size_t index = 0;
            index < target;
            ++index
        ) {
            if (
                !animationPlayer_.
                    stepForward()
            ) {
                break;
            }
        }

        rebuildAnimationFrame();
    }



    // --------------------------------------------------------
    // TRANSPORT CARD
    // --------------------------------------------------------

    const animation_hud::BottomRow hudLayout =
        animation_hud::bottomRow(
            viewportSize.x
        );

    constexpr float TransportWidth =
        animation_hud::TransportWidth;

    constexpr float TransportHeight =
        animation_hud::CardHeight;

    constexpr float BottomInset =
        animation_hud::BottomInset;

    const float TransportX =
        hudLayout.transportX;

    const float transportY =
        std::max(
            viewportSize.y -
                BottomInset -
                TransportHeight,
            8.0f
        );

    ImGui::SetCursorScreenPos(
        ImVec2(
            viewportPosition.x +
                TransportX,
            viewportPosition.y +
                transportY
        )
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_ChildRounding,
        7.0f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowPadding,
        ImVec2(
            10.0f,
            9.0f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_ChildBg,
        ui::themePalette().hudBackground
    );

    ImGui::BeginChild(
        "##AnimationTransportCard",
        ImVec2(
            TransportWidth,
            TransportHeight
        ),
        false,
        ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse
    );

    // Main media controls sit on their own centered row.
    constexpr float PlaybackControlsWidth =
        141.0f;

    ImGui::SetCursorPos(
        ImVec2(
            std::max(
                (
                    TransportWidth -
                    PlaybackControlsWidth
                ) *
                    0.5f,
                8.0f
            ),
            7.0f
        )
    );


    // --------------------------------------------------------
    // MODERN MEDIA CONTROLS
    //
    // Secondary actions are essentially borderless icons.
    // Play/pause gets stronger visual priority.
    // --------------------------------------------------------

    const auto mediaButton =
        [](
            const char* id,
            ui::Icon icon,
            const char* tooltip,
            const ImVec2 size,
            bool primary
        ) {
            const auto& palette =
                ui::themePalette();

            const ImVec4 transparent{
                0.0f,
                0.0f,
                0.0f,
                0.0f
            };

            const ImVec4 hover =
                primary
                    ? ImVec4(
                          palette.primary.x,
                          palette.primary.y,
                          palette.primary.z,
                          0.34f
                      )
                    : ImVec4(
                          palette.text.x,
                          palette.text.y,
                          palette.text.z,
                          0.09f
                      );

            const ImVec4 active =
                primary
                    ? ImVec4(
                          palette.primary.x,
                          palette.primary.y,
                          palette.primary.z,
                          0.46f
                      )
                    : ImVec4(
                          palette.primary.x,
                          palette.primary.y,
                          palette.primary.z,
                          0.18f
                      );

            const ImVec4 normal =
                primary
                    ? ImVec4(
                          palette.primary.x,
                          palette.primary.y,
                          palette.primary.z,
                          0.24f
                      )
                    : transparent;

            ImGui::PushStyleVar(
                ImGuiStyleVar_FrameRounding,
                size.y * 0.5f
            );

            ImGui::PushStyleVar(
                ImGuiStyleVar_FrameBorderSize,
                0.0f
            );

            ImGui::PushStyleColor(
                ImGuiCol_Button,
                normal
            );

            ImGui::PushStyleColor(
                ImGuiCol_ButtonHovered,
                hover
            );

            ImGui::PushStyleColor(
                ImGuiCol_ButtonActive,
                active
            );

            ImGui::PushStyleColor(
                ImGuiCol_Text,
                palette.text
            );

            const bool clicked =
                ui::iconButton(
                    id,
                    icon,
                    tooltip,
                    size
                );

            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(2);

            return clicked;
        };


    constexpr ImVec2 SecondaryButtonSize{
        30.0f,
        30.0f
    };

    constexpr ImVec2 PrimaryButtonSize{
        36.0f,
        36.0f
    };


    if (
        mediaButton(
            "##AnimationStepBackward",
            ui::Icon::StepBackward,
            "Previous frame",
            SecondaryButtonSize,
            false
        )
    ) {
        animationPlayer_.pause();

        if (
            animationPlayer_.
                stepBackward()
        ) {
            rebuildAnimationFrame();
        }
    }


    ImGui::SameLine(
        0.0f,
        5.0f
    );


    const bool playing =
        animationPlayer_.isPlaying();

    if (
        mediaButton(
            "##AnimationPlayPause",
            playing
                ? ui::Icon::Pause
                : ui::Icon::Play,
            playing
                ? "Pause"
                : "Play",
            PrimaryButtonSize,
            true
        )
    ) {
        animationPlayer_.setPlaying(
            !playing
        );
    }


    ImGui::SameLine(
        0.0f,
        5.0f
    );


    if (
        mediaButton(
            "##AnimationStepForward",
            ui::Icon::StepForward,
            "Next frame",
            SecondaryButtonSize,
            false
        )
    ) {
        animationPlayer_.pause();

        if (
            animationPlayer_.
                stepForward()
        ) {
            rebuildAnimationFrame();
        }
    }


    ImGui::SameLine(
        0.0f,
        5.0f
    );


    if (
        mediaButton(
            "##AnimationRestart",
            ui::Icon::Restart,
            "Restart animation",
            SecondaryButtonSize,
            false
        )
    ) {
        animationPlayer_.restart();

        rebuildAnimationFrame();
    }


    // --------------------------------------------------------
    // PLAYBACK OPTIONS
    // --------------------------------------------------------

    const std::string rangeText =
        "1-" +
        std::to_string(
            frameCount
        );

    constexpr float LoopWidth =
        50.0f;

    constexpr float OptionGapA =
        7.0f;

    constexpr float SpeedWidth =
        72.0f;

    constexpr float OptionGapB =
        9.0f;

    const float playbackOptionsWidth =
        LoopWidth +
        OptionGapA +
        SpeedWidth +
        OptionGapB +
        ImGui::CalcTextSize(
            rangeText.c_str()
        ).x;

    ImGui::SetCursorPos(
        ImVec2(
            std::max(
                (
                    TransportWidth -
                    playbackOptionsWidth
                ) *
                    0.5f,
                8.0f
            ),
            51.0f
        )
    );

    const auto& optionPalette =
        ui::themePalette();

    bool looping =
        animationPlayer_.looping();

    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameRounding,
        13.0f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameBorderSize,
        0.0f
    );

    ImGui::PushStyleColor(
        ImGuiCol_Button,
        looping
            ? ImVec4(
                  optionPalette.primary.x,
                  optionPalette.primary.y,
                  optionPalette.primary.z,
                  0.22f
              )
            : ImVec4(
                  optionPalette.text.x,
                  optionPalette.text.y,
                  optionPalette.text.z,
                  0.055f
              )
    );

    ImGui::PushStyleColor(
        ImGuiCol_ButtonHovered,
        ImVec4(
            optionPalette.primary.x,
            optionPalette.primary.y,
            optionPalette.primary.z,
            0.18f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_ButtonActive,
        ImVec4(
            optionPalette.primary.x,
            optionPalette.primary.y,
            optionPalette.primary.z,
            0.32f
        )
    );

    if (
        ImGui::Button(
            "LOOP",
            ImVec2(
                50.0f,
                27.0f
            )
        )
    ) {
        looping =
            !looping;

        animationPlayer_.setLooping(
            looping
        );
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);


    ImGui::SameLine(
        0.0f,
        7.0f
    );


    float speed =
        animationPlayer_.speed();

    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameRounding,
        13.0f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameBorderSize,
        0.0f
    );

    ImGui::PushStyleColor(
        ImGuiCol_FrameBg,
        ImVec4(
            optionPalette.text.x,
            optionPalette.text.y,
            optionPalette.text.z,
            0.055f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_FrameBgHovered,
        ImVec4(
            optionPalette.text.x,
            optionPalette.text.y,
            optionPalette.text.z,
            0.10f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_FrameBgActive,
        ImVec4(
            optionPalette.primary.x,
            optionPalette.primary.y,
            optionPalette.primary.z,
            0.17f
        )
    );

    ImGui::SetNextItemWidth(
        72.0f
    );

    if (
        ImGui::DragFloat(
            "##AnimationSpeed",
            &speed,
            0.05f,
            0.10f,
            3.00f,
            "%.2fx",
            ImGuiSliderFlags_AlwaysClamp
        )
    ) {
        animationPlayer_.setSpeed(
            speed
        );
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);


    ImGui::SameLine(
        0.0f,
        9.0f
    );

    ImGui::AlignTextToFramePadding();

    ImGui::TextDisabled(
        "%s",
        rangeText.c_str()
    );

    if (
        ImGui::IsItemHovered(
            ImGuiHoveredFlags_DelayShort
        )
    ) {
        ImGui::SetTooltip(
            "Playback range: full sequence"
        );
    }


    ImGui::EndChild();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);






    // --------------------------------------------------------
    // CURRENT FRAME INSPECTOR
    // --------------------------------------------------------

    if (
        state_.activeAnimation.has_value() &&
        hudLayout.frameWidth >=
            animation_hud::FrameMinimumWidth
    ) {
        ImGui::SetCursorScreenPos(
            ImVec2(
                viewportPosition.x +
                    hudLayout.frameX,
                viewportPosition.y +
                    transportY
            )
        );

        ImGui::PushStyleVar(
            ImGuiStyleVar_ChildRounding,
            8.0f
        );

        ImGui::PushStyleVar(
            ImGuiStyleVar_WindowPadding,
            ImVec2(
                14.0f,
                10.0f
            )
        );

        ImGui::PushStyleColor(
            ImGuiCol_ChildBg,
            ui::themePalette().
                hudBackground
        );

        ImGui::BeginChild(
            "##AnimationFrameInspector",
            ImVec2(
                hudLayout.frameWidth,
                animation_hud::CardHeight
            ),
            true,
            ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse
        );


        const auto drawTextAt =
            [](
                const std::string& text,
                float x,
                float y,
                bool disabled
            ) {
                ImGui::SetCursorPos(
                    ImVec2(
                        x,
                        y
                    )
                );

                if (disabled) {
                    ImGui::TextDisabled(
                        "%s",
                        text.c_str()
                    );
                }
                else {
                    ImGui::TextUnformatted(
                        text.c_str()
                    );
                }
            };


        const auto drawCentered =
            [&drawTextAt](
                const std::string& text,
                float y,
                bool disabled
            ) {
                const float width =
                    ImGui::CalcTextSize(
                        text.c_str()
                    ).x;

                drawTextAt(
                    text,
                    std::max(
                        (
                            ImGui::GetWindowWidth() -
                            width
                        ) *
                            0.5f,
                        8.0f
                    ),
                    y,
                    disabled
                );
            };


        constexpr float ContentInset =
            14.0f;

        constexpr float ValueGap =
            10.0f;


        // Title.
        drawCentered(
            "FRAME",
            10.0f,
            true
        );


        // Frame position belongs on the left.
        const std::string framePosition =
            std::to_string(
                frameCount > 0
                    ? frameIndex + 1
                    : 0
            ) +
            " / " +
            std::to_string(
                frameCount
            );

        drawTextAt(
            framePosition,
            ContentInset,
            35.0f,
            false
        );


        // Global frame ID belongs on the right.
        const std::string globalFrame =
            currentResolved
                ? "#" +
                    std::to_string(
                        static_cast<unsigned int>(
                            currentResolved->frame.id
                        )
                    )
                : "-";

        const float globalFrameWidth =
            ImGui::CalcTextSize(
                globalFrame.c_str()
            ).x;

        drawTextAt(
            globalFrame,
            ImGui::GetWindowWidth() -
                ContentInset -
                globalFrameWidth,
            35.0f,
            false
        );


        if (currentResolved) {
            const unsigned int duration =
                static_cast<unsigned int>(
                    animationPlayer_.
                        currentFrameDurationMilliseconds()
                );

            const std::size_t transformCount =
                currentResolved->frame.
                    transforms.size();


            // Duration belongs on the left.
            const std::string durationText =
                std::to_string(
                    duration
                ) +
                " ms";


            // Transform count belongs on the right.
            // Prefer the full word and abbreviate only if the
            // actual current card width requires it.
            const std::string transformText =
                std::to_string(
                    transformCount
                ) +
                " transforms";

            const std::string compactTransformText =
                std::to_string(
                    transformCount
                ) +
                " xforms";


            const float durationWidth =
                ImGui::CalcTextSize(
                    durationText.c_str()
                ).x;

            const float availableWidth =
                std::max(
                    ImGui::GetWindowWidth() -
                        ContentInset *
                            2.0f,
                    1.0f
                );

            const float transformWidth =
                ImGui::CalcTextSize(
                    transformText.c_str()
                ).x;


            const std::string& visibleTransformText =
                durationWidth +
                    ValueGap +
                    transformWidth <=
                        availableWidth
                    ? transformText
                    : compactTransformText;

            const float visibleTransformWidth =
                ImGui::CalcTextSize(
                    visibleTransformText.c_str()
                ).x;


            drawTextAt(
                durationText,
                ContentInset,
                62.0f,
                true
            );

            drawTextAt(
                visibleTransformText,
                ImGui::GetWindowWidth() -
                    ContentInset -
                    visibleTransformWidth,
                62.0f,
                true
            );
        }


        ImGui::EndChild();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

}

void CacheExplorer::selectNextNpcWithProjectile() {
    if (!state_.activeNpc.has_value()) {
        return;
    }

    const std::uint16_t currentId =
        state_.activeNpc->id;

    const auto hasProjectile =
        [this](const eld::npc::Npc& npc) {
            const eld::animation::presentation::NpcAnimationProfile profile =
                animationPresentationCatalog_.resolveNpc(npc);

            return std::any_of(
                profile.bindings.begin(),
                profile.bindings.end(),
                [this](
                    const eld::animation::presentation::AnimationBinding& binding
                ) {
                    return std::any_of(
                        binding.effects.begin(),
                        binding.effects.end(),
                        [this](
                            const eld::animation::presentation::AnimationEffectBinding& effect
                        ) {
                            return
                                effect.projectile &&
                                spotAnimationRepository_.contains(
                                    effect.spotAnimationId
                                );
                        }
                    );
                }
            );
        };

    const auto selectNpc =
        [this](const eld::npc::Npc& npc) {
            state_.selection.type =
                CacheTreeNodeType::Npc;

            state_.selection.definitionId =
                static_cast<int>(npc.id);

            state_.selection.name = "npc";
            state_.selection.label =
                "NPC " + std::to_string(npc.id);

            if (
                !npc.name.empty() &&
                npc.name != "null"
            ) {
                state_.selection.label +=
                    " - " + npc.name;
            }

            const std::string NpcKeyMarker =
                "/definitions/npc/";

            const std::size_t marker =
                state_.selection.key.find(NpcKeyMarker);

            if (marker != std::string::npos) {
                const std::size_t idStart =
                    marker + NpcKeyMarker.size();

                state_.selection.key =
                    state_.selection.key.substr(0, idStart) +
                    std::to_string(npc.id);
            }
            else {
                // Still force a selection-key change if this NPC was reached
                // from a non-tree code path. handleSelectionChanged() only
                // needs the type + definition id.
                state_.selection.key =
                    "npc/" + std::to_string(npc.id);
            }
        };

    const auto npcIds =
        npcRepository_.listIds();

    for (const auto id : npcIds) {
        const eld::npc::Npc npc =
            npcRepository_.get(id);
        if (
            npc.id > currentId &&
            hasProjectile(npc)
        ) {
            selectNpc(npc);
            return;
        }
    }

    // Wrap so the button can be hammered continuously while reviewing NPCs.
    for (const auto id : npcIds) {
        const eld::npc::Npc npc =
            npcRepository_.get(id);
        if (
            npc.id <= currentId &&
            hasProjectile(npc)
        ) {
            selectNpc(npc);
            return;
        }
    }
}

void CacheExplorer::renderNpcAnimationControls() {
    if (
        !state_.activeNpc.has_value() ||
        !animationSource_.has_value()
    ) {
        return;
    }

    if (ImGui::SmallButton("Next NPC w/ projectile")) {
        selectNextNpcWithProjectile();
    }

    ImGui::SameLine();
    ImGui::Text(
        "NPC %u",
        static_cast<unsigned int>(state_.activeNpc->id)
    );

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
            actionBindingLabel(binding);

        if (ImGui::Button(label.c_str())) {
            clearNpcActionView();
            startAnimationView(binding.sequenceId);
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
            actionBindingLabel(binding);

        if (ImGui::Button(label.c_str())) {
            startNpcActionView(binding);
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
            actionBindingLabel(*activeNpcAction_);

        ImGui::Text(
            "Active action: %s",
            actionLabel.c_str()
        );

        for (
            const eld::animation::presentation::AnimationEffectBinding& effect :
            activeNpcAction_->effects
        ) {
            if (effect.projectile) {
                ImGui::BulletText(
                    "SpotAnim %u projectile  h=%u->%u slope=%u start=%u",
                    static_cast<unsigned int>(effect.spotAnimationId),
                    static_cast<unsigned int>(effect.projectileStartHeight),
                    static_cast<unsigned int>(effect.projectileEndHeight),
                    static_cast<unsigned int>(effect.projectileSlope),
                    static_cast<unsigned int>(effect.projectileStartDistance)
                );
            }
            else {
                ImGui::BulletText(
                    "SpotAnim %u %s  delay=%ums  duration=%ums",
                    static_cast<unsigned int>(effect.spotAnimationId),
                    effect.target ? "target" : "attached",
                    static_cast<unsigned int>(effect.delayMilliseconds),
                    static_cast<unsigned int>(effect.durationMilliseconds)
                );
            }
        }
    }

    renderAnimationPlayerHud();
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

    renderAnimationPlayerHud();
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

    renderAnimationPlayerHud();
    ImGui::Separator();
}

void CacheExplorer::renderAnimationControls() {
    if (state_.activeAnimation.has_value()) {
        renderAnimationPlayerHud();
        return;
    }

    if (state_.activeNpc.has_value()) {
        renderNpcAnimationControls();
        return;
    }

    if (state_.activeItem.has_value()) {
        renderItemAnimationControls();
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

}
