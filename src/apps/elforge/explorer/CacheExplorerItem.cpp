#include "explorer/CacheExplorer.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <imgui.h>

#include "views/item/ItemAnimationPresets.h"
#include "views/item/ItemView.h"

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

void CacheExplorer::startItemActionView(
    const eld::animation::presentation::AnimationBinding& binding
) {
    if (
        !state_.activeItem.has_value() ||
        !animationSource_.has_value()
    ) {
        return;
    }

    clearNpcActionView();
    activeItemAction_ = binding;

    // Player models use the same authored +Z forward axis as the classic NPC
    // models. Face the mannequin toward the target once when starting a combat
    // view so projectile/special effects leave in the direction Bob faces.
    const float deltaX =
        actionTargetWorld_.x - state_.modelTransform.position.x;
    const float deltaZ =
        actionTargetWorld_.z - state_.modelTransform.position.z;

    if (deltaX * deltaX + deltaZ * deltaZ > 0.0001f) {
        constexpr float ForwardAxisOffset = 1.57079632679f;
        state_.modelTransform.rotation.y =
            std::atan2(-deltaZ, deltaX) - ForwardAxisOffset;
    }

    if (binding.sequenceId.has_value()) {
        startAnimationView(binding.sequenceId);
        animationPlayer_.setLooping(false);
    }

    appendActionEffects(binding);
}

void CacheExplorer::showItemInventoryView() {
    if (!state_.activeItem.has_value()) {
        return;
    }

    const ItemView view;
    std::optional<eld::model::Model> model =
        view.build(*state_.activeItem, modelRepository_);

    if (!model.has_value()) {
        return;
    }

    resetAnimationView();

    state_.activeModelHandle =
        graphicsResources_.resolveModel(model->mesh);
    state_.activeModel = std::move(*model);
    itemViewMode_ = ItemViewMode::Inventory;
}

void CacheExplorer::showItemEquippedView(
    ItemViewGender gender
) {
    if (!state_.activeItem.has_value()) {
        return;
    }

    const ItemView view;
    std::optional<eld::model::Model> model =
        view.buildEquipped(
            *state_.activeItem,
            gender,
            identityKitRepository_,
            modelRepository_
        );

    if (!model.has_value()) {
        return;
    }

    resetAnimationView();

    state_.activeModelHandle =
        graphicsResources_.resolveModel(model->mesh);
    state_.activeModel = std::move(*model);

    animationSource_ = state_.activeModel->mesh;
    animationTargetKind_ = AnimationTargetKind::Item;

    itemViewMode_ =
        gender == ItemViewGender::Male
            ? ItemViewMode::MaleEquipped
            : ItemViewMode::FemaleEquipped;

    // Classic default player standing animation (human_ready).
    startAnimationView(std::uint16_t{808});
}

void CacheExplorer::selectNextWearableItem() {
    if (!state_.activeItem.has_value()) {
        return;
    }

    const std::uint16_t currentId =
        state_.activeItem->id;

    const ItemViewMode previousMode =
        itemViewMode_;

    const ItemView view;

    const auto isWearable =
        [&view](const eld::definition::ItemDefinition& item) {
            return
                view.hasEquippedModel(
                    item,
                    ItemViewGender::Male
                ) ||
                view.hasEquippedModel(
                    item,
                    ItemViewGender::Female
                );
        };

    const auto selectItem =
        [this, previousMode, &view](
            const eld::definition::ItemDefinition& item
        ) {
            state_.selection.type =
                CacheTreeNodeType::ItemDefinition;

            state_.selection.definitionId =
                static_cast<int>(item.id);

            state_.selection.name = "obj";
            state_.selection.label =
                "Item " + std::to_string(item.id);

            if (
                !item.name.empty() &&
                item.name != "null"
            ) {
                state_.selection.label +=
                    " - " + item.name;
            }

            const std::string ItemKeyMarker =
                "/definitions/obj/";

            const std::size_t marker =
                state_.selection.key.find(ItemKeyMarker);

            if (marker != std::string::npos) {
                const std::size_t idStart =
                    marker + ItemKeyMarker.size();

                state_.selection.key =
                    state_.selection.key.substr(0, idStart) +
                    std::to_string(item.id);
            }
            else {
                state_.selection.key =
                    "obj/" + std::to_string(item.id);
            }

            // Apply the new selection immediately so hammering Next wearable
            // feels instant, and mark the key consumed so update() does not
            // rebuild the same selection again on the following frame.
            lastSelectedKey_ =
                state_.selection.key;
            handleSelectionChanged();

            if (!state_.activeItem.has_value()) {
                return;
            }

            if (previousMode == ItemViewMode::MaleEquipped) {
                if (view.hasEquippedModel(
                    *state_.activeItem,
                    ItemViewGender::Male
                )) {
                    showItemEquippedView(ItemViewGender::Male);
                    return;
                }
            }
            else if (previousMode == ItemViewMode::FemaleEquipped) {
                if (view.hasEquippedModel(
                    *state_.activeItem,
                    ItemViewGender::Female
                )) {
                    showItemEquippedView(ItemViewGender::Female);
                    return;
                }
            }

            // If the current view was equipped but the next item lacks that
            // gender's model, stay in an equipped view using the other gender
            // when possible instead of unexpectedly dropping to raw-item view.
            if (previousMode != ItemViewMode::Inventory) {
                if (view.hasEquippedModel(
                    *state_.activeItem,
                    ItemViewGender::Male
                )) {
                    showItemEquippedView(ItemViewGender::Male);
                }
                else if (view.hasEquippedModel(
                    *state_.activeItem,
                    ItemViewGender::Female
                )) {
                    showItemEquippedView(ItemViewGender::Female);
                }
            }
        };

    const auto& items =
        itemRepository_.list();

    for (const eld::definition::ItemDefinition& item : items) {
        if (
            item.id > currentId &&
            isWearable(item)
        ) {
            selectItem(item);
            return;
        }
    }

    // Wrap so the button can be used continuously while reviewing equipment.
    for (const eld::definition::ItemDefinition& item : items) {
        if (
            item.id <= currentId &&
            isWearable(item)
        ) {
            selectItem(item);
            return;
        }
    }
}

void CacheExplorer::renderItemAnimationControls() {
    if (!state_.activeItem.has_value()) {
        return;
    }

    const eld::definition::ItemDefinition& item =
        *state_.activeItem;

    const ItemView view;
    const bool hasMale =
        view.hasEquippedModel(
            item,
            ItemViewGender::Male
        );
    const bool hasFemale =
        view.hasEquippedModel(
            item,
            ItemViewGender::Female
        );

    if (ImGui::SmallButton("Next wearable")) {
        selectNextWearableItem();
        return;
    }

    ImGui::SameLine();
    ImGui::Text(
        "Item %u",
        static_cast<unsigned int>(item.id)
    );

    ImGui::TextUnformatted("ITEM VIEW");

    if (ImGui::Button("Item")) {
        showItemInventoryView();
    }

    if (hasMale) {
        ImGui::SameLine();
        if (ImGui::Button("Male equip")) {
            showItemEquippedView(ItemViewGender::Male);
        }
    }

    if (hasFemale) {
        ImGui::SameLine();
        if (ImGui::Button("Female equip")) {
            showItemEquippedView(ItemViewGender::Female);
        }
    }

    if (!hasMale && !hasFemale) {
        ImGui::SameLine();
        ImGui::TextDisabled("No worn model");
    }

    const char* viewLabel = "item";

    if (itemViewMode_ == ItemViewMode::MaleEquipped) {
        viewLabel = "male equipped";
    }
    else if (itemViewMode_ == ItemViewMode::FemaleEquipped) {
        viewLabel = "female equipped";
    }

    ImGui::Text(
        "View: %s",
        viewLabel
    );

    if (
        itemViewMode_ == ItemViewMode::Inventory ||
        !animationSource_.has_value()
    ) {
        ImGui::Separator();
        return;
    }

    using Action =
        eld::animation::presentation::AnimationAction;

    const auto sequenceAvailable =
        [this](std::uint16_t id) {
            const eld::definition::SequenceDefinition* sequence =
                sequenceRepository_.find(id);

            return
                sequence != nullptr &&
                !sequence->frames.empty();
        };

    ImGui::TextUnformatted("MOVEMENT");

    if (
        sequenceAvailable(808) &&
        ImGui::Button("Idle")
    ) {
        clearNpcActionView();
        startAnimationView(std::uint16_t{808});
        animationPlayer_.setLooping(true);
    }

    if (sequenceAvailable(819)) {
        ImGui::SameLine();
        if (ImGui::Button("Walk")) {
            clearNpcActionView();
            startAnimationView(std::uint16_t{819});
            animationPlayer_.setLooping(true);
        }
    }

    const eld::animation::presentation::ItemAnimationProfile profile =
        animationPresentationCatalog_.resolveItem(item);

    const auto hasAuthoredAction =
        [&](Action action) {
            return std::any_of(
                profile.bindings.begin(),
                profile.bindings.end(),
                [&](const eld::animation::presentation::AnimationBinding& binding) {
                    return binding.action == action;
                }
            );
        };

    ImGui::Spacing();
    ImGui::TextUnformatted("ACTIONS");

    bool hasActions = false;
    bool firstAction = true;

    const auto renderActionButton =
        [&](
            const std::string& label,
            const eld::animation::presentation::AnimationBinding& binding
        ) {
            if (!firstAction) {
                ImGui::SameLine();
            }

            if (ImGui::Button(label.c_str())) {
                startItemActionView(binding);
            }

            firstAction = false;
            hasActions = true;
        };

    const std::vector<eld::animation::presentation::AnimationBinding>
        generatedActions =
            ItemAnimationPresets::actions(item);

    for (
        const eld::animation::presentation::AnimationBinding& binding :
        generatedActions
    ) {
        if (
            hasAuthoredAction(binding.action) ||
            !binding.sequenceId.has_value() ||
            !sequenceAvailable(*binding.sequenceId)
        ) {
            continue;
        }

        renderActionButton(
            actionBindingLabel(binding),
            binding
        );
    }

    if (!hasAuthoredAction(Action::Defend)) {
        const std::optional<std::uint16_t> defendSequence =
            ItemAnimationPresets::defendSequence(item);

        if (
            defendSequence.has_value() &&
            sequenceAvailable(*defendSequence)
        ) {
            eld::animation::presentation::AnimationBinding defend;
            defend.action = Action::Defend;
            defend.sequenceId = *defendSequence;
            renderActionButton("Defend", defend);
        }
    }

    if (
        !hasAuthoredAction(Action::Death) &&
        sequenceAvailable(836)
    ) {
        eld::animation::presentation::AnimationBinding death;
        death.action = Action::Death;
        death.sequenceId = 836; // human_death
        renderActionButton("Death", death);
    }

    for (
        const eld::animation::presentation::AnimationBinding& binding :
        profile.bindings
    ) {
        if (
            binding.action == Action::Idle ||
            binding.action == Action::Walk ||
            binding.action == Action::TurnAround ||
            binding.action == Action::TurnLeft ||
            binding.action == Action::TurnRight
        ) {
            continue;
        }

        if (
            !binding.sequenceId.has_value() &&
            binding.effects.empty()
        ) {
            continue;
        }

        renderActionButton(
            actionBindingLabel(binding),
            binding
        );
    }

    if (!hasActions) {
        ImGui::TextUnformatted(
            "No item action animation is known for this item."
        );
    }

    if (activeItemAction_.has_value()) {
        const std::string actionLabel =
            actionBindingLabel(*activeItemAction_);

        ImGui::Text(
            "Active action: %s",
            actionLabel.c_str()
        );

        for (
            const eld::animation::presentation::AnimationEffectBinding& effect :
            activeItemAction_->effects
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
    ImGui::Separator();
}

}
