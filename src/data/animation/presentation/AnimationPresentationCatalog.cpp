#include "AnimationPresentationCatalog.h"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace eld::animation::presentation {

namespace {

std::string trim(
    std::string value
) {
    const auto first =
        value.find_first_not_of(" \t\r\n");

    if (first == std::string::npos) {
        return {};
    }

    const auto last =
        value.find_last_not_of(" \t\r\n");

    return value.substr(
        first,
        last - first + 1
    );
}

std::vector<std::string> splitCsv(
    const std::string& line
) {
    std::vector<std::string> values;
    std::stringstream stream(line);
    std::string value;

    while (std::getline(stream, value, ',')) {
        values.push_back(trim(value));
    }

    // kind,id,action,sequence,spotanim,projectile,delay_ms,duration_ms,
    // variant,placement,start_height,end_height,slope,start_distance
    while (values.size() < 14) {
        values.emplace_back();
    }

    return values;
}

std::optional<std::uint32_t> parseU32(
    const std::string& text
) {
    if (text.empty()) {
        return std::nullopt;
    }

    std::uint32_t value = 0;

    const auto result =
        std::from_chars(
            text.data(),
            text.data() + text.size(),
            value
        );

    if (
        result.ec != std::errc{} ||
        result.ptr != text.data() + text.size()
    ) {
        return std::nullopt;
    }

    return value;
}

std::optional<std::uint16_t> parseU16(
    const std::string& text
) {
    const auto value = parseU32(text);

    if (
        !value.has_value() ||
        *value > 65535u
    ) {
        return std::nullopt;
    }

    return static_cast<std::uint16_t>(*value);
}

bool parseBool(
    std::string_view text
) {
    return
        text == "1" ||
        text == "true" ||
        text == "yes" ||
        text == "projectile";
}

void addDefinitionBinding(
    std::vector<AnimationBinding>& bindings,
    AnimationAction action,
    const std::optional<std::uint16_t>& sequenceId
) {
    if (!sequenceId.has_value()) {
        return;
    }

    AnimationBinding binding;
    binding.action = action;
    binding.sequenceId = sequenceId;

    bindings.push_back(
        std::move(binding)
    );
}

void mergeBinding(
    std::vector<AnimationBinding>& bindings,
    const AnimationBinding& authored
) {
    auto existing =
        std::find_if(
            bindings.begin(),
            bindings.end(),
            [&](const AnimationBinding& binding) {
                return
                    binding.action == authored.action &&
                    binding.variant == authored.variant;
            }
        );

    if (existing == bindings.end()) {
        bindings.push_back(authored);
        return;
    }

    if (authored.sequenceId.has_value()) {
        existing->sequenceId = authored.sequenceId;
    }

    existing->effects.insert(
        existing->effects.end(),
        authored.effects.begin(),
        authored.effects.end()
    );
}

}

AnimationPresentationCatalog::AnimationPresentationCatalog(
    const std::filesystem::path& path
) {
    load(path);
}

void AnimationPresentationCatalog::load(
    const std::filesystem::path& path
) {
    authored_.clear();

    std::ifstream input(path);

    if (!input.is_open()) {
        return;
    }

    std::string line;

    while (std::getline(input, line)) {
        line = trim(line);

        if (
            line.empty() ||
            line.front() == '#' ||
            line.rfind("kind,", 0) == 0
        ) {
            continue;
        }

        const std::vector<std::string> fields =
            splitCsv(line);

        const std::optional<std::uint16_t> entityId =
            parseU16(fields[1]);

        const std::optional<AnimationAction> action =
            animationActionFromString(fields[2]);

        if (
            !entityId.has_value() ||
            !action.has_value()
        ) {
            continue;
        }

        EntityKind kind;

        if (fields[0] == "npc") {
            kind = EntityKind::Npc;
        }
        else if (fields[0] == "item") {
            kind = EntityKind::Item;
        }
        else {
            continue;
        }

        const Key key{
            kind,
            *entityId,
            *action,
            fields[8]
        };

        AnimationBinding& binding =
            authored_[key];

        binding.action = *action;
        binding.variant = fields[8];

        const std::optional<std::uint16_t> sequenceId =
            parseU16(fields[3]);

        if (sequenceId.has_value()) {
            binding.sequenceId = sequenceId;
        }

        const std::optional<std::uint16_t> spotAnimationId =
            parseU16(fields[4]);

        if (spotAnimationId.has_value()) {
            AnimationEffectBinding effect;
            effect.spotAnimationId = *spotAnimationId;
            effect.projectile = parseBool(fields[5]);

            if (fields[9] == "target") {
                effect.projectile = false;
                effect.target = true;
            }
            else if (fields[9] == "projectile") {
                effect.projectile = true;
            }
            else if (
                fields[9] == "source" ||
                fields[9] == "attached"
            ) {
                effect.projectile = false;
            }

            // Classic RuneTek defaults from the preserved projectile helpers.
            // Generic NPC ranged: 40,36,... angle=15, offset=11.
            // Magic helper example: 43,31,... angle=16, offset=64.
            if (effect.projectile && *action == AnimationAction::Cast) {
                effect.projectileStartHeight = 43;
                effect.projectileEndHeight = 31;
                effect.projectileSlope = 16;
                effect.projectileStartDistance = 64;
            }

            if (const auto value = parseU16(fields[10])) {
                effect.projectileStartHeight = *value;
            }

            if (const auto value = parseU16(fields[11])) {
                effect.projectileEndHeight = *value;
            }

            if (const auto value = parseU16(fields[12])) {
                effect.projectileSlope = *value;
            }

            if (const auto value = parseU16(fields[13])) {
                effect.projectileStartDistance = *value;
            }

            if (const auto delay = parseU32(fields[6])) {
                effect.delayMilliseconds = *delay;
            }

            if (const auto duration = parseU32(fields[7])) {
                effect.durationMilliseconds =
                    std::max<std::uint32_t>(*duration, 1u);
            }

            binding.effects.push_back(effect);
        }
    }
}

NpcAnimationProfile AnimationPresentationCatalog::resolveNpc(
    const eld::definition::NpcDefinition& definition
) const {
    NpcAnimationProfile profile;
    profile.npcId = definition.id;

    addDefinitionBinding(
        profile.bindings,
        AnimationAction::Idle,
        definition.idleAnimationId
    );

    addDefinitionBinding(
        profile.bindings,
        AnimationAction::Walk,
        definition.walkAnimationId
    );

    addDefinitionBinding(
        profile.bindings,
        AnimationAction::TurnAround,
        definition.turnAroundAnimationId
    );

    addDefinitionBinding(
        profile.bindings,
        AnimationAction::TurnLeft,
        definition.turnLeftAnimationId
    );

    addDefinitionBinding(
        profile.bindings,
        AnimationAction::TurnRight,
        definition.turnRightAnimationId
    );

    for (const auto& [key, binding] : authored_) {
        if (
            key.kind == EntityKind::Npc &&
            key.id == definition.id
        ) {
            mergeBinding(
                profile.bindings,
                binding
            );
        }
    }

    return profile;
}

ItemAnimationProfile AnimationPresentationCatalog::resolveItem(
    const eld::definition::ItemDefinition& definition
) const {
    ItemAnimationProfile profile;
    profile.itemId = definition.id;

    for (const auto& [key, binding] : authored_) {
        if (
            key.kind == EntityKind::Item &&
            key.id == definition.id
        ) {
            mergeBinding(
                profile.bindings,
                binding
            );
        }
    }

    return profile;
}

}
