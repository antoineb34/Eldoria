#!/usr/bin/env bash
set -euo pipefail

project_root="${1:-$PWD}"
cd "$project_root"

if [[ ! -f src/assets/CMakeLists.txt || ! -f src/assets/AssetManager.h ]]; then
    printf 'Run this script from the Eldoria project root.\n' >&2
    exit 1
fi

backup_dir="$(mktemp -d "$project_root/eldoria-definition-loaders-backup.XXXXXXXX")"
cp -a src/assets "$backup_dir/assets"

for app_dir in src/apps src/runtime; do
    if [[ -d "$app_dir" ]]; then
        mkdir -p "$backup_dir/$(dirname "$app_dir")"
        cp -a "$app_dir" "$backup_dir/$app_dir"
    fi
done

printf 'Backup: %s\n' "$backup_dir"

python3 - <<'PY'
from pathlib import Path
import re
import shutil

root = Path.cwd()
assets = root / "src/assets"

definitions = [
    ("Floor", "floor", "floor"),
    ("IdentityKit", "identity_kit", "identity_kit"),
    ("Item", "item", "item"),
    ("Location", "location", "location"),
    ("MessageAnimation", "message_animation", "message_animation"),
    ("Message", "message", "message"),
    ("Npc", "npc", "npc"),
    ("Parameter", "parameter", "parameter"),
    ("Varbit", "varbit", "varbit"),
    ("Varp", "varp", "varp"),
    ("Widget", "interface", "interface"),
]


def lower_camel(name: str) -> str:
    return name[0].lower() + name[1:]


def require(path: Path) -> None:
    if not path.is_file():
        raise SystemExit(f"Missing required file: {path.relative_to(root)}")


def move(source: Path, target: Path) -> None:
    require(source)
    target.parent.mkdir(parents=True, exist_ok=True)
    if target.exists():
        raise SystemExit(f"Target already exists: {target.relative_to(root)}")
    source.rename(target)
    print(f"Moved: {source.relative_to(root)} -> {target.relative_to(root)}")


for name, domain, namespace in definitions:
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
    and (path.suffix in {".h", ".hpp", ".cpp", ".cc", ".cxx"}
         or path.name == "CMakeLists.txt")
]


for path in source_files:
    text = path.read_text()
    updated = text

    for name, domain, namespace in definitions:
        variable = lower_camel(name)

        replacements = {
            f'"{name}.h"': f'"{domain}/{name}Data.h"',
            f'"decoders/{name}Decoder.h"':
                f'"{domain}/{name}Decoder.h"',
            f'"repositories/{name}Repository.h"':
                f'"{domain}/{name}Loader.h"',
            f"{name}Repository": f"{name}Loader",
            f"{variable}Repository": f"{variable}Loader",
        }

        for old, new in replacements.items():
            updated = updated.replace(old, new)

        updated = re.sub(
            rf"\beld::{re.escape(namespace)}::{name}\b",
            f"eld::{namespace}::{name}Data",
            updated,
        )

        updated = re.sub(
            rf"(namespace eld::{re.escape(namespace)} \{{\s*)struct {name};",
            rf"\1struct {name}Data;",
            updated,
        )

    if updated != text:
        path.write_text(updated)


for name, domain, namespace in definitions:
    directory = assets / domain
    data_header = directory / f"{name}Data.h"
    decoder_header = directory / f"{name}Decoder.h"
    decoder_source = directory / f"{name}Decoder.cpp"
    loader_header = directory / f"{name}Loader.h"
    loader_source = directory / f"{name}Loader.cpp"

    domain_files = [
        data_header,
        decoder_header,
        decoder_source,
        loader_header,
        loader_source,
    ]

    for path in domain_files:
        text = path.read_text()
        text = re.sub(rf"\b{name}Repository\b", f"{name}Loader", text)
        text = re.sub(rf"\b{name}\b", f"{name}Data", text)
        restored_lines = []
        for line in text.splitlines(keepends=True):
            if not line.lstrip().startswith("#include"):
                line = re.sub(
                    r'"(?:\\.|[^"\\])*"',
                    lambda match: match.group(0).replace(
                        f"{name}Data",
                        name,
                    ),
                    line,
                )
            restored_lines.append(line)
        text = "".join(restored_lines)
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
        path.write_text(text)

    resource_header = directory / f"{name}Resource.h"
    resource_header.write_text(
        f'''#pragma once

#include "{domain}/{name}Data.h"

namespace eld::{namespace} {{

struct {name}Resource {{
    {name}Data data;
}};

}}
'''
    )

    assembler_header = directory / f"{name}Assembler.h"
    assembler_header.write_text(
        f'''#pragma once

#include "{domain}/{name}Data.h"
#include "{domain}/{name}Resource.h"

namespace eld::{namespace} {{

class {name}Assembler {{
public:
    {name}Resource assemble(
        {name}Data data
    ) const;
}};

}}
'''
    )

    assembler_source = directory / f"{name}Assembler.cpp"
    assembler_source.write_text(
        f'''#include "{domain}/{name}Assembler.h"

#include <utility>

namespace eld::{namespace} {{

{name}Resource {name}Assembler::assemble(
    {name}Data data
) const {{
    return {name}Resource{{
        .data = std::move(data),
    }};
}}

}}
'''
    )

    header = loader_header.read_text()
    include_anchor = f'#include "{domain}/{name}Data.h"\n'
    if include_anchor not in header:
        raise SystemExit(
            f"Could not find data include in {loader_header.relative_to(root)}"
        )

    header = header.replace(
        include_anchor,
        include_anchor
        + f'#include "{domain}/{name}Assembler.h"\n'
        + f'#include "{domain}/{name}Resource.h"\n',
        1,
    )

    if "#include <unordered_map>" not in header:
        header = header.replace(
            "#include <vector>\n",
            "#include <vector>\n#include <unordered_map>\n",
            1,
        )

    old_get = re.compile(
        rf"    {name}Data get\(\s*std::uint16_t id\s*\) const;",
        re.MULTILINE,
    )
    header, changed = old_get.subn(
        f'''    const {name}Data& data(
        std::uint16_t id
    ) const;

    const {name}Resource& resource(
        std::uint16_t id
    ) const;''',
        header,
        count=1,
    )
    if changed != 1:
        raise SystemExit(
            f"Could not replace get declaration in {loader_header.relative_to(root)}"
        )

    private_anchor = "private:\n"
    if private_anchor not in header:
        raise SystemExit(
            f"Could not find private section in {loader_header.relative_to(root)}"
        )

    private_members = f'''private:
    {name}Data loadData(
        std::uint16_t id
    ) const;

'''
    header = header.replace(private_anchor, private_members, 1)

    final_class = "};\n\n}"
    cache_members = f'''    {name}Assembler assembler_;

    mutable std::unordered_map<
        std::uint16_t,
        {name}Data
    > dataCache_;

    mutable std::unordered_map<
        std::uint16_t,
        {name}Resource
    > resourceCache_;
}};

}}'''
    if final_class not in header:
        raise SystemExit(
            f"Could not find class end in {loader_header.relative_to(root)}"
        )
    header = header.replace(final_class, cache_members, 1)
    loader_header.write_text(header)

    source = loader_source.read_text()
    source = re.sub(
        rf"\b{name}Data {name}Loader::get\(",
        f"{name}Data {name}Loader::loadData(",
        source,
        count=1,
    )
    if f"{name}Loader::loadData(" not in source:
        raise SystemExit(
            f"Could not convert get implementation in {loader_source.relative_to(root)}"
        )

    source = re.sub(
        r"return get\(id\);",
        "return data(id);",
        source,
    )

    implementations = f'''

const {name}Data& {name}Loader::data(
    std::uint16_t id
) const {{
    const auto cached = dataCache_.find(id);

    if (cached != dataCache_.end()) {{
        return cached->second;
    }}

    const auto [entry, inserted] =
        dataCache_.emplace(
            id,
            loadData(id)
        );

    return entry->second;
}}


const {name}Resource& {name}Loader::resource(
    std::uint16_t id
) const {{
    const auto cached = resourceCache_.find(id);

    if (cached != resourceCache_.end()) {{
        return cached->second;
    }}

    const auto [entry, inserted] =
        resourceCache_.emplace(
            id,
            assembler_.assemble(data(id))
        );

    return entry->second;
}}
'''

    namespace_end = "\n}\n"
    position = source.rfind(namespace_end)
    if position < 0:
        raise SystemExit(
            f"Could not find namespace end in {loader_source.relative_to(root)}"
        )
    source = source[:position] + implementations + source[position:]
    loader_source.write_text(source)


for path in source_files:
    if not path.exists():
        continue

    text = path.read_text()
    updated = text

    for name, domain, namespace in definitions:
        variable = lower_camel(name) + "Loader"
        updated = re.sub(
            rf"\b{re.escape(variable)}(_?)\.get\(",
            rf"{variable}\1.data(",
            updated,
        )

    # The remaining generic variable name is used by definition-only helper
    # functions. Change it only when the nearby signature names one of the
    # migrated loader types.
    matches = list(re.finditer(r"\brepository\.get\(", updated))
    for match in reversed(matches):
        context = updated[max(0, match.start() - 1200):match.start()]
        if any(f"{name}Loader& repository" in context for name, _, _ in definitions):
            updated = (
                updated[:match.start()]
                + "repository.data("
                + updated[match.end():]
            )

    if updated != text:
        path.write_text(updated)


cmake = assets / "CMakeLists.txt"
text = cmake.read_text()

for name, domain, namespace in definitions:
    old_decoder = f"    decoders/{name}Decoder.cpp"
    # The earlier class-name rewrite also updates the basename inside this
    # still-old CMake path.
    old_repository = f"    repositories/{name}Loader.cpp"
    new_entries = (
        f"    {domain}/{name}Decoder.cpp\n"
        f"    {domain}/{name}Assembler.cpp\n"
        f"    {domain}/{name}Loader.cpp"
    )

    if old_decoder not in text or old_repository not in text:
        raise SystemExit(f"Missing old CMake entries for {name}")

    text = text.replace(old_decoder, new_entries, 1)
    text = text.replace(old_repository + "\n", "", 1)

cmake.write_text(text)


# C++ warnings are enabled on some configurations. Avoid unused structured
# binding fields while keeping the insertion result readable.
for _, domain, _ in definitions:
    path = next((assets / domain).glob("*Loader.cpp"))
    text = path.read_text().replace(
        "const auto [entry, inserted] =",
        "const auto entry =",
    )
    text = text.replace(
        "dataCache_.emplace(\n",
        "dataCache_.emplace(\n",
    )
    text = text.replace(
        "resourceCache_.emplace(\n",
        "resourceCache_.emplace(\n",
    )
    text = text.replace("\n\n    return entry->second;", "\n\n    return entry.first->second;")
    path.write_text(text)

print("Definition loader migration written.")
PY

printf '\n===== FORMAT =====\n'

if command -v clang-format >/dev/null 2>&1; then
    find src/assets \
        src/apps \
        src/runtime \
        -type f \
        \( -name '*.h' -o -name '*.cpp' \) \
        -newer "$backup_dir" \
        -print0 2>/dev/null |
        xargs -0 -r clang-format -i
fi

printf '\n===== ARCHITECTURE CHECK =====\n'

if rg -n \
    'FloorRepository|IdentityKitRepository|ItemRepository|LocationRepository|MessageAnimationRepository|MessageRepository|NpcRepository|ParameterRepository|VarbitRepository|VarpRepository|WidgetRepository|decoders/(Floor|IdentityKit|Item|Location|MessageAnimation|Message|Npc|Parameter|Varbit|Varp|Widget)Decoder|repositories/(Floor|IdentityKit|Item|Location|MessageAnimation|Message|Npc|Parameter|Varbit|Varp|Widget)Repository' \
    src \
    --glob '*.h' \
    --glob '*.cpp' \
    --glob 'CMakeLists.txt'
then
    printf 'Old definition architecture references remain.\n' >&2
    exit 1
fi

printf '\n===== CONFIGURE =====\n'
cmake -S . -B build

printf '\n===== BUILD =====\n'
cmake --build build -j 4

printf '\n===== RESULT =====\n'
git status --short
git --no-pager diff --stat

printf '\nAll remaining definition domains migrated successfully.\n'
