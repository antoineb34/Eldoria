#!/usr/bin/env bash
(
set -euo pipefail

cd "${HOME}/projects/Eldoria" || exit 1

if [[ "$(git branch --show-current)" != "feature/asset-manager" ]]; then
    echo "Expected feature/asset-manager; no files changed."
    exit 1
fi

required=(
    src/apps/elforge/dump/AssetDumper.cpp
    src/apps/elforge/explorer/tree/CacheTreeBuilder.cpp
    src/apps/elforge/explorer/CacheExplorerSelection.cpp
    src/assets/sequence/SequenceLoader.h
    src/assets/sprite/SpriteLoader.h
)

for file in "${required[@]}"; do
    if [[ ! -f "$file" ]]; then
        echo "Missing: $file; no files changed."
        exit 1
    fi
done

python3 - <<'PY'
from pathlib import Path
import re
import shutil
import tempfile

updates = {}


def update(path_name, transform):
    path = Path(path_name)
    original = path.read_text()
    changed = transform(original)

    if changed != original:
        updates[path] = changed


def repair_asset_dumper(text):
    # Undo the earlier global replacement on every non-sequence asset.
    text = text.replace(
        "value.data.idleAnimationId",
        "value.idleAnimationId",
    )
    text = text.replace(
        "value.data.id",
        "value.id",
    )

    start = text.find(
        "        if (state.activeSequence.has_value()) {"
    )
    end = text.find(
        "        if (state.activeItem.has_value()) {",
        start,
    )

    if start == -1 or end == -1:
        raise SystemExit(
            "Could not locate the sequence section in AssetDumper; "
            "no files changed."
        )

    section = text[start:end]
    section = re.sub(
        r"\bvalue\.(?!data\.)",
        "value.data.",
        section,
    )

    return text[:start] + section + text[end:]


def repair_cache_tree(text):
    text = text.replace(
        "const eld::sequence::SequenceResource& definition",
        "const eld::sequence::SequenceData& definition",
    )

    resource_load = re.compile(
        r"const eld::sequence::SequenceResource\s+definition\s*=\s*"
        r"repository\.get\(id\);"
    )

    text, load_count = resource_load.subn(
        "const eld::sequence::SequenceData& definition =\n"
        "            repository.data(id);",
        text,
    )

    if load_count > 1:
        raise SystemExit(
            "Unexpected duplicate sequence tree loaders; no files changed."
        )

    sprite_load = re.compile(
        r"spriteLoader\.emplace\(\s*"
        r"cache,\s*"
        r"entry\.fileId\s*"
        r"\);"
    )

    text, sprite_count = sprite_load.subn(
        "spriteLoader.emplace(\n"
        "            cache,\n"
        "            static_cast<eld::sprite::SpriteArchive>(\n"
        "                entry.fileId\n"
        "            )\n"
        "        );",
        text,
    )

    if sprite_count > 1:
        raise SystemExit(
            "Unexpected duplicate sprite loader construction; "
            "no files changed."
        )

    return text


def repair_inspector_call(text):
    return re.sub(
        r"^\s*animationFrameTable_,\s*\n",
        "",
        text,
        flags=re.MULTILINE,
    )


update(
    "src/apps/elforge/dump/AssetDumper.cpp",
    repair_asset_dumper,
)
update(
    "src/apps/elforge/explorer/tree/CacheTreeBuilder.cpp",
    repair_cache_tree,
)

for name in (
    "src/apps/elforge/explorer/CacheExplorerSelection.cpp",
    "src/apps/elforge/explorer/CacheExplorerAnimation.cpp",
    "src/apps/elforge/explorer/CacheExplorer.cpp",
):
    path = Path(name)
    if path.exists():
        update(name, repair_inspector_call)

if not updates:
    print("Post-loader repairs are already applied.")
else:
    backup = Path(
        tempfile.mkdtemp(
            prefix="eldoria-post-loader-repair-backup.",
            dir=Path.cwd(),
        )
    )

    for path in updates:
        saved = backup / path
        saved.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, saved)

    print(f"Backup: {backup}", flush=True)

    for path, text in updates.items():
        path.write_text(text)
        print(f"Updated: {path}")
PY

echo
echo "===== TARGETED CHECK ====="

if rg -n \
    'value\.data\.idleAnimationId|SequenceResource.*definition|animationFrameTable_,' \
    src/apps/elforge/dump/AssetDumper.cpp \
    src/apps/elforge/explorer/tree/CacheTreeBuilder.cpp \
    src/apps/elforge/explorer/CacheExplorerSelection.cpp \
    src/apps/elforge/explorer/CacheExplorerAnimation.cpp \
    src/apps/elforge/explorer/CacheExplorer.cpp; then
    echo "A stale reference remains."
    exit 1
else
    echo "Clean"
fi

echo
echo "===== BUILD ====="
cmake --build build -j 4

echo
echo "===== RESULT ====="
git status --short
git --no-pager diff --stat
)
