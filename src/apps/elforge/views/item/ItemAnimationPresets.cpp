#include "views/item/ItemAnimationPresets.h"

#include <algorithm>
#include <cctype>
#include <string>

#include "Item.h"

namespace eld::elforge {

namespace {

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
    const eld::item::Item& definition,
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
    const eld::item::Item& definition,
    eld::item::ItemWearPosition wanted
) {
    return
        definition.wearPosition == wanted ||
        definition.wearPosition2 == wanted ||
        definition.wearPosition3 == wanted;
}

bool itemLooksLikeShield(
    const eld::item::Item& definition
) {
    if (
        itemHasWearPosition(
            definition,
            eld::item::ItemWearPosition::LeftHand
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
    const eld::item::Item& definition
) {
    if (itemLooksLikeShield(definition)) {
        return false;
    }

    if (
        itemHasWearPosition(
            definition,
            eld::item::ItemWearPosition::RightHand
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
    const eld::item::Item& definition
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
    const eld::item::Item& definition
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

}

std::vector<eld::animation::presentation::AnimationBinding>
ItemAnimationPresets::actions(
    const eld::item::Item& definition
) {
    return genericItemActionBindings(definition);
}

std::optional<std::uint16_t>
ItemAnimationPresets::defendSequence(
    const eld::item::Item& definition
) {
    return genericItemDefendSequence(definition);
}

}
