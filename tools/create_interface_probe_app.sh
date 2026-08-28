#!/usr/bin/env bash
set -euo pipefail

APP_NAME="interface_probe"
APP_DIR="src/apps/${APP_NAME}"
ROOT_CMAKE="CMakeLists.txt"

echo "== Eldoria interface probe app creator =="

if [ ! -f "${ROOT_CMAKE}" ]; then
    echo "ERROR: ${ROOT_CMAKE} not found."
    echo "Run this from the Eldoria repo root."
    exit 1
fi

if [ ! -d "src/apps" ]; then
    echo "ERROR: src/apps not found."
    echo "Run this from the Eldoria repo root."
    exit 1
fi

if [ -d "${APP_DIR}" ]; then
    echo "ERROR: ${APP_DIR} already exists."
    echo "Refusing to overwrite it."
    exit 1
fi

mkdir -p "${APP_DIR}"

cat > "${APP_DIR}/CMakeLists.txt" <<'CMAKE'
add_executable(interface_probe
    main.cpp
)

target_link_libraries(interface_probe PRIVATE
    Eldoria::Data
)

set_target_properties(interface_probe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
CMAKE

cat > "${APP_DIR}/main.cpp" <<'CPP'
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "cache/Cache.h"
#include "cache/File.h"
#include "cache/Index.h"
#include "cache/Store.h"

namespace {

void printUsage(std::string_view program)
{
    std::cout
        << "Usage:\n"
        << "  " << program << " <cache-root> [config-file-id]\n\n"
        << "Examples:\n"
        << "  " << program << " ./cache\n"
        << "  " << program << " ./cache 3\n";
}

std::uint16_t parseFileId(const char* text)
{
    const int value = std::stoi(text);

    if (value < 0 || value > 65535) {
        throw std::runtime_error("config-file-id must fit in uint16_t");
    }

    return static_cast<std::uint16_t>(value);
}

void printHexPreview(
    const std::vector<std::uint8_t>& bytes,
    std::size_t maxCount = 256
)
{
    const std::size_t count = std::min(bytes.size(), maxCount);

    std::cout
        << "\nHex preview: "
        << count
        << " / "
        << bytes.size()
        << " bytes\n";

    for (std::size_t i = 0; i < count; ++i) {
        if (i % 16 == 0) {
            std::cout
                << "\n"
                << std::setw(6)
                << std::setfill('0')
                << std::hex
                << i
                << "  ";
        }

        std::cout
            << std::setw(2)
            << std::setfill('0')
            << std::hex
            << static_cast<int>(bytes[i])
            << " ";
    }

    std::cout << std::dec << "\n\n";
}

void printAsciiPreview(
    const std::vector<std::uint8_t>& bytes,
    std::size_t maxCount = 256
)
{
    const std::size_t count = std::min(bytes.size(), maxCount);

    std::cout
        << "ASCII preview: "
        << count
        << " / "
        << bytes.size()
        << " bytes\n\n";

    for (std::size_t i = 0; i < count; ++i) {
        const auto value = bytes[i];

        if (value >= 32 && value <= 126) {
            std::cout << static_cast<char>(value);
        }
        else {
            std::cout << '.';
        }
    }

    std::cout << "\n\n";
}

} // namespace

int main(int argc, char** argv)
{
    if (argc < 2 || argc > 3) {
        printUsage(argv[0]);
        return 1;
    }

    try {
        const std::filesystem::path cacheRoot = argv[1];

        std::cout << "Eldoria Interface Probe\n";
        std::cout << "Cache root: " << cacheRoot << "\n\n";

        eld::cache::Cache cache(cacheRoot);
        eld::cache::Store configStore =
            cache.open(eld::cache::IndexId::Config);

        const auto entries = configStore.list();

        std::cout << "Config index entries: " << entries.size() << "\n\n";

        for (const auto& entry : entries) {
            std::cout
                << "file="
                << entry.fileId
                << " size="
                << entry.indexEntry.size
                << " firstSector="
                << entry.indexEntry.firstSector
                << "\n";
        }

        if (argc == 2) {
            std::cout
                << "\nNo config file selected.\n"
                << "Run again with a file id to dump a candidate payload.\n";

            return 0;
        }

        const std::uint16_t fileId = parseFileId(argv[2]);

        std::cout
            << "\nLoading config file "
            << fileId
            << "...\n";

        const eld::cache::File file = configStore.get(fileId);
        const auto bytes = file.getBytes();

        std::cout
            << "Loaded file "
            << file.getId()
            << "\n"
            << "Index size: "
            << file.getEntry().size
            << "\n"
            << "Decoded byte size: "
            << bytes.size()
            << "\n";

        printHexPreview(bytes);
        printAsciiPreview(bytes);

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "interface_probe failed: " << e.what() << "\n";
        return 1;
    }
}
CPP

python3 - <<'PY'
from pathlib import Path

path = Path("CMakeLists.txt")
text = path.read_text()

line = "eldoria_add_app(interface_probe src/apps/interface_probe)"

if line in text:
    print("Root CMake already registers interface_probe.")
    raise SystemExit(0)

anchor = "eldoria_add_app(elserver src/apps/elserver)"

if anchor not in text:
    raise SystemExit(
        "ERROR: Could not find elserver app registration anchor in root CMakeLists.txt."
    )

text = text.replace(anchor, anchor + "\n" + line)
path.write_text(text)

print("Registered interface_probe in root CMakeLists.txt.")
PY

echo ""
echo "Created:"
echo "  ${APP_DIR}/CMakeLists.txt"
echo "  ${APP_DIR}/main.cpp"
echo ""
echo "Build:"
echo "  cmake --build build"
echo ""
echo "Run:"
echo "  ./build/bin/interface_probe <cache-root>"
echo "  ./build/bin/interface_probe <cache-root> <config-file-id>"
