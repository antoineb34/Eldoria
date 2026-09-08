#!/usr/bin/env bash
set -euo pipefail

project_root="${1:-$PWD}"
cd "$project_root"

if [[ ! -f src/assets/CMakeLists.txt ]]; then
    printf 'Run this script from the Eldoria project root.\n' >&2
    exit 1
fi

backup_dir="$(mktemp -d "$project_root/eldoria-identity-assembler-backup.XXXXXXXX")"
mkdir -p "$backup_dir/src"
cp -a src/assets "$backup_dir/src/assets"

if [[ -d src/apps ]]; then
    cp -a src/apps "$backup_dir/src/apps"
fi

printf 'Backup: %s\n' "$backup_dir"

python3 - <<'PY'
from pathlib import Path
import re

root = Path.cwd()
assets = root / "src/assets"

definitions = [
    ("Floor", "floor"),
    ("IdentityKit", "identity_kit"),
    ("Item", "item"),
    ("Location", "location"),
    ("MessageAnimation", "message_animation"),
    ("Message", "message"),
    ("Npc", "npc"),
    ("Parameter", "parameter"),
    ("Varbit", "varbit"),
    ("Varp", "varp"),
]


def remove_function(text: str, marker: str) -> str:
    marker_at = text.find(marker)

    if marker_at < 0:
        raise SystemExit(f"Missing function: {marker}")

    start = text.rfind("\n\n", 0, marker_at)
    start = 0 if start < 0 else start + 2

    brace = text.find("{", marker_at)
    if brace < 0:
        raise SystemExit(f"Missing function body: {marker}")

    depth = 0
    end = None

    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1

            if depth == 0:
                end = index + 1
                break

    if end is None:
        raise SystemExit(f"Unbalanced function body: {marker}")

    while end < len(text) and text[end] in " \t\r\n":
        end += 1

    return text[:start] + text[end:]


for name, domain in definitions:
    directory = assets / domain
    loader_header = directory / f"{name}Loader.h"
    loader_source = directory / f"{name}Loader.cpp"
    assembler_header = directory / f"{name}Assembler.h"
    assembler_source = directory / f"{name}Assembler.cpp"
    resource_header = directory / f"{name}Resource.h"

    for path in (
        loader_header,
        loader_source,
        assembler_header,
        assembler_source,
        resource_header,
    ):
        if not path.is_file():
            raise SystemExit(f"Missing: {path.relative_to(root)}")

    header = loader_header.read_text()

    for include in (
        f'#include "{domain}/{name}Assembler.h"\n',
        f'#include "{domain}/{name}Resource.h"\n',
    ):
        if include not in header:
            raise SystemExit(
                f"Missing generated include in {loader_header.relative_to(root)}: "
                f"{include.strip()}"
            )
        header = header.replace(include, "", 1)

    resource_method = re.compile(
        rf"\s*const\s+{name}Resource\s*&\s*resource\s*"
        rf"\(\s*std::uint16_t\s+id\s*\)\s*const\s*;\s*"
    )
    header, method_count = resource_method.subn("\n\n", header, count=1)

    if method_count != 1:
        raise SystemExit(
            f"Could not remove resource declaration from "
            f"{loader_header.relative_to(root)}"
        )

    assembler_member = re.compile(
        rf"\s*{name}Assembler\s+assembler_\s*;\s*"
    )
    header, assembler_count = assembler_member.subn("\n\n", header, count=1)

    if assembler_count != 1:
        raise SystemExit(
            f"Could not remove assembler member from "
            f"{loader_header.relative_to(root)}"
        )

    resource_cache = re.compile(
        rf"\s*mutable\s+std::unordered_map\s*<\s*"
        rf"std::uint16_t\s*,\s*{name}Resource\s*>\s*"
        rf"resourceCache_\s*;\s*"
    )
    header, cache_count = resource_cache.subn("\n", header, count=1)

    if cache_count != 1:
        raise SystemExit(
            f"Could not remove resource cache from "
            f"{loader_header.relative_to(root)}"
        )

    loader_header.write_text(header)

    source = loader_source.read_text()
    source = remove_function(
        source,
        f"{name}Loader::resource(",
    )
    loader_source.write_text(source)

    assembler_header.unlink()
    assembler_source.unlink()
    resource_header.unlink()


cmake = assets / "CMakeLists.txt"
text = cmake.read_text()

for name, domain in definitions:
    entry = f"    {domain}/{name}Assembler.cpp\n"

    if entry not in text:
        raise SystemExit(f"Missing CMake entry: {entry.strip()}")

    text = text.replace(entry, "", 1)

cmake.write_text(text)


repairs = {
    "npcs_->get(": "npcs_->data(",
    "locations_->get(": "locations_->data(",
    "items_->get(": "items_->data(",
}

for path in (root / "src").rglob("*"):
    if not path.is_file() or path.suffix not in {
        ".h", ".hpp", ".cpp", ".cc", ".cxx"
    }:
        continue

    source = path.read_text()
    updated = source

    for old, new in repairs.items():
        updated = updated.replace(old, new)

    if path.name == "CacheTreeBuilder.cpp":
        updated = updated.replace(
            "repository.get(id)",
            "repository.data(id)",
        )

    if updated != source:
        path.write_text(updated)
        print(f"Updated: {path.relative_to(root)}")

print("Removed identity-only assemblers and resources.")
PY

if command -v clang-format >/dev/null 2>&1; then
    for domain in \
        floor \
        identity_kit \
        item \
        location \
        message_animation \
        message \
        npc \
        parameter \
        varbit \
        varp
    do
        clang-format -i \
            "src/assets/$domain/"*Loader.h \
            "src/assets/$domain/"*Loader.cpp
    done

    clang-format -i \
        src/apps/elforge/explorer/tree/CacheTreeBuilder.cpp \
        src/apps/elforge/inspection/AnimationInspector.cpp
fi

printf '\n===== ARCHITECTURE CHECK =====\n'

if rg -n \
    '(Floor|IdentityKit|Item|Location|MessageAnimation|Message|Npc|Parameter|Varbit|Varp)(Assembler|Resource)|resourceCache_' \
    src/assets/floor \
    src/assets/identity_kit \
    src/assets/item \
    src/assets/location \
    src/assets/message_animation \
    src/assets/message \
    src/assets/npc \
    src/assets/parameter \
    src/assets/varbit \
    src/assets/varp \
    src/assets/CMakeLists.txt
then
    printf 'Identity-only composition code remains.\n' >&2
    exit 1
fi

printf '\n===== STALE LOADER GET CALLS =====\n'

if rg -n \
    '(floorLoader|identityKitLoader|itemLoader|locationLoader|messageAnimationLoader|messageLoader|npcLoader|parameterLoader|varbitLoader|varpLoader|npcs_|locations_|items_)(\.|->)get\(' \
    src \
    --glob '*.h' \
    --glob '*.cpp'
then
    printf 'Stale definition-loader get calls remain.\n' >&2
    exit 1
fi

printf '\n===== CONFIGURE =====\n'
cmake -S . -B build

printf '\n===== BUILD =====\n'
cmake --build build -j 4

printf '\n===== RESULT =====\n'
git status --short
git --no-pager diff --stat

