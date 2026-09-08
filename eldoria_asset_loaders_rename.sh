#!/usr/bin/env bash
(
set -euo pipefail

cd "${HOME}/projects/Eldoria" || exit 1

if [[ "$(git branch --show-current)" != "feature/asset-manager" ]]; then
    echo "Expected feature/asset-manager; no files changed."
    exit 1
fi

python3 - <<'PY'
from pathlib import Path
import re
import shutil
import tempfile

renames = {
    "src/assets/animation/AnimationPipeline.h":
        "src/assets/animation/AnimationLoader.h",
    "src/assets/animation/AnimationPipeline.cpp":
        "src/assets/animation/AnimationLoader.cpp",
    "src/assets/model/ModelPipeline.h":
        "src/assets/model/ModelLoader.h",
    "src/assets/model/ModelPipeline.cpp":
        "src/assets/model/ModelLoader.cpp",
    "src/assets/sequence/SequencePipeline.h":
        "src/assets/sequence/SequenceLoader.h",
    "src/assets/sequence/SequencePipeline.cpp":
        "src/assets/sequence/SequenceLoader.cpp",
    "src/assets/sprite/SpritePipeline.h":
        "src/assets/sprite/SpriteLoader.h",
    "src/assets/sprite/SpritePipeline.cpp":
        "src/assets/sprite/SpriteLoader.cpp",
    "src/assets/texture/TexturePipeline.h":
        "src/assets/texture/TextureLoader.h",
    "src/assets/texture/TexturePipeline.cpp":
        "src/assets/texture/TextureLoader.cpp",
    "src/assets/title/TitlePipeline.h":
        "src/assets/title/TitleLoader.h",
    "src/assets/title/TitlePipeline.cpp":
        "src/assets/title/TitleLoader.cpp",
}

replacements = (
    ("AnimationPipeline", "AnimationLoader"),
    ("animationPipeline", "animationLoader"),
    ("ModelPipeline", "ModelLoader"),
    ("modelPipeline", "modelLoader"),
    ("SequencePipeline", "SequenceLoader"),
    ("sequencePipeline", "sequenceLoader"),
    ("SpritePipeline", "SpriteLoader"),
    ("spritePipeline", "spriteLoader"),
    ("TexturePipeline", "TextureLoader"),
    ("texturePipeline", "textureLoader"),
    ("TitlePipeline", "TitleLoader"),
    ("titlePipeline", "titleLoader"),
)

pending_renames = {}
missing = []
for source_name, destination_name in renames.items():
    source = Path(source_name)
    destination = Path(destination_name)

    if source.exists() and destination.exists():
        raise SystemExit(
            f"Both names exist: {source} and {destination}; no files changed."
        )

    if source.exists():
        pending_renames[source_name] = destination_name
    elif not destination.exists():
        missing.append(source_name)

if missing:
    raise SystemExit(
        "Expected asset pipeline files are missing:\n" +
        "\n".join(missing) +
        "\nNo files changed."
    )

text_files = []
for root_name in ("src", "tests"):
    root = Path(root_name)
    if not root.exists():
        continue

    for path in root.rglob("*"):
        if not path.is_file() or path.is_symlink():
            continue

        if path.suffix in {".h", ".hpp", ".cpp", ".cc", ".cxx"}:
            text_files.append(path)
        elif path.name == "CMakeLists.txt":
            text_files.append(path)

updates = {}
for path in text_files:
    text = path.read_text()
    updated = text

    for old, new in replacements:
        updated = updated.replace(old, new)

    if path.as_posix() == "src/apps/elforge/dump/AssetDumper.cpp":
        start = updated.find(
            "        if (state.activeSequence.has_value()) {"
        )
        end = updated.find(
            "        if (state.activeItem.has_value()) {",
            start,
        )

        if start != -1 and end != -1:
            section = updated[start:end]
            section = re.sub(
                r"\bvalue\.(?!data\.)",
                "value.data.",
                section,
            )
            updated = updated[:start] + section + updated[end:]

    if updated != text:
        updates[path] = updated

if not updates and not pending_renames:
    print("Asset loaders are already renamed.")
else:
    backup = Path(
        tempfile.mkdtemp(
            prefix="eldoria-asset-loaders-backup.",
            dir=Path.cwd(),
        )
    )

    for path in updates:
        destination = backup / path
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, destination)

    print(f"Backup: {backup}", flush=True)

    for source_name, destination_name in pending_renames.items():
        source = Path(source_name)
        destination = Path(destination_name)
        source.rename(destination)
        print(f"Renamed: {source} -> {destination}")

    for original_path, text in updates.items():
        path = Path(
            pending_renames.get(
                original_path.as_posix(),
                original_path.as_posix(),
            )
        )
        path.write_text(text)

    print(f"Updated {len(updates)} source files.")
PY

echo
echo "===== OLD ASSET PIPELINE REFERENCES ====="

if rg -n \
    'AnimationPipeline|ModelPipeline|SequencePipeline|SpritePipeline|TexturePipeline|TitlePipeline' \
    src tests 2>/dev/null; then
    echo "Old asset pipeline references remain."
    exit 1
else
    echo "None"
fi

echo
echo "===== RENDER PIPELINE CHECK ====="
rg -n 'RenderPipeline' src/runtime src/apps || true

echo
echo "===== CONFIGURE ====="
cmake -S . -B build

echo
echo "===== BUILD ====="
cmake --build build -j 4

echo
echo "===== RESULT ====="
git status --short
git --no-pager diff --stat
)
