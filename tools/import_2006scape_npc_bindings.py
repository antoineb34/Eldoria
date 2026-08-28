#!/usr/bin/env python3
"""Bulk-import NPC combat animation presentation bindings.

The hand-authored rows already present in content/animation_bindings.csv remain
canonical. This tool only fills missing (npc id, action) keys from the public
2006Scape NPC definition dataset.

It is intentionally presentation metadata: nothing here changes npc.dat/cache
ownership or the animation runtime.
"""

from __future__ import annotations

import argparse
import json
import sys
import urllib.request
from pathlib import Path
from typing import Any, Iterable

SOURCE_URL = (
    "https://raw.githubusercontent.com/2006-Scape/2006Scape/master/"
    "2006Scape%20Server/data/cfg/npcDefinitions.json"
)

BEGIN_MARKER = "# BEGIN AUTO-IMPORTED 2006SCAPE NPC DEFINITIONS"
END_MARKER = "# END AUTO-IMPORTED 2006SCAPE NPC DEFINITIONS"

# NPCDefinition.java uses these as its generic constructor defaults. They are
# still imported by default because the goal of this tool is maximum coverage,
# but the summary reports how many generated NPCs use the generic triplet.
GENERIC_DEFAULT_TRIPLET = (422, 404, 2304)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Fill missing NPC attack/defend/death presentation bindings from "
            "the 2006Scape NPC definition dataset."
        )
    )
    parser.add_argument(
        "--bindings",
        type=Path,
        default=Path("content/animation_bindings.csv"),
        help="Animation binding CSV to update (default: content/animation_bindings.csv).",
    )
    source = parser.add_mutually_exclusive_group()
    source.add_argument(
        "--source-file",
        type=Path,
        help="Use an already-downloaded npcDefinitions.json instead of the network.",
    )
    source.add_argument(
        "--source-url",
        default=SOURCE_URL,
        help="Override the npcDefinitions.json URL.",
    )
    parser.add_argument(
        "--skip-generic-defaults",
        action="store_true",
        help=(
            "Do not import NPCs whose exact combat animation triplet is the "
            "generic 422/404/2304 fallback. Existing authored rows are never removed."
        ),
    )
    parser.add_argument(
        "--max-sequence",
        type=int,
        default=65535,
        help=(
            "Ignore imported sequence IDs above this value. Useful when testing "
            "against a smaller cache revision (default: 65535)."
        ),
    )
    return parser.parse_args()


def load_source(args: argparse.Namespace) -> list[dict[str, Any]]:
    if args.source_file is not None:
        raw = args.source_file.read_text(encoding="utf-8")
    else:
        request = urllib.request.Request(
            args.source_url,
            headers={"User-Agent": "Eldoria-animation-binding-importer/1"},
        )
        with urllib.request.urlopen(request, timeout=30) as response:
            raw = response.read().decode("utf-8")

    data = json.loads(raw)
    if not isinstance(data, list):
        raise ValueError("Expected npcDefinitions.json to contain a top-level JSON array")
    return data


def remove_generated_block(lines: list[str]) -> list[str]:
    cleaned: list[str] = []
    inside = False

    for line in lines:
        stripped = line.strip()
        if stripped == BEGIN_MARKER:
            inside = True
            continue
        if stripped == END_MARKER:
            inside = False
            continue
        if not inside:
            cleaned.append(line)

    if inside:
        raise ValueError(f"Found {BEGIN_MARKER!r} without matching end marker")

    while cleaned and not cleaned[-1].strip():
        cleaned.pop()
    return cleaned


def existing_action_keys(lines: Iterable[str]) -> set[tuple[str, int, str]]:
    keys: set[tuple[str, int, str]] = set()

    for raw_line in lines:
        line = raw_line.strip()
        if not line or line.startswith("#") or line.startswith("kind,"):
            continue

        fields = [field.strip() for field in line.split(",")]
        if len(fields) < 3:
            continue

        kind, entity_text, action = fields[:3]
        if kind not in {"npc", "item"}:
            continue

        try:
            entity_id = int(entity_text)
        except ValueError:
            continue

        keys.add((kind, entity_id, action))

    return keys


def valid_sequence(value: Any, max_sequence: int) -> int | None:
    if isinstance(value, bool):
        return None
    try:
        sequence = int(value)
    except (TypeError, ValueError):
        return None

    if sequence <= 0 or sequence > max_sequence or sequence > 65535:
        return None
    return sequence


def build_rows(
    definitions: list[dict[str, Any]],
    occupied: set[tuple[str, int, str]],
    *,
    skip_generic_defaults: bool,
    max_sequence: int,
) -> tuple[list[str], dict[str, int]]:
    rows: list[str] = []
    stats = {
        "source_entries": 0,
        "attackable_entries": 0,
        "generic_triplet_entries": 0,
        "skipped_generic_entries": 0,
        "generated_rows": 0,
        "generated_npcs": 0,
        "skipped_existing_rows": 0,
        "skipped_invalid_rows": 0,
    }

    generated_npc_ids: set[int] = set()

    for definition in definitions:
        if not isinstance(definition, dict):
            continue
        stats["source_entries"] += 1

        if definition.get("attackable") is not True:
            continue
        stats["attackable_entries"] += 1

        try:
            npc_id = int(definition.get("id"))
        except (TypeError, ValueError):
            continue

        if npc_id < 0 or npc_id > 65535:
            continue

        raw_triplet = (
            definition.get("attackAnim"),
            definition.get("defenceAnim"),
            definition.get("deathAnim"),
        )
        try:
            triplet = tuple(int(value) for value in raw_triplet)
        except (TypeError, ValueError):
            triplet = raw_triplet

        if triplet == GENERIC_DEFAULT_TRIPLET:
            stats["generic_triplet_entries"] += 1
            if skip_generic_defaults:
                stats["skipped_generic_entries"] += 1
                continue

        actions = (
            ("attack", definition.get("attackAnim")),
            ("defend", definition.get("defenceAnim")),
            ("death", definition.get("deathAnim")),
        )

        for action, raw_sequence in actions:
            key = ("npc", npc_id, action)
            if key in occupied:
                stats["skipped_existing_rows"] += 1
                continue

            sequence = valid_sequence(raw_sequence, max_sequence)
            if sequence is None:
                stats["skipped_invalid_rows"] += 1
                continue

            rows.append(f"npc,{npc_id},{action},{sequence},,,,")
            occupied.add(key)
            generated_npc_ids.add(npc_id)
            stats["generated_rows"] += 1

    stats["generated_npcs"] = len(generated_npc_ids)
    return rows, stats


def main() -> int:
    args = parse_args()

    if args.max_sequence <= 0 or args.max_sequence > 65535:
        print("--max-sequence must be in the range 1..65535", file=sys.stderr)
        return 2

    bindings_path = args.bindings
    if not bindings_path.is_file():
        print(f"Bindings file not found: {bindings_path}", file=sys.stderr)
        return 2

    try:
        definitions = load_source(args)
        original_lines = bindings_path.read_text(encoding="utf-8").splitlines()
        base_lines = remove_generated_block(original_lines)
        occupied = existing_action_keys(base_lines)
        generated_rows, stats = build_rows(
            definitions,
            occupied,
            skip_generic_defaults=args.skip_generic_defaults,
            max_sequence=args.max_sequence,
        )
    except Exception as error:  # command-line utility: concise failure is preferable
        print(f"Import failed: {error}", file=sys.stderr)
        return 1

    generated_block = [
        "",
        BEGIN_MARKER,
        "# Source: 2006-Scape/2006Scape data/cfg/npcDefinitions.json",
        "# Existing authored NPC/action keys above this block always win.",
        "# This broad source is lower-confidence than the hand-authored RuneScript/projectile rows.",
        f"# Imported attackable NPCs with max_sequence={args.max_sequence}.",
        *generated_rows,
        END_MARKER,
    ]

    output = "\n".join([*base_lines, *generated_block]) + "\n"
    bindings_path.write_text(output, encoding="utf-8")

    print(f"Source definitions:      {stats['source_entries']}")
    print(f"Attackable definitions:  {stats['attackable_entries']}")
    print(f"Generated NPCs:          {stats['generated_npcs']}")
    print(f"Generated rows:          {stats['generated_rows']}")
    print(f"Existing rows preserved: {stats['skipped_existing_rows']}")
    print(f"Invalid/zero skipped:    {stats['skipped_invalid_rows']}")
    print(f"Generic 422/404/2304:    {stats['generic_triplet_entries']}")
    if args.skip_generic_defaults:
        print(f"Generic NPCs skipped:    {stats['skipped_generic_entries']}")
    print(f"Updated: {bindings_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
