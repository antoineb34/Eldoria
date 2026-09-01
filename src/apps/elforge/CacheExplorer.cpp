#include "CacheExplorer.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include "IdentityKitPreviewBuilder.h"
#include "LocationPreviewBuilder.h"
#include "NpcPreviewBuilder.h"
#include "ItemPreviewBuilder.h"
#include "SpotAnimationPreviewBuilder.h"
#include "FontPreviewBuilder.h"
#include "MapPreviewBuilder.h"

#include <exception>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <utility>

#include <imgui.h>

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

std::string lowerAscii(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return value;
}

bool itemHasInventoryAction(
    const eld::definition::ItemDefinition& definition,
    const std::string& wanted
) {
    const std::string wantedLower = lowerAscii(wanted);

    return std::any_of(
        definition.inventoryActions.begin(),
        definition.inventoryActions.end(),
        [&](const std::string& action) {
            return lowerAscii(action) == wantedLower;
        }
    );
}

bool itemHasWearPosition(
    const eld::definition::ItemDefinition& definition,
    eld::definition::ItemWearPosition wanted
) {
    return
        definition.wearPosition == wanted ||
        definition.wearPosition2 == wanted ||
        definition.wearPosition3 == wanted;
}

bool itemLooksLikeShield(
    const eld::definition::ItemDefinition& definition
) {
    if (
        itemHasWearPosition(
            definition,
            eld::definition::ItemWearPosition::LeftHand
        )
    ) {
        return true;
    }

    const std::string name = lowerAscii(definition.name);

    return
        name.find("shield") != std::string::npos ||
        name.find("defender") != std::string::npos;
}

bool itemLooksLikeWeapon(
    const eld::definition::ItemDefinition& definition
) {
    if (itemLooksLikeShield(definition)) {
        return false;
    }

    if (
        itemHasWearPosition(
            definition,
            eld::definition::ItemWearPosition::RightHand
        )
    ) {
        return true;
    }

    // The stock 317 client item definition does not expose server wearpos
    // metadata, but weapons use the inventory action "Wield".
    return itemHasInventoryAction(definition, "Wield");
}

eld::animation::presentation::AnimationBinding makeItemActionBinding(
    eld::animation::presentation::AnimationAction action,
    const char* variant,
    std::uint16_t sequenceId
) {
    eld::animation::presentation::AnimationBinding binding;
    binding.action = action;
    binding.variant = variant;
    binding.sequenceId = sequenceId;
    return binding;
}

std::vector<eld::animation::presentation::AnimationBinding>
genericItemActionBindings(
    const eld::definition::ItemDefinition& definition
) {
    using Action =
        eld::animation::presentation::AnimationAction;
    using Binding =
        eld::animation::presentation::AnimationBinding;

    std::vector<Binding> bindings;
    const std::string name = lowerAscii(definition.name);

    const auto add =
        [&](Action action, const char* variant, std::uint16_t sequenceId) {
            bindings.push_back(
                makeItemActionBinding(
                    action,
                    variant,
                    sequenceId
                )
            );
        };

    // Combat animations were selected by the server/content layer rather
    // than owned by obj.dat. Expose the classic human animation family for
    // every style the weapon can plausibly use instead of forcing one generic
    // "Attack" animation.
    if (itemLooksLikeWeapon(definition)) {
        if (name.find("crossbow") != std::string::npos) {
            add(Action::Attack, "Ranged", 427); // human_crossbow
        }
        else if (name.find("bow") != std::string::npos) {
            add(Action::Attack, "Ranged", 426); // human_bow
        }
        else if (name.find("claw") != std::string::npos) {
            // Preserved claw configs explicitly select these two animations.
            add(Action::Attack, "Stab", 1067);  // claws_punch
            add(Action::Attack, "Slash", 393);  // human_axe_chop
        }
        else if (name.find("dagger") != std::string::npos) {
            add(Action::Attack, "Stab / lunge", 376);
            add(Action::Attack, "Slash / hack", 377);
        }
        else if (
            name.find("spear") != std::string::npos ||
            name.find("halberd") != std::string::npos
        ) {
            add(Action::Attack, "Spike", 428);
            add(Action::Attack, "Lunge", 429);

            if (
                name.find("spear") != std::string::npos &&
                name.find("halberd") == std::string::npos
            ) {
                add(Action::Attack, "Throw", 432);
            }
        }
        else if (name.find("scythe") != std::string::npos) {
            add(Action::Attack, "Slash", 437);
            add(Action::Attack, "Lunge", 438);
            add(Action::Attack, "Spin", 439);
            add(Action::Attack, "Sweep", 440);
        }
        else if (
            name.find("javelin") != std::string::npos ||
            name.find("dart") != std::string::npos ||
            name.find("throwing knife") != std::string::npos ||
            name.find("thrownaxe") != std::string::npos ||
            name.find("thrown axe") != std::string::npos
        ) {
            add(Action::Attack, "Throw", 385);
        }
        else if (name.find("staff") != std::string::npos) {
            add(Action::Attack, "Spike", 412);
            add(Action::Attack, "Pound / crush", 413);
            add(Action::Attack, "Pummel / crush", 414);
        }
        else if (
            name.find("2h") != std::string::npos ||
            name.find("two-handed") != std::string::npos ||
            name.find("two handed") != std::string::npos
        ) {
            add(Action::Attack, "Stab", 405);
            add(Action::Attack, "Chop", 406);
            add(Action::Attack, "Slash", 407);
            add(Action::Attack, "Lunge", 408);
            add(Action::Attack, "Spin", 409);
        }
        else if (name.find("pickaxe") != std::string::npos) {
            // Preserved pickaxe configs map "stab" to the blunt pound body
            // animation and "crush" to the blunt spike animation.
            add(Action::Attack, "Stab", 401);
            add(Action::Attack, "Crush", 400);
        }
        else if (
            name.find("mace") != std::string::npos ||
            name.find("maul") != std::string::npos ||
            name.find("hammer") != std::string::npos
        ) {
            add(Action::Attack, "Spike", 400);
            add(Action::Attack, "Pound / crush", 401);
            add(Action::Attack, "Pummel / crush", 402);
        }
        else if (name.find("axe") != std::string::npos) {
            add(Action::Attack, "Chop", 393);
            add(Action::Attack, "Hack / slash", 395);
            add(Action::Attack, "Smash / crush", 396);
        }
        else if (
            name.find("sword") != std::string::npos ||
            name.find("scimitar") != std::string::npos
        ) {
            add(Action::Attack, "Stab", 386);
            add(Action::Attack, "Slash", 390);
            add(Action::Attack, "Lunge", 392);
        }
        else {
            add(Action::Attack, "Slash", 390);
        }
    }

    // Skill tools have their own action animations. These models are present
    // as manwear/womanwear pieces even though many are not normal equipment.
    if (name == "bronze axe") {
        add(Action::Use, "Woodcut", 879);
    }
    else if (name == "iron axe") {
        add(Action::Use, "Woodcut", 877);
    }
    else if (name == "steel axe") {
        add(Action::Use, "Woodcut", 875);
    }
    else if (name == "black axe") {
        add(Action::Use, "Woodcut", 873);
    }
    else if (name == "mithril axe") {
        add(Action::Use, "Woodcut", 871);
    }
    else if (name == "adamant axe") {
        add(Action::Use, "Woodcut", 869);
    }
    else if (name == "rune axe") {
        add(Action::Use, "Woodcut", 867);
    }

    if (name == "bronze pickaxe") {
        add(Action::Use, "Mine", 625);
    }
    else if (name == "iron pickaxe") {
        add(Action::Use, "Mine", 626);
    }
    else if (name == "steel pickaxe") {
        add(Action::Use, "Mine", 627);
    }
    else if (name == "mithril pickaxe") {
        add(Action::Use, "Mine", 629);
    }
    else if (name == "adamant pickaxe") {
        add(Action::Use, "Mine", 628);
    }
    else if (name == "rune pickaxe") {
        add(Action::Use, "Mine", 624);
    }

    if (name.find("harpoon") != std::string::npos) {
        add(Action::Use, "Harpoon", 618);
    }
    else if (name.find("lobster pot") != std::string::npos) {
        add(Action::Use, "Lobster pot", 619);
    }
    else if (name.find("big fishing net") != std::string::npos) {
        add(Action::Use, "Big net", 620);
    }
    else if (name.find("small fishing net") != std::string::npos) {
        add(Action::Use, "Small net", 621);
    }
    else if (name.find("fishing rod") != std::string::npos) {
        add(Action::Use, "Fish", 622);
    }
    else if (name.find("karambwan vessel") != std::string::npos) {
        add(Action::Use, "Fish", 623);
    }

    return bindings;
}

std::optional<std::uint16_t> genericItemDefendSequence(
    const eld::definition::ItemDefinition& definition
) {
    const std::string name = lowerAscii(definition.name);

    // Match the classic human weapon block families when possible. Shields
    // and ranged/unknown wearables fall back to the ordinary human block.
    if (name.find("claw") != std::string::npos) {
        return 397; // preserved claw configs use human_axe_block
    }

    if (name.find("dagger") != std::string::npos) {
        return 378; // human_ddagger_block
    }

    if (
        name.find("spear") != std::string::npos ||
        name.find("halberd") != std::string::npos
    ) {
        return 430; // human_spear_block
    }

    if (name.find("scythe") != std::string::npos) {
        return 435; // human_scythe_block
    }

    if (name.find("staff") != std::string::npos) {
        return 415; // human_staff_block
    }

    if (
        name.find("2h") != std::string::npos ||
        name.find("two-handed") != std::string::npos ||
        name.find("two handed") != std::string::npos
    ) {
        return 410; // human_dhsword_block
    }

    if (
        name.find("mace") != std::string::npos ||
        name.find("maul") != std::string::npos ||
        name.find("hammer") != std::string::npos
    ) {
        return 403; // human_blunt_block
    }

    if (name.find("axe") != std::string::npos) {
        return 397; // human_axe_block
    }

    if (
        name.find("sword") != std::string::npos ||
        name.find("scimitar") != std::string::npos
    ) {
        return 387; // human_sword_block
    }

    return 424; // human_unarmedblock
}

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
    activeItemAction_.reset();
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

    // ELFORGE_NPC_FACE_ACTION_TARGET_V1
    //
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

    // ELFORGE_NPC_FACING_AXIS_FIX_V1
    //
    // The target-heading atan2 below produces an angle whose zero direction
    // is +X. RS NPC models are authored facing +Z at zero model yaw, so using
    // that heading directly points the NPC's left side at the target.
    //
    // Eldoria's rotationY(+pi/2) rotates local +Z onto world +X, therefore
    // add a quarter-turn to convert the world heading into model yaw.
    constexpr float NpcForwardAxisOffset =
        1.57079632679f;

    // ELFORGE_NPC_FACING_AXIS_FIX_V2
    //
    // V1 proved the offset magnitude (90 degrees) but used the wrong sign:
    // +pi/2 aligned the dragon's BACK with the target. Use -pi/2 so the
    // authored forward/front axis points at the target.
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

void CacheExplorer::startNpcActionPreview(
    const eld::animation::presentation::AnimationBinding& binding
) {
    clearNpcActionPreview();
    activeNpcAction_ = binding;

    faceNpcTowardActionTarget();

    if (binding.sequenceId.has_value()) {
        startAnimationPreview(binding.sequenceId);
        animationPlayer_.setLooping(false);
    }

    appendActionEffects(binding);
}

void CacheExplorer::startItemActionPreview(
    const eld::animation::presentation::AnimationBinding& binding
) {
    if (
        !state_.activeItem.has_value() ||
        !animationSource_.has_value()
    ) {
        return;
    }

    clearNpcActionPreview();
    activeItemAction_ = binding;

    // Player models use the same authored +Z forward axis as the classic NPC
    // models. Face the mannequin toward the target once when starting a combat
    // preview so projectile/special effects leave in the direction Bob faces.
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
        startAnimationPreview(binding.sequenceId);
        animationPlayer_.setLooping(false);
    }

    appendActionEffects(binding);
}

void CacheExplorer::showItemInventoryPreview() {
    if (!state_.activeItem.has_value()) {
        return;
    }

    const ItemPreviewBuilder builder;
    std::optional<eld::model::Model> preview =
        builder.build(*state_.activeItem, modelRepository_);

    if (!preview.has_value()) {
        return;
    }

    resetAnimationPreview();

    state_.activeModelHandle =
        graphicsResources_.resolveModel(preview->mesh);
    state_.activeModel = std::move(*preview);
    itemPreviewMode_ = ItemPreviewMode::Inventory;
}

void CacheExplorer::showItemEquippedPreview(
    ItemPreviewGender gender
) {
    if (!state_.activeItem.has_value()) {
        return;
    }

    const ItemPreviewBuilder builder;
    std::optional<eld::model::Model> preview =
        builder.buildEquipped(
            *state_.activeItem,
            gender,
            identityKitRepository_,
            modelRepository_
        );

    if (!preview.has_value()) {
        return;
    }

    resetAnimationPreview();

    state_.activeModelHandle =
        graphicsResources_.resolveModel(preview->mesh);
    state_.activeModel = std::move(*preview);

    animationSource_ = state_.activeModel->mesh;
    animationPreviewKind_ = AnimationPreviewKind::Item;

    itemPreviewMode_ =
        gender == ItemPreviewGender::Male
            ? ItemPreviewMode::MaleEquipped
            : ItemPreviewMode::FemaleEquipped;

    // Classic default player standing animation (human_ready).
    startAnimationPreview(std::uint16_t{808});
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
            // human-sized server height blindly to tiny actor previews. RuneTek
            // projectile heights are scene-space values; some reconstructed
            // bindings only tell us the projectile identity, not actor-specific
            // launch geometry. Cap the requested launch point to the current
            // preview actor's upper body in graphics space. Exact lower values
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

void CacheExplorer::selectNextNpcWithProjectile() {
    if (!state_.activeNpc.has_value()) {
        return;
    }

    const std::uint16_t currentId =
        state_.activeNpc->id;

    const auto hasProjectile =
        [this](const eld::definition::NpcDefinition& npc) {
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
                                spotAnimationRepository_.find(
                                    effect.spotAnimationId
                                ) != nullptr;
                        }
                    );
                }
            );
        };

    const auto selectNpc =
        [this](const eld::definition::NpcDefinition& npc) {
            state_.selection.type =
                CacheTreeNodeType::NpcDefinition;

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

    const auto& npcs =
        npcRepository_.list();

    for (const eld::definition::NpcDefinition& npc : npcs) {
        if (
            npc.id > currentId &&
            hasProjectile(npc)
        ) {
            selectNpc(npc);
            return;
        }
    }

    // Wrap so the button can be hammered continuously while reviewing NPCs.
    for (const eld::definition::NpcDefinition& npc : npcs) {
        if (
            npc.id <= currentId &&
            hasProjectile(npc)
        ) {
            selectNpc(npc);
            return;
        }
    }
}

void CacheExplorer::selectNextWearableItem() {
    if (!state_.activeItem.has_value()) {
        return;
    }

    const std::uint16_t currentId =
        state_.activeItem->id;

    const ItemPreviewMode previousMode =
        itemPreviewMode_;

    const ItemPreviewBuilder previewBuilder;

    const auto isWearable =
        [&previewBuilder](const eld::definition::ItemDefinition& item) {
            return
                previewBuilder.hasEquippedModel(
                    item,
                    ItemPreviewGender::Male
                ) ||
                previewBuilder.hasEquippedModel(
                    item,
                    ItemPreviewGender::Female
                );
        };

    const auto selectItem =
        [this, previousMode, &previewBuilder](
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

            if (previousMode == ItemPreviewMode::MaleEquipped) {
                if (previewBuilder.hasEquippedModel(
                    *state_.activeItem,
                    ItemPreviewGender::Male
                )) {
                    showItemEquippedPreview(ItemPreviewGender::Male);
                    return;
                }
            }
            else if (previousMode == ItemPreviewMode::FemaleEquipped) {
                if (previewBuilder.hasEquippedModel(
                    *state_.activeItem,
                    ItemPreviewGender::Female
                )) {
                    showItemEquippedPreview(ItemPreviewGender::Female);
                    return;
                }
            }

            // If the current view was equipped but the next item lacks that
            // gender's model, stay in an equipped view using the other gender
            // when possible instead of unexpectedly dropping to raw-item view.
            if (previousMode != ItemPreviewMode::Inventory) {
                if (previewBuilder.hasEquippedModel(
                    *state_.activeItem,
                    ItemPreviewGender::Male
                )) {
                    showItemEquippedPreview(ItemPreviewGender::Male);
                }
                else if (previewBuilder.hasEquippedModel(
                    *state_.activeItem,
                    ItemPreviewGender::Female
                )) {
                    showItemEquippedPreview(ItemPreviewGender::Female);
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
            actionBindingLabel(binding);

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

    renderAnimationPlaybackControls();
    renderManualNpcActionComposer();
    ImGui::Separator();
}

void CacheExplorer::renderItemAnimationControls() {
    if (!state_.activeItem.has_value()) {
        return;
    }

    const eld::definition::ItemDefinition& item =
        *state_.activeItem;

    const ItemPreviewBuilder previewBuilder;
    const bool hasMale =
        previewBuilder.hasEquippedModel(
            item,
            ItemPreviewGender::Male
        );
    const bool hasFemale =
        previewBuilder.hasEquippedModel(
            item,
            ItemPreviewGender::Female
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

    ImGui::TextUnformatted("ITEM PREVIEW");

    if (ImGui::Button("Item")) {
        showItemInventoryPreview();
    }

    if (hasMale) {
        ImGui::SameLine();
        if (ImGui::Button("Male equip")) {
            showItemEquippedPreview(ItemPreviewGender::Male);
        }
    }

    if (hasFemale) {
        ImGui::SameLine();
        if (ImGui::Button("Female equip")) {
            showItemEquippedPreview(ItemPreviewGender::Female);
        }
    }

    if (!hasMale && !hasFemale) {
        ImGui::SameLine();
        ImGui::TextDisabled("No worn model");
    }

    const char* viewLabel = "item";

    if (itemPreviewMode_ == ItemPreviewMode::MaleEquipped) {
        viewLabel = "male equipped";
    }
    else if (itemPreviewMode_ == ItemPreviewMode::FemaleEquipped) {
        viewLabel = "female equipped";
    }

    ImGui::Text(
        "View: %s",
        viewLabel
    );

    if (
        itemPreviewMode_ == ItemPreviewMode::Inventory ||
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
        clearNpcActionPreview();
        startAnimationPreview(std::uint16_t{808});
        animationPlayer_.setLooping(true);
    }

    if (sequenceAvailable(819)) {
        ImGui::SameLine();
        if (ImGui::Button("Walk")) {
            clearNpcActionPreview();
            startAnimationPreview(std::uint16_t{819});
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
                startItemActionPreview(binding);
            }

            firstAction = false;
            hasActions = true;
        };

    const std::vector<eld::animation::presentation::AnimationBinding>
        generatedActions =
            genericItemActionBindings(item);

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
            genericItemDefendSequence(item);

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

    renderAnimationPlaybackControls();
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

CacheExplorer::CacheExplorer()
    : cache_("cache"),
      mapLoader_(cache_),
      midiRepository_(
          cache_.open(
              eld::cache::IndexId::Midi
          )
      ),
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

void CacheExplorer::shutdown() {
    midiPlayer_.shutdown();
    viewportPanel_.shutdown();
}

bool CacheExplorer::initialize() {
    // ELFORGE_CAMERA_NAVIGATION_V1
    state_.viewportCameraPivot = {
        0.0f,
        0.0f,
        0.0f
    };

    state_.viewportCameraDistance =
        650.0f;

    state_.camera.rotation = {
        0.42f,
        -0.55f,
        0.0f
    };

    const float pitch =
        state_.camera.rotation.x;

    const float yaw =
        state_.camera.rotation.y;

    const eld::math::Vec3 forward{
        std::cos(pitch) * std::sin(yaw),
        -std::sin(pitch),
        std::cos(pitch) * std::cos(yaw)
    };

    state_.camera.position =
        state_.viewportCameraPivot -
        forward *
            state_.viewportCameraDistance;

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

    if (!midiPlayer_.initialize()) {
        state_.midiPlaybackStatus =
            midiPlayer_.statusMessage();
    }
    else {
        state_.midiPlaybackStatus =
            "MIDI playback ready";
    }

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

    if (state_.animationDumpAllRequested) {
        state_.animationDumpAllRequested = false;

        try {
            const AnimationRelations relations(
                animationRepository_,
                animationFrameIndex_,
                sequenceRepository_,
                npcRepository_,
                locationRepository_,
                spotAnimationRepository_,
                itemRepository_,
                interfaceRepository_,
                animationPresentationCatalog_
            );

            const std::filesystem::path path =
                defaultAnimationRelationsDumpPath();

            std::string error;

            if (
                dumpAllAnimationRelations(
                    relations,
                    path,
                    error
                )
            ) {
                state_.animationDumpStatus =
                    "Exported: " +
                    path.string();
            }
            else {
                state_.animationDumpStatus =
                    "Export failed: " +
                    error;
            }
        }
        catch (const std::exception& exception) {
            state_.animationDumpStatus =
                std::string("Export failed: ") +
                exception.what();
        }
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

    // ELFORGE_TARGET_LOCK_PRESETS_V1
    //
    // An active semantic/composed action owns facing while this lock is on.
    // Moving the NPC or moving the target therefore recomputes yaw every
    // frame instead of taking a one-time snapshot at action start.
    if (
        lockNpcFacingToActionTarget_ &&
        activeNpcAction_.has_value() &&
        state_.activeNpc.has_value()
    ) {
        faceNpcTowardActionTarget();
    }

}

void CacheExplorer::handleSelectionChanged() {
    resetAnimationPreview();

    midiPlayer_.unload();

    state_.activeAnimation.reset();
    state_.animationDumpStatus.clear();
    state_.animationDumpAllRequested = false;

    state_.activeMidi.reset();
    state_.midiExportStatus.clear();
    state_.midiPlaybackStatus.clear();
    state_.midiSeekPreviewTick = 0;
    state_.midiSeekActive = false;
    state_.activeMap.reset();
    state_.mapPreviewError.clear();
    state_.selectedMapTile.reset();
    state_.selectedMapLocIndex.reset();
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

        case CacheTreeNodeType::Animation: {
            if (
                state_.selection.fileId < 0 ||
                state_.selection.fileId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            try {
                const AnimationRelations relations(
                    animationRepository_,
                    animationFrameIndex_,
                    sequenceRepository_,
                    npcRepository_,
                    locationRepository_,
                    spotAnimationRepository_,
                    itemRepository_,
                    interfaceRepository_,
                    animationPresentationCatalog_
                );

                state_.activeAnimation =
                    relations.inspect(
                        static_cast<std::uint16_t>(
                            state_.selection.fileId
                        )
                    );
            }
            catch (const std::exception& exception) {
                state_.activeAnimation.reset();
                state_.animationDumpStatus =
                    std::string("Failed to inspect animation: ") +
                    exception.what();
            }

            break;
        }

        case CacheTreeNodeType::Midi: {
            if (
                state_.selection.fileId < 0 ||
                state_.selection.fileId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                break;
            }

            try {
                state_.activeMidi =
                    midiRepository_.get(
                        static_cast<std::uint16_t>(
                            state_.selection.fileId
                        )
                    );

                if (midiPlayer_.isAvailable()) {
                    if (midiPlayer_.load(
                            state_.activeMidi->data.bytes
                        )) {
                        state_.midiPlaybackStatus =
                            "Ready to play";
                    }
                    else {
                        state_.midiPlaybackStatus =
                            midiPlayer_.statusMessage();
                    }
                }
                else {
                    state_.midiPlaybackStatus =
                        midiPlayer_.statusMessage();
                }
            }
            catch (const std::exception& exception) {
                const std::string message =
                    std::string("Failed to load MIDI: ") +
                    exception.what();

                state_.midiExportStatus = message;
                state_.midiPlaybackStatus = message;
            }

            break;
        }

        case CacheTreeNodeType::MapRegion: {
            if (
                state_.selection.regionId < 0 ||
                state_.selection.regionId >
                    std::numeric_limits<std::uint16_t>::max()
            ) {
                state_.mapPreviewError =
                    "Selected map region id is invalid";
                break;
            }

            try {
                const MapPreviewBuilder previewBuilder(
                    mapLoader_,
                    floorRepository_,
                    locationRepository_,
                    modelRepository_,
                    graphicsResources_
                );

                state_.activeMap =
                    previewBuilder.build(
                        static_cast<std::uint16_t>(
                            state_.selection.regionId
                        )
                    );

                resetMapView(
                    state_.mapPlane,
                    state_.mapYaw,
                    state_.mapPitch,
                    state_.mapDistance
                );

                state_.mapShowTerrain = true;
                state_.mapShowLocs = true;
                state_.mapViewportDirty = true;
            }
            catch (const std::exception& exception) {
                state_.activeMap.reset();
                state_.mapPreviewError =
                    exception.what();
            }

            break;
        }

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
                state_.activeItem = *definition;
                showItemInventoryPreview();
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

void CacheExplorer::prepareViewport(
    SDL_Renderer* renderer
) {
    viewportPanel_.prepareViewport(
        renderer,
        state_,
        graphicsResources_
    );
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

    // ELFORGE_REMOVE_FIND_ALPHA_BUTTON_V1

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
        midiPlayer_,
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
