#!/usr/bin/env python3
"""
Import historically sourced NPC combat presentation into Eldoria.

This importer focuses on the server-side relationships that are missing from the
client NPC definitions: attack/cast body sequences, launch/casting SpotAnims,
travelling projectiles, and (where explicit server code gives one) target impact
GFX.

Sources:
- RustCityRS/rs-majula revision 289 symbolic NPC configs + numeric pack files.
- 2006-Scape/2006Scape NpcCombat/NpcEmotes behavior for explicit spell/special
  variants not represented by a single NPC config row.

The generated block is idempotent. Hand-authored rows outside the generated
block are never removed and are used to avoid duplicating already-researched
effects.
"""

from __future__ import annotations

import argparse
import csv
import io
import re
import sys
import urllib.request
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable

BINDINGS_PATH = Path("content/animation_bindings.csv")
PROVENANCE_PATH = Path("content/animation_binding_provenance.csv")

BEGIN = "# BEGIN AUTO-IMPORTED HISTORICAL NPC ORCHESTRATION"
END = "# END AUTO-IMPORTED HISTORICAL NPC ORCHESTRATION"

RS_MAJULA_COMMIT = "767d3bfedafc3fd91d5d994cdfffed2cec628dcc"
RS_BASE = (
    "https://raw.githubusercontent.com/RustCityRS/rs-majula/"
    f"{RS_MAJULA_COMMIT}/content/289"
)

PACK_URLS = {
    "npc": f"{RS_BASE}/pack/npc.pack",
    "seq": f"{RS_BASE}/pack/seq.pack",
    "spotanim": f"{RS_BASE}/pack/spotanim.pack",
}

# GitHub code-search for `param=proj_travel` in revision 289 reduces to these
# authored config families (older-revision/_unpack hits are duplicates).
NPC_CONFIG_PATHS = [
    "scripts/areas/area_ardougne_east/configs/ardougne_east.npc",
    "scripts/areas/area_gnome/configs/gnome.npc",
    "scripts/minigames/game_ranging/configs/ranging.npc",
    "scripts/quests/quest_death/configs/quest_death.npc",
    "scripts/quests/quest_horror/configs/quest_horror.npc",
    "scripts/quests/quest_regicide/configs/quest_regicide.npc",
    "scripts/quests/quest_troll/configs/quest_troll.npc",
]

SCAPE_COMBAT_URL = (
    "https://github.com/2006-Scape/2006Scape/blob/master/"
    "2006Scape%20Server/src/main/java/com/rs2/game/content/combat/npcs/"
    "NpcCombat.java"
)
SCAPE_EMOTES_URL = (
    "https://github.com/2006-Scape/2006Scape/blob/master/"
    "2006Scape%20Server/src/main/java/com/rs2/game/content/combat/npcs/"
    "NpcEmotes.java"
)
IKOV_SCRIPT_URL = (
    "https://github.com/RustCityRS/rs-majula/blob/"
    f"{RS_MAJULA_COMMIT}/content/289/scripts/quests/quest_ikov/scripts/"
    "ikov_dungeon.rs2"
)


@dataclass
class Effect:
    spotanim: int
    placement: str
    delay_ms: int = 0
    duration_ms: int = 700
    symbol: str = ""


@dataclass
class Orchestration:
    npc_id: int
    action: str
    sequence: int | None
    effects: list[Effect]
    variant: str = ""
    npc_symbol: str = ""
    sequence_symbol: str = ""
    source: str = ""
    confidence: str = "high"
    evidence: str = ""


@dataclass
class Existing:
    sequences: dict[tuple[int, str, str], int] = field(default_factory=dict)
    effects: set[tuple[int, str, str, int, str]] = field(default_factory=set)
    effect_variants: dict[tuple[int, str, int], str] = field(default_factory=dict)


def fetch_text(url: str) -> str:
    request = urllib.request.Request(
        url,
        headers={"User-Agent": "Eldoria-historical-animation-importer/1"},
    )
    with urllib.request.urlopen(request, timeout=30) as response:
        return response.read().decode("utf-8")


def parse_pack(text: str) -> tuple[dict[str, int], dict[int, str]]:
    by_name: dict[str, int] = {}
    by_id: dict[int, str] = {}
    for raw in text.splitlines():
        line = raw.strip()
        if not line or "=" not in line:
            continue
        left, right = line.split("=", 1)
        try:
            ident = int(left)
        except ValueError:
            continue
        name = right.strip()
        by_id[ident] = name
        by_name.setdefault(name, ident)
    return by_name, by_id


def parse_npc_blocks(text: str) -> list[tuple[str, dict[str, str]]]:
    blocks: list[tuple[str, dict[str, str]]] = []
    symbol = ""
    values: dict[str, str] = {}

    def flush() -> None:
        nonlocal symbol, values
        if symbol:
            blocks.append((symbol, values))
        symbol = ""
        values = {}

    for raw in text.splitlines():
        line = raw.strip()
        if not line or line.startswith("//"):
            continue

        if line.startswith("[") and line.endswith("]"):
            flush()
            symbol = line[1:-1].strip()
            continue

        if not symbol or "=" not in line:
            continue

        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip()

        if key == "param" and "," in value:
            param_name, param_value = value.split(",", 1)
            values[f"param:{param_name.strip()}"] = param_value.strip()
        else:
            values[key] = value

    flush()
    return blocks


def strip_generated_block(lines: list[str]) -> list[str]:
    output: list[str] = []
    skipping = False
    for line in lines:
        if line.strip() == BEGIN:
            skipping = True
            continue
        if line.strip() == END:
            skipping = False
            continue
        if not skipping:
            output.append(line)
    while output and not output[-1].strip():
        output.pop()
    return output


def placement_from_fields(fields: list[str]) -> str:
    if len(fields) > 9 and fields[9]:
        value = fields[9].strip().lower()
        if value in {"source", "attached", "projectile", "target"}:
            return "source" if value == "attached" else value
    projectile = len(fields) > 5 and fields[5].strip().lower() in {
        "1", "true", "yes", "projectile"
    }
    return "projectile" if projectile else "source"


def parse_existing(lines: Iterable[str]) -> Existing:
    result = Existing()
    for raw in lines:
        line = raw.strip()
        if not line or line.startswith("#") or line.startswith("kind,"):
            continue

        fields = [value.strip() for value in line.split(",")]
        while len(fields) < 10:
            fields.append("")

        if fields[0] != "npc":
            continue

        try:
            npc_id = int(fields[1])
        except ValueError:
            continue

        action = fields[2]
        variant = fields[8]

        if fields[3]:
            try:
                result.sequences[(npc_id, action, variant)] = int(fields[3])
            except ValueError:
                pass

        if fields[4]:
            try:
                spotanim = int(fields[4])
            except ValueError:
                continue
            placement = placement_from_fields(fields)
            result.effects.add((npc_id, action, variant, spotanim, placement))
            result.effect_variants.setdefault(
                (npc_id, action, spotanim),
                variant,
            )
    return result


def pretty_variant(symbol: str) -> str:
    value = symbol
    for suffix in ("_travel", "_launch", "_casting", "_impact"):
        if value.endswith(suffix):
            value = value[: -len(suffix)]
            break
    value = re.sub(r"^sp_attack_", "", value)
    value = re.sub(r"^sp_attack", "", value)
    if re.fullmatch(r"spotanim_\d+", value):
        return "Ranged"
    words = value.replace("_", " ").strip()
    if not words:
        return "Projectile"
    return words[0].upper() + words[1:]


def is_spell_symbol(symbol: str) -> bool:
    lowered = symbol.lower()
    spell_tokens = (
        "strike", "bolt", "blast", "wave", "iban", "spell",
        "magic", "flame", "curse", "weaken", "confuse",
        "entangle", "snare", "bind",
    )
    return any(token in lowered for token in spell_tokens)


def config_orchestrations(
    *,
    npc_by_name: dict[str, int],
    seq_by_name: dict[str, int],
    spot_by_name: dict[str, int],
) -> tuple[list[Orchestration], list[str]]:
    mappings: list[Orchestration] = []
    unresolved: list[str] = []

    for path in NPC_CONFIG_PATHS:
        url = f"{RS_BASE}/{path}"
        text = fetch_text(url)

        for npc_symbol, values in parse_npc_blocks(text):
            seq_symbol = values.get("param:attack_anim", "")
            launch_symbol = values.get("param:proj_launch", "")
            travel_symbol = values.get("param:proj_travel", "")

            if not seq_symbol or (not launch_symbol and not travel_symbol):
                continue

            npc_id = npc_by_name.get(npc_symbol)
            sequence = seq_by_name.get(seq_symbol)

            if npc_id is None or sequence is None:
                unresolved.append(
                    f"{path}: [{npc_symbol}] npc={npc_id} "
                    f"sequence={seq_symbol}:{sequence}"
                )
                continue

            effects: list[Effect] = []
            unresolved_effect = False

            if launch_symbol:
                launch_id = spot_by_name.get(launch_symbol)
                if launch_id is None:
                    unresolved.append(
                        f"{path}: [{npc_symbol}] launch={launch_symbol}"
                    )
                    unresolved_effect = True
                else:
                    effects.append(
                        Effect(
                            spotanim=launch_id,
                            placement="source",
                            delay_ms=0,
                            duration_ms=700,
                            symbol=launch_symbol,
                        )
                    )

            if travel_symbol:
                travel_id = spot_by_name.get(travel_symbol)
                if travel_id is None:
                    unresolved.append(
                        f"{path}: [{npc_symbol}] travel={travel_symbol}"
                    )
                    unresolved_effect = True
                else:
                    effects.append(
                        Effect(
                            spotanim=travel_id,
                            placement="projectile",
                            delay_ms=120,
                            duration_ms=700,
                            symbol=travel_symbol,
                        )
                    )

            if not effects or unresolved_effect:
                # Keep partial evidence only when at least one concrete effect
                # resolved. The unresolved item still lands in the report.
                if not effects:
                    continue

            action = "cast" if is_spell_symbol(travel_symbol) else "attack"
            variant = pretty_variant(travel_symbol or launch_symbol)

            mappings.append(
                Orchestration(
                    npc_id=npc_id,
                    action=action,
                    sequence=sequence,
                    effects=effects,
                    variant=variant,
                    npc_symbol=npc_symbol,
                    sequence_symbol=seq_symbol,
                    source=url,
                    confidence="high",
                    evidence=(
                        "NPC config explicitly supplies attack_anim and "
                        "proj_launch/proj_travel."
                    ),
                )
            )

    return mappings, unresolved


# Explicit server-side variants that cannot be represented by a single static
# NPC config row. Empty variant deliberately enriches a previously authored
# Eldoria action instead of creating a duplicate button.
SERVER_VARIANTS: list[Orchestration] = [
    Orchestration(
        277, "cast", 711,
        [
            Effect(129, "source", 0, 700, "fireblast_casting"),
            Effect(130, "projectile", 120, 700, "fireblast_travel"),
            Effect(131, "target", 820, 500, "fireblast_impact"),
        ],
        "", "ikov_firewarrior", "human_caststrike", IKOV_SCRIPT_URL,
        "high",
        "Quest script explicitly animates the NPC, casting GFX, travel projectile, and impact GFX.",
    ),
    *[
        Orchestration(
            npc_id, "cast", 711,
            [
                Effect(96, "source", 0, 700, "earthstrike_casting"),
                Effect(97, "projectile", 120, 700, "earthstrike_travel"),
                Effect(98, "target", 820, 500, "earthstrike_impact"),
            ],
            "", symbol, "human_caststrike", SCAPE_COMBAT_URL, "high",
            "NpcCombat explicitly assigns Earth Strike casting/projectile/end GFX.",
        )
        for npc_id, symbol in [
            (13, "wizard"),
            (172, "bearded_dark_wizard"),
            (174, "young_dark_wizard"),
        ]
    ],
    Orchestration(
        2025, "cast", 729,
        [
            Effect(158, "source", 0, 700, "windwave_casting"),
            Effect(159, "projectile", 120, 700, "windwave_travel"),
            Effect(160, "target", 820, 500, "windwave_impact"),
        ],
        "", "ahrim", "seq_729", SCAPE_COMBAT_URL, "high",
        "NpcCombat random spell branch: Wind Wave casting/projectile/end GFX.",
    ),
    Orchestration(
        2025, "cast", 729,
        [
            Effect(161, "source", 0, 700, "waterwave_casting"),
            Effect(162, "projectile", 120, 700, "waterwave_travel"),
            Effect(163, "target", 820, 500, "waterwave_impact"),
        ],
        "Water wave", "ahrim", "seq_729", SCAPE_COMBAT_URL, "high",
        "NpcCombat random spell branch: Water Wave casting/projectile/end GFX.",
    ),
    Orchestration(
        2025, "cast", 729,
        [
            Effect(164, "source", 0, 700, "earthwave_casting"),
            Effect(165, "projectile", 120, 700, "earthwave_travel"),
            Effect(166, "target", 820, 500, "earthwave_impact"),
        ],
        "Earth wave", "ahrim", "seq_729", SCAPE_COMBAT_URL, "high",
        "NpcCombat random spell branch: Earth Wave casting/projectile/end GFX.",
    ),
    Orchestration(
        2025, "cast", 729,
        [
            Effect(155, "source", 0, 700, "firewave_casting"),
            Effect(156, "projectile", 120, 700, "firewave_travel"),
        ],
        "Fire wave", "ahrim", "seq_729", SCAPE_COMBAT_URL, "high",
        "NpcCombat random spell branch explicitly gives Fire Wave casting/projectile; no end GFX is assigned in that branch.",
    ),
    Orchestration(
        2881, "attack", 2855,
        [
            Effect(162, "projectile", 120, 700, "waterwave_travel"),
            Effect(477, "target", 820, 500, "spotanim_477"),
        ],
        "", "dagannoth_prime", "seq_2855", SCAPE_COMBAT_URL, "medium",
        "2006Scape assigns projectile 162 and end GFX 477 to Dagannoth Prime.",
    ),
    Orchestration(
        2882, "attack", 2854,
        [Effect(298, "projectile", 120, 700, "spotanim_298")],
        "", "dagannoth_supreme", "seq_2854", SCAPE_COMBAT_URL, "medium",
        "2006Scape assigns projectile 298 to Dagannoth Supreme.",
    ),
    Orchestration(
        2028, "attack", 2075,
        [Effect(27, "projectile", 120, 700, "crossbowbolt_travel")],
        "", "karil", "seq_2075", SCAPE_COMBAT_URL, "medium",
        "2006Scape assigns projectile 27 to Karil.",
    ),
    Orchestration(
        2892, "attack", 2868,
        [
            Effect(94, "projectile", 120, 700, "spotanim_94"),
            Effect(95, "target", 820, 500, "spotanim_95"),
        ],
        "", "spinolyp", "seq_2868", SCAPE_COMBAT_URL, "medium",
        "2006Scape assigns magic projectile 94 and end GFX 95 to Spinolyp 2892.",
    ),
    Orchestration(
        2894, "attack", 2868,
        [Effect(298, "projectile", 120, 700, "spotanim_298")],
        "", "spinolyp_2894", "seq_2868", SCAPE_COMBAT_URL, "medium",
        "2006Scape assigns ranged projectile 298 to Spinolyp 2894.",
    ),
    Orchestration(
        3200, "attack", 3146,
        [
            Effect(550, "source", 0, 700, "spotanim_550"),
            Effect(551, "projectile", 120, 700, "spotanim_551"),
            Effect(552, "target", 820, 500, "spotanim_552"),
        ],
        "", "chaos_elemental", "seq_3146", SCAPE_COMBAT_URL, "medium",
        "2006Scape range branch assigns source/projectile/end GFX 550/551/552.",
    ),
    Orchestration(
        3200, "special_attack", 3146,
        [
            Effect(553, "source", 0, 700, "spotanim_553"),
            Effect(554, "projectile", 120, 700, "spotanim_554"),
            Effect(555, "target", 820, 500, "spotanim_555"),
        ],
        "", "chaos_elemental", "seq_3146", SCAPE_COMBAT_URL, "medium",
        "2006Scape magic branch assigns source/projectile/end GFX 553/554/555.",
    ),
    Orchestration(
        2745, "cast", 2656,
        [
            Effect(448, "projectile", 120, 700, "spotanim_448"),
            Effect(157, "target", 820, 500, "firewave_impact"),
        ],
        "", "tztok_jad", "seq_2656", SCAPE_COMBAT_URL, "medium",
        "2006Scape Jad magic branch assigns projectile 448 and end GFX 157.",
    ),
    Orchestration(
        2745, "special_attack", 2652,
        [Effect(451, "projectile", 120, 700, "spotanim_451")],
        "", "tztok_jad", "seq_2652", SCAPE_COMBAT_URL, "medium",
        "2006Scape Jad ranged branch assigns projectile 451.",
    ),
    Orchestration(
        2745, "attack", 2655, [],
        "Melee", "tztok_jad", "seq_2655", SCAPE_EMOTES_URL, "medium",
        "NpcEmotes selects sequence 2655 when Jad attackType is melee.",
    ),
    *[
        Orchestration(
            npc_id, "attack", 2644,
            [
                Effect(445, "projectile", 120, 700, "spotanim_445"),
                Effect(446, "target", 820, 500, "spotanim_446"),
            ],
            "", symbol, "seq_2644", SCAPE_COMBAT_URL, "medium",
            "2006Scape assigns projectile 445 and end GFX 446 to Ket-Zek.",
        )
        for npc_id, symbol in [(2743, "ket_zek"), (2744, "ket_zek_2")]
    ],
    *[
        Orchestration(
            npc_id, "attack", 2628,
            [Effect(443, "projectile", 120, 700, "spotanim_443")],
            "", symbol, "seq_2628", SCAPE_COMBAT_URL, "medium",
            "2006Scape assigns projectile 443 to Tok-Xil.",
        )
        for npc_id, symbol in [(2631, "tok_xil"), (2632, "tok_xil_2")]
    ],
    Orchestration(
        3068, "special_attack", 2989,
        [
            Effect(393, "projectile", 120, 700, "spotanim_393"),
            Effect(430, "target", 820, 500, "spotanim_430"),
        ],
        "Breath", "skeletal_wyvern", "seq_2989", SCAPE_COMBAT_URL, "medium",
        "2006Scape explicit Wyvern special uses sequence 2989, projectile 393, end GFX 430.",
    ),
    Orchestration(
        1159, "special_attack", 1185,
        [
            Effect(280, "projectile", 120, 700, "kalphite_glow_travel"),
            Effect(279, "target", 820, 500, "kalphite_queen_glow"),
        ],
        "", "kalphite_queen", "seq_1185", SCAPE_COMBAT_URL, "medium",
        "2006Scape first-form KQ magic branch assigns projectile 280 and end GFX 279.",
    ),
    Orchestration(
        1160, "special_attack", 1177,
        [
            Effect(279, "projectile", 120, 700, "kalphite_queen_glow"),
            Effect(278, "target", 820, 500, "kalphite_glow"),
        ],
        "", "kalphite_queen_2", "seq_1177", SCAPE_COMBAT_URL, "medium",
        "2006Scape second-form KQ ranged/magic branch assigns projectile 279 and end GFX 278.",
    ),
    # Existing red KBD/metal-dragon special is enriched with its target GFX.
    *[
        Orchestration(
            npc_id, "special_attack", 80,
            [
                Effect(393, "projectile", 120, 700, "spotanim_393"),
                Effect(430, "target", 820, 500, "spotanim_430"),
            ],
            "", symbol, "dragon_attack", SCAPE_COMBAT_URL, "medium",
            "2006Scape fire-breath branch assigns red projectile 393 and end GFX 430.",
        )
        for npc_id, symbol in [
            (50, "king_dragon"),
            (1590, "bronze_dragon"),
            (1591, "iron_dragon"),
            (1592, "steel_dragon"),
        ]
    ],
    *[
        Orchestration(
            50, "special_attack", 80,
            [
                Effect(projectile, "projectile", 120, 700, f"spotanim_{projectile}"),
                Effect(impact, "target", 820, 500, f"spotanim_{impact}"),
            ],
            variant, "king_dragon", "dragon_attack", SCAPE_COMBAT_URL, "medium",
            "2006Scape KBD random breath branch supplies this projectile/end-GFX pair.",
        )
        for variant, projectile, impact in [
            ("Green breath", 394, 429),
            ("White breath", 395, 431),
            ("Blue breath", 396, 428),
        ]
    ],
    *[
        Orchestration(
            npc_id, "special_attack", 91,
            [
                Effect(393, "projectile", 120, 700, "spotanim_393"),
                Effect(430, "target", 820, 500, "spotanim_430"),
            ],
            "", symbol, "dragon_head_attack", SCAPE_COMBAT_URL, "medium",
            "2006Scape normal dragon fire-breath branch assigns projectile 393 and end GFX 430.",
        )
        for npc_id, symbol in [(53, "red_dragon"), (54, "black_dragon"), (55, "blue_dragon")]
    ],
    Orchestration(
        941, "cast", 82,
        [
            Effect(394, "projectile", 120, 700, "spotanim_394"),
            Effect(429, "target", 820, 500, "spotanim_429"),
        ],
        "", "dragon_941", "dragon_firebreath_left_attack", SCAPE_COMBAT_URL, "medium",
        "Enriches the already-tested Eldoria green-breath binding with the server end GFX paired with projectile 394.",
    ),
]


def choose_variant(existing: Existing, mapping: Orchestration) -> str:
    # If a previous hand-authored row already contains one of these effects,
    # enrich that same binding instead of creating a duplicate variant button.
    for effect in mapping.effects:
        found = existing.effect_variants.get(
            (mapping.npc_id, mapping.action, effect.spotanim)
        )
        if found is not None:
            return found
    return mapping.variant


def binding_line(
    npc_id: int,
    action: str,
    sequence: int | None,
    effect: Effect | None,
    variant: str,
) -> str:
    sequence_text = "" if sequence is None else str(sequence)
    if effect is None:
        return (
            f"npc,{npc_id},{action},{sequence_text},,,,,"
            f"{variant},"
        )

    projectile = "true" if effect.placement == "projectile" else "false"

    projectile_suffix = ""
    if effect.placement == "projectile":
        # Preserved RuneScript generic NPC ranged helper:
        #   startheight=40 endheight=36 angle=15 offset=11
        # Preserved magic helper example:
        #   startheight=43 endheight=31 angle=16 offset=64
        start_height, end_height, slope, start_distance = (
            (43, 31, 16, 64)
            if action == "cast"
            else (40, 36, 15, 11)
        )

        # Troll throwers are explicitly recorded with their own projectile
        # geometry in the Burthorpe combat script.
        if effect.symbol == "troll_rock_travel":
            start_height, end_height, slope, start_distance = (36, 36, 8, 128)

        projectile_suffix = (
            f",{start_height},{end_height},{slope},{start_distance}"
        )

    return (
        f"npc,{npc_id},{action},{sequence_text},{effect.spotanim},"
        f"{projectile},{effect.delay_ms},{effect.duration_ms},"
        f"{variant},{effect.placement}{projectile_suffix}"
    )


def generate_rows(
    base_lines: list[str],
    mappings: list[Orchestration],
) -> tuple[list[str], list[dict[str, str]], int]:
    existing = parse_existing(base_lines)
    generated: list[str] = []
    provenance: list[dict[str, str]] = []
    rows_added = 0

    # Avoid repeated candidates from mirrored configs.
    seen_mappings: set[tuple] = set()

    for mapping in mappings:
        variant = choose_variant(existing, mapping)
        mapping_key = (
            mapping.npc_id,
            mapping.action,
            mapping.sequence,
            variant,
            tuple(
                (e.spotanim, e.placement, e.delay_ms, e.duration_ms)
                for e in mapping.effects
            ),
        )
        if mapping_key in seen_mappings:
            continue
        seen_mappings.add(mapping_key)

        sequence_key = (mapping.npc_id, mapping.action, variant)
        need_sequence = (
            mapping.sequence is not None and
            sequence_key not in existing.sequences
        )

        # Sequence-only variants matter (e.g. Jad melee).
        if not mapping.effects:
            if need_sequence:
                generated.append(
                    binding_line(
                        mapping.npc_id,
                        mapping.action,
                        mapping.sequence,
                        None,
                        variant,
                    )
                )
                existing.sequences[sequence_key] = mapping.sequence
                rows_added += 1
            continue

        sequence_written = not need_sequence

        for effect in mapping.effects:
            effect_key = (
                mapping.npc_id,
                mapping.action,
                variant,
                effect.spotanim,
                effect.placement,
            )
            if effect_key in existing.effects:
                continue

            sequence = (
                mapping.sequence
                if not sequence_written and mapping.sequence is not None
                else None
            )
            generated.append(
                binding_line(
                    mapping.npc_id,
                    mapping.action,
                    sequence,
                    effect,
                    variant,
                )
            )
            if sequence is not None:
                existing.sequences[sequence_key] = sequence
                sequence_written = True

            existing.effects.add(effect_key)
            existing.effect_variants[
                (mapping.npc_id, mapping.action, effect.spotanim)
            ] = variant
            rows_added += 1

            provenance.append({
                "npc_id": str(mapping.npc_id),
                "npc_symbol": mapping.npc_symbol,
                "action": mapping.action,
                "variant": variant,
                "sequence_id": (
                    "" if mapping.sequence is None else str(mapping.sequence)
                ),
                "sequence_symbol": mapping.sequence_symbol,
                "spotanim_id": str(effect.spotanim),
                "spotanim_symbol": effect.symbol,
                "placement": effect.placement,
                "source": mapping.source,
                "confidence": mapping.confidence,
                "evidence": mapping.evidence,
            })

        # A mapping may only need its sequence because every effect was already
        # present in a hand-authored binding.
        if not sequence_written and mapping.sequence is not None:
            generated.append(
                binding_line(
                    mapping.npc_id,
                    mapping.action,
                    mapping.sequence,
                    None,
                    variant,
                )
            )
            existing.sequences[sequence_key] = mapping.sequence
            rows_added += 1

    return generated, provenance, rows_added


def write_provenance(rows: list[dict[str, str]]) -> None:
    fieldnames = [
        "npc_id",
        "npc_symbol",
        "action",
        "variant",
        "sequence_id",
        "sequence_symbol",
        "spotanim_id",
        "spotanim_symbol",
        "placement",
        "confidence",
        "source",
        "evidence",
    ]
    PROVENANCE_PATH.parent.mkdir(parents=True, exist_ok=True)
    with PROVENANCE_PATH.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--bindings",
        type=Path,
        default=BINDINGS_PATH,
        help="animation bindings CSV to update",
    )
    args = parser.parse_args()

    if not args.bindings.exists():
        print(f"error: missing {args.bindings}", file=sys.stderr)
        return 2

    print("Fetching revision-289 symbolic packs...")
    npc_by_name, _ = parse_pack(fetch_text(PACK_URLS["npc"]))
    seq_by_name, _ = parse_pack(fetch_text(PACK_URLS["seq"]))
    spot_by_name, _ = parse_pack(fetch_text(PACK_URLS["spotanim"]))

    print("Mining NPC configs with explicit projectile metadata...")
    config_rows, unresolved = config_orchestrations(
        npc_by_name=npc_by_name,
        seq_by_name=seq_by_name,
        spot_by_name=spot_by_name,
    )

    original_lines = args.bindings.read_text(encoding="utf-8").splitlines()
    base_lines = strip_generated_block(original_lines)

    generated, provenance, rows_added = generate_rows(
        base_lines,
        config_rows + SERVER_VARIANTS,
    )

    output = list(base_lines)
    output.append("")
    output.append(BEGIN)
    output.append(
        "# Columns after duration_ms are optional: variant,placement,start_height,end_height,slope,start_distance."
    )
    output.append(
        "# placement: source | projectile | target. Projectile geometry uses classic RuneTek server values; existing rows remain valid."
    )
    output.extend(generated)
    output.append(END)
    output.append("")

    args.bindings.write_text(
        "\n".join(output),
        encoding="utf-8",
    )
    write_provenance(provenance)

    unique_npcs = len({
        mapping.npc_id
        for mapping in config_rows + SERVER_VARIANTS
    })
    print(
        f"Scanned {len(NPC_CONFIG_PATHS)} projectile-bearing config families; "
        f"resolved {len(config_rows)} config orchestrations."
    )
    print(
        f"Historical source set covers {unique_npcs} unique NPC ids before "
        f"deduplication against your existing bindings."
    )
    print(f"Added/refreshed {rows_added} generated binding rows.")
    print(f"Wrote provenance: {PROVENANCE_PATH}")

    if unresolved:
        print(f"Unresolved symbolic relationships: {len(unresolved)}")
        for item in unresolved[:20]:
            print(f"  - {item}")
        if len(unresolved) > 20:
            print(f"  ... {len(unresolved) - 20} more")
    else:
        print("Unresolved symbolic relationships: 0")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
