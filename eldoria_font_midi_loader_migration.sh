#!/usr/bin/env bash
set -euo pipefail

project_root="${1:-$PWD}"
cd "$project_root"

if [[ ! -f src/assets/CMakeLists.txt || ! -f src/assets/AssetManager.h ]]; then
    printf 'Run this script from the Eldoria project root.\n' >&2
    exit 1
fi

for name in Font Midi; do
    for file in \
        "src/assets/$name.h" \
        "src/assets/decoders/${name}Decoder.h" \
        "src/assets/decoders/${name}Decoder.cpp" \
        "src/assets/repositories/${name}Repository.h" \
        "src/assets/repositories/${name}Repository.cpp"
    do
        if [[ ! -f "$file" ]]; then
            printf 'Missing: %s\n' "$file" >&2
            exit 1
        fi
    done
done

backup_dir="$(mktemp -d "$project_root/eldoria-font-midi-backup.XXXXXXXX")"
mkdir -p "$backup_dir/src"
cp -a src/assets "$backup_dir/src/assets"

if [[ -d src/apps ]]; then
    cp -a src/apps "$backup_dir/src/apps"
fi

if [[ -d src/runtime ]]; then
    cp -a src/runtime "$backup_dir/src/runtime"
fi

printf 'Backup: %s\n' "$backup_dir"

python3 - <<'PY'
from pathlib import Path
import re

root = Path.cwd()
assets = root / "src/assets"
domains = [
    ("Font", "font", "font"),
    ("Midi", "midi", "midi"),
]


def move(source: Path, target: Path) -> None:
    target.parent.mkdir(parents=True, exist_ok=True)

    if target.exists():
        raise SystemExit(f"Target already exists: {target.relative_to(root)}")

    source.rename(target)
    print(f"Moved: {source.relative_to(root)} -> {target.relative_to(root)}")


for name, domain, namespace in domains:
    move(assets / f"{name}.h", assets / domain / f"{name}Data.h")
    move(
        assets / "decoders" / f"{name}Decoder.h",
        assets / domain / f"{name}Decoder.h",
    )
    move(
        assets / "decoders" / f"{name}Decoder.cpp",
        assets / domain / f"{name}Decoder.cpp",
    )
    move(
        assets / "repositories" / f"{name}Repository.h",
        assets / domain / f"{name}Loader.h",
    )
    move(
        assets / "repositories" / f"{name}Repository.cpp",
        assets / domain / f"{name}Loader.cpp",
    )


source_files = [
    path
    for path in (root / "src").rglob("*")
    if path.is_file()
    and (
        path.suffix in {".h", ".hpp", ".cpp", ".cc", ".cxx"}
        or path.name == "CMakeLists.txt"
    )
]


for path in source_files:
    text = path.read_text()
    updated = text

    for name, domain, namespace in domains:
        lower = name.lower()

        replacements = {
            f'"{name}.h"': f'"{domain}/{name}Data.h"',
            f'"decoders/{name}Decoder.h"': f'"{domain}/{name}Decoder.h"',
            f'"repositories/{name}Repository.h"': f'"{domain}/{name}Loader.h"',
            f"{name}Repository": f"{name}Loader",
            f"{lower}Repository": f"{lower}Loader",
        }

        for old, new in replacements.items():
            updated = updated.replace(old, new)

        updated = re.sub(
            rf"\beld::{namespace}::{name}\b",
            f"eld::{namespace}::{name}Data",
            updated,
        )

        updated = re.sub(
            rf"(namespace eld::{namespace} \{{\s*)struct {name};",
            rf"\1struct {name}Data;",
            updated,
        )

    if updated != text:
        path.write_text(updated)


for name, domain, namespace in domains:
    directory = assets / domain
    paths = [
        directory / f"{name}Data.h",
        directory / f"{name}Decoder.h",
        directory / f"{name}Decoder.cpp",
        directory / f"{name}Loader.h",
        directory / f"{name}Loader.cpp",
    ]

    for path in paths:
        text = path.read_text()
        text = re.sub(rf"\b{name}Repository\b", f"{name}Loader", text)
        text = re.sub(rf"\b{name}\b", f"{name}Data", text)
        text = text.replace(
            f'"{name}Data.h"',
            f'"{domain}/{name}Data.h"',
        )
        text = text.replace(
            f'"decoders/{name}Decoder.h"',
            f'"{domain}/{name}Decoder.h"',
        )
        text = text.replace(
            f'"repositories/{name}Loader.h"',
            f'"{domain}/{name}Loader.h"',
        )

        quoted = re.compile(r'"(?:\\.|[^"\\])*"')
        restored_lines = []

        for line in text.splitlines(keepends=True):
            if not line.lstrip().startswith("#include"):
                line = quoted.sub(
                    lambda match: match.group(0).replace(
                        f"{name}Data",
                        name,
                    ),
                    line,
                )

            restored_lines.append(line)

        text = "".join(restored_lines)
        path.write_text(text)

    header_path = directory / f"{name}Loader.h"
    header = header_path.read_text()

    if "#include <unordered_map>" not in header:
        include_position = header.find("\n\n#include \"")

        if include_position < 0:
            raise SystemExit(
                f"Could not place unordered_map include in {header_path.relative_to(root)}"
            )

        header = (
            header[:include_position]
            + "\n#include <unordered_map>"
            + header[include_position:]
        )

    get_declaration = re.compile(
        rf"\s*{name}Data\s+get\(\s*"
        rf"std::uint16_t\s+id\s*\)\s*const\s*;\s*"
    )
    header, get_count = get_declaration.subn(
        f'''\n\n    const {name}Data& data(
        std::uint16_t id
    ) const;\n''',
        header,
        count=1,
    )

    if get_count != 1:
        raise SystemExit(
            f"Expected numeric get declaration in {header_path.relative_to(root)}"
        )

    header = header.replace(
        "private:\n",
        f'''private:
    {name}Data loadData(
        std::uint16_t id
    ) const;

''',
        1,
    )

    class_end = "};\n\n}"

    if class_end not in header:
        raise SystemExit(f"Could not find class end in {header_path.relative_to(root)}")

    header = header.replace(
        class_end,
        f'''    mutable std::unordered_map<
        std::uint16_t,
        {name}Data
    > dataCache_;
}};

}}''',
        1,
    )
    header_path.write_text(header)

    source_path = directory / f"{name}Loader.cpp"
    source = source_path.read_text()
    source, implementation_count = re.subn(
        rf"\b{name}Data\s+{name}Loader::get\(",
        f"{name}Data {name}Loader::loadData(",
        source,
        count=1,
    )

    if implementation_count != 1:
        raise SystemExit(
            f"Could not convert get implementation in {source_path.relative_to(root)}"
        )

    source = re.sub(
        r"return\s+get\(id\);",
        "return data(id);",
        source,
    )

    data_method = f'''

const {name}Data& {name}Loader::data(
    std::uint16_t id
) const {{
    const auto cached = dataCache_.find(id);

    if (cached != dataCache_.end()) {{
        return cached->second;
    }}

    const auto entry = dataCache_.emplace(
        id,
        loadData(id)
    );

    return entry.first->second;
}}
'''

    namespace_end = "\n}\n"
    position = source.rfind(namespace_end)

    if position < 0:
        raise SystemExit(
            f"Could not find namespace end in {source_path.relative_to(root)}"
        )

    source = source[:position] + data_method + source[position:]
    source_path.write_text(source)


for path in source_files:
    if not path.exists() or path.name == "CMakeLists.txt":
        continue

    text = path.read_text()
    updated = text

    for identifier in (
        "fontLoader",
        "fontLoader_",
        "titleFontLoader",
        "titleFontLoader_",
        "midiLoader",
        "midiLoader_",
    ):
        updated = updated.replace(
            f"{identifier}.get(",
            f"{identifier}.data(",
        )
        updated = updated.replace(
            f"{identifier}->get(",
            f"{identifier}->data(",
        )

    matches = list(re.finditer(r"\brepository(?:_)?(?:\.|->)get\(", updated))

    for match in reversed(matches):
        context = updated[max(0, match.start() - 1600):match.start()]

        if "FontLoader" in context or "MidiLoader" in context:
            call = match.group(0).replace("get(", "data(")
            updated = updated[:match.start()] + call + updated[match.end():]

    if updated != text:
        path.write_text(updated)


cmake = assets / "CMakeLists.txt"
text = cmake.read_text()

for name, domain, namespace in domains:
    old_decoder = f"    decoders/{name}Decoder.cpp"
    old_loader = f"    repositories/{name}Loader.cpp"

    if old_decoder not in text or old_loader not in text:
        raise SystemExit(f"Missing old CMake entries for {name}")

    text = text.replace(
        old_decoder,
        f"    {domain}/{name}Decoder.cpp\n    {domain}/{name}Loader.cpp",
        1,
    )
    text = text.replace(old_loader + "\n", "", 1)

cmake.write_text(text)

print("Font and MIDI loaders migrated.")
PY

if command -v clang-format >/dev/null 2>&1; then
    find src \
        -type f \
        \( -name '*.h' -o -name '*.cpp' \) \
        -newer "$backup_dir" \
        -print0 |
        xargs -0 -r clang-format -i
fi

printf '\n===== ARCHITECTURE CHECK =====\n'

if rg -n \
    'FontRepository|MidiRepository|repositories/(Font|Midi)Repository|decoders/(Font|Midi)Decoder' \
    src \
    --glob '*.h' \
    --glob '*.cpp' \
    --glob 'CMakeLists.txt'
then
    printf 'Old Font/MIDI architecture references remain.\n' >&2
    exit 1
fi

if rg -n \
    '(fontLoader|titleFontLoader|midiLoader)(_|)(\.|->)get\(' \
    src \
    --glob '*.h' \
    --glob '*.cpp'
then
    printf 'Stale Font/MIDI loader get calls remain.\n' >&2
    exit 1
fi

printf '\n===== MAP UNTOUCHED =====\n'
rg -n \
    'MapRepository|repositories/MapRepository|decoders/(MapIndex|Terrain|LocationSpawn)Decoder' \
    src/assets \
    --glob '*.h' \
    --glob '*.cpp' \
    --glob 'CMakeLists.txt' || true

printf '\n===== CONFIGURE =====\n'
cmake -S . -B build

printf '\n===== BUILD =====\n'
cmake --build build -j 4

printf '\n===== RESULT =====\n'
git status --short
git --no-pager diff --stat
