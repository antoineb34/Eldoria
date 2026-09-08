#!/usr/bin/env bash
set -euo pipefail

project_root="${1:-$PWD}"
cd "$project_root"

for file in \
    src/assets/interface/WidgetData.h \
    src/assets/interface/WidgetLoader.h \
    src/assets/interface/WidgetLoader.cpp \
    src/assets/interface/WidgetAssembler.h \
    src/assets/interface/WidgetAssembler.cpp \
    src/assets/interface/WidgetResource.h \
    src/assets/CMakeLists.txt
do
    if [[ ! -f "$file" ]]; then
        printf 'Missing: %s\n' "$file" >&2
        exit 1
    fi
done

backup_dir="$(mktemp -d "$project_root/eldoria-widget-assembler-backup.XXXXXXXX")"

mkdir -p "$backup_dir/src/assets/interface"
cp src/assets/interface/WidgetLoader.h \
   src/assets/interface/WidgetLoader.cpp \
   src/assets/interface/WidgetAssembler.h \
   src/assets/interface/WidgetAssembler.cpp \
   src/assets/interface/WidgetResource.h \
   "$backup_dir/src/assets/interface/"

cp src/assets/CMakeLists.txt "$backup_dir/src/assets/"

for file in \
    src/assets/InterfaceComposer.h \
    src/assets/InterfaceComposer.cpp
do
    if [[ -f "$file" ]]; then
        cp "$file" "$backup_dir/src/assets/"
    fi
done

printf 'Backup: %s\n' "$backup_dir"

python3 - <<'PY'
from pathlib import Path
import re

resource = Path("src/assets/interface/WidgetResource.h")
resource.write_text('''#pragma once

#include <cstdint>
#include <vector>

#include "interface/WidgetData.h"

namespace eld::interface {

struct WidgetNode {
    WidgetData data;

    std::int16_t x = 0;
    std::int16_t y = 0;

    std::vector<WidgetNode> children;
};


struct WidgetResource {
    std::uint16_t rootId = 0;
    WidgetNode root;
};

}
''')

header = Path("src/assets/interface/WidgetAssembler.h")
header.write_text('''#pragma once

#include <cstdint>
#include <unordered_set>

#include "interface/WidgetData.h"
#include "interface/WidgetResource.h"

namespace eld::interface {

class WidgetLoader;


class WidgetAssembler {
public:
    explicit WidgetAssembler(
        const WidgetLoader& widgets
    );

    WidgetResource assemble(
        std::uint16_t rootId
    ) const;

private:
    WidgetNode assembleNode(
        const WidgetData& data,
        std::int16_t x,
        std::int16_t y,
        std::unordered_set<std::uint16_t>& stack
    ) const;

    const WidgetLoader& widgets_;
};

}
''')

source = Path("src/assets/interface/WidgetAssembler.cpp")
source.write_text('''#include "interface/WidgetAssembler.h"

#include <stdexcept>

#include "interface/WidgetLoader.h"

namespace eld::interface {

WidgetAssembler::WidgetAssembler(
    const WidgetLoader& widgets
)
    : widgets_(widgets) {
}


WidgetResource WidgetAssembler::assemble(
    std::uint16_t rootId
) const {
    if (!widgets_.contains(rootId)) {
        throw std::out_of_range(
            "Interface root widget does not exist"
        );
    }

    std::unordered_set<std::uint16_t> stack;

    return WidgetResource{
        .rootId = rootId,
        .root = assembleNode(
            widgets_.data(rootId),
            0,
            0,
            stack
        ),
    };
}


WidgetNode WidgetAssembler::assembleNode(
    const WidgetData& data,
    std::int16_t x,
    std::int16_t y,
    std::unordered_set<std::uint16_t>& stack
) const {
    stack.insert(data.id);

    WidgetNode node{
        .data = data,
        .x = x,
        .y = y,
    };

    node.children.reserve(
        data.children.size()
    );

    for (const WidgetChild& child : data.children) {
        if (
            stack.contains(child.id) ||
            !widgets_.contains(child.id)
        ) {
            continue;
        }

        node.children.push_back(
            assembleNode(
                widgets_.data(child.id),
                child.x,
                child.y,
                stack
            )
        );
    }

    stack.erase(data.id);

    return node;
}

}
''')

loader = Path("src/assets/interface/WidgetLoader.cpp")
text = loader.read_text()

constructor = re.compile(
    r"(WidgetLoader::WidgetLoader\(\s*"
    r"const eld::cache::Cache\s*&\s*cache\s*\))\s*\{"
)

text, count = constructor.subn(
    r"\1\n    : assembler_(*this) {",
    text,
    count=1,
)

if count != 1 and ": assembler_(*this)" not in text:
    raise SystemExit("Could not initialize WidgetAssembler in WidgetLoader")

text, count = re.subn(
    r"assembler_\.assemble\(\s*data\(id\)\s*\)",
    "assembler_.assemble(id)",
    text,
    count=1,
)

if count != 1 and "assembler_.assemble(id)" not in text:
    raise SystemExit("Could not update WidgetLoader::resource")

loader.write_text(text)

cmake = Path("src/assets/CMakeLists.txt")
text = cmake.read_text()

cmake.write_text(
    text.replace(
        "    InterfaceComposer.cpp\n",
        "",
        1,
    )
)

Path("src/assets/InterfaceComposer.h").unlink(missing_ok=True)
Path("src/assets/InterfaceComposer.cpp").unlink(missing_ok=True)
PY

if command -v clang-format >/dev/null 2>&1; then
    clang-format -i \
        src/assets/interface/WidgetResource.h \
        src/assets/interface/WidgetAssembler.h \
        src/assets/interface/WidgetAssembler.cpp \
        src/assets/interface/WidgetLoader.cpp
fi

printf '\n===== ARCHITECTURE CHECK =====\n'

if rg -n \
    'InterfaceComposer|ComposedInterface|InterfaceNode|"Widget\.h"|"repositories/WidgetRepository\.h"' \
    src/assets \
    --glob '*.h' \
    --glob '*.cpp' \
    --glob 'CMakeLists.txt'
then
    printf 'Stale interface architecture references remain.\n' >&2
    exit 1
fi

printf '\n===== CONFIGURE =====\n'
cmake -S . -B build

printf '\n===== BUILD =====\n'
cmake --build build -j 4

printf '\n===== RESULT =====\n'
git status --short
git --no-pager diff --stat
