#!/usr/bin/env bash

set -euo pipefail

project_dir="${1:-$PWD}"

cd "$project_dir"

required_files=(
    src/assets/content/map/MapIndex.h
    src/assets/content/map/MapRegion.h
    src/assets/content/map/MapTile.h
    src/assets/decoders/LocationSpawnDecoder.cpp
    src/assets/decoders/LocationSpawnDecoder.h
    src/assets/decoders/MapIndexDecoder.cpp
    src/assets/decoders/MapIndexDecoder.h
    src/assets/decoders/TerrainDecoder.cpp
    src/assets/decoders/TerrainDecoder.h
    src/assets/repositories/MapRepository.cpp
    src/assets/repositories/MapRepository.h
    src/assets/AssetManager.h
    src/assets/CMakeLists.txt
)

for file in "${required_files[@]}"; do
    if [[ ! -f "$file" ]]; then
        printf 'Missing: %s\n' "$file" >&2
        exit 1
    fi
done

backup_dir="$(mktemp -d "$project_dir/eldoria-map-backup.XXXXXXXX")"

for file in "${required_files[@]}"; do
    mkdir -p "$backup_dir/$(dirname "$file")"
    cp -a "$file" "$backup_dir/$file"
done

if [[ -f src/assets/midi/MidiLoader.h ]]; then
    mkdir -p "$backup_dir/src/assets/midi"
    cp -a src/assets/midi/MidiLoader.h \
        "$backup_dir/src/assets/midi/MidiLoader.h"

    python3 - <<'PY'
from pathlib import Path

path = Path("src/assets/midi/MidiLoader.h")
text = path.read_text()

if "eld::cache::IndexId::MidiData" in text:
    path.write_text(
        text.replace(
            "eld::cache::IndexId::MidiData",
            "eld::cache::IndexId::Midi"
        )
    )
    print("Repaired the lingering MIDI cache-index typo.")
PY
fi

printf 'Backup: %s\n' "$backup_dir"

mkdir -p src/assets/map

mv src/assets/content/map/MapTile.h \
    src/assets/map/MapTile.h

cat > src/assets/map/MapIndexData.h <<'EOF'
#pragma once

#include <cstdint>
#include <vector>

namespace eld::map {

struct MapIndexEntry {
    std::uint16_t regionId = 0;
    std::uint16_t terrainFileId = 0;
    std::uint16_t locationFileId = 0;
    bool shouldPreload = false;

    int regionX() const {
        return static_cast<int>(regionId >> 8);
    }

    int regionY() const {
        return static_cast<int>(regionId & 0xFFu);
    }
};

struct MapIndexData {
    std::vector<MapIndexEntry> entries;
};

}
EOF

cat > src/assets/map/TerrainData.h <<'EOF'
#pragma once

#include <array>
#include <cstddef>

#include "map/MapTile.h"

namespace eld::map {

struct TerrainData {
    std::array<MapTile, RegionTileCount> tiles{};

    MapTile& operator[](std::size_t index) {
        return tiles[index];
    }

    const MapTile& operator[](std::size_t index) const {
        return tiles[index];
    }

    MapTile& at(std::size_t index) {
        return tiles.at(index);
    }

    const MapTile& at(std::size_t index) const {
        return tiles.at(index);
    }

    const MapTile& tile(
        std::size_t plane,
        std::size_t x,
        std::size_t y
    ) const {
        return at(tileIndex(plane, x, y));
    }
};

}
EOF

cat > src/assets/map/MapLocationData.h <<'EOF'
#pragma once

#include <cstdint>
#include <vector>

namespace eld::map {

struct MapLocationSpawn {
    std::uint16_t id = 0;
    std::uint8_t plane = 0;
    std::uint8_t x = 0;
    std::uint8_t y = 0;
    std::uint8_t type = 0;
    std::uint8_t rotation = 0;
};

using MapLocationData =
    std::vector<MapLocationSpawn>;

}
EOF

cat > src/assets/map/MapRegionResource.h <<'EOF'
#pragma once

#include <cstddef>
#include <cstdint>

#include "map/MapLocationData.h"
#include "map/TerrainData.h"

namespace eld::map {

struct MapRegionResource {
    std::uint16_t regionId = 0;
    std::uint16_t terrainFileId = 0;
    std::uint16_t locationFileId = 0;
    bool shouldPreload = false;

    TerrainData tiles;
    MapLocationData locations;

    int regionX() const {
        return static_cast<int>(regionId >> 8);
    }

    int regionY() const {
        return static_cast<int>(regionId & 0xFFu);
    }

    int worldBaseX() const {
        return regionX() * static_cast<int>(RegionSize);
    }

    int worldBaseY() const {
        return regionY() * static_cast<int>(RegionSize);
    }

    const MapTile& tile(
        std::size_t plane,
        std::size_t x,
        std::size_t y
    ) const {
        return tiles.tile(plane, x, y);
    }
};

}
EOF

cat > src/assets/map/MapAssembler.h <<'EOF'
#pragma once

#include "map/MapIndexData.h"
#include "map/MapLocationData.h"
#include "map/MapRegionResource.h"
#include "map/TerrainData.h"

namespace eld::map {

class MapAssembler {
public:
    MapRegionResource assemble(
        const MapIndexEntry& entry,
        const TerrainData& terrain,
        const MapLocationData& locations
    ) const;
};

}
EOF

cat > src/assets/map/MapAssembler.cpp <<'EOF'
#include "map/MapAssembler.h"

namespace eld::map {

MapRegionResource MapAssembler::assemble(
    const MapIndexEntry& entry,
    const TerrainData& terrain,
    const MapLocationData& locations
) const {
    MapRegionResource resource;

    resource.regionId = entry.regionId;
    resource.terrainFileId = entry.terrainFileId;
    resource.locationFileId = entry.locationFileId;
    resource.shouldPreload = entry.shouldPreload;
    resource.tiles = terrain;
    resource.locations = locations;

    return resource;
}

}
EOF

mv src/assets/decoders/MapIndexDecoder.h \
    src/assets/map/MapIndexDecoder.h
mv src/assets/decoders/MapIndexDecoder.cpp \
    src/assets/map/MapIndexDecoder.cpp
mv src/assets/decoders/TerrainDecoder.h \
    src/assets/map/TerrainDecoder.h
mv src/assets/decoders/TerrainDecoder.cpp \
    src/assets/map/TerrainDecoder.cpp
mv src/assets/decoders/LocationSpawnDecoder.h \
    src/assets/map/LocationSpawnDecoder.h
mv src/assets/decoders/LocationSpawnDecoder.cpp \
    src/assets/map/LocationSpawnDecoder.cpp

python3 - <<'PY'
from pathlib import Path

replacements = {
    Path("src/assets/map/MapTile.h"): [
        ("#include <array>\n", ""),
        ("using MapTileArray = std::array<MapTile, RegionTileCount>;\n\n", ""),
    ],
    Path("src/assets/map/MapIndexDecoder.h"): [
        ('#include "map/MapIndex.h"', '#include "map/MapIndexData.h"'),
        ("MapIndex decode(", "MapIndexData decode("),
    ],
    Path("src/assets/map/MapIndexDecoder.cpp"): [
        ('#include "decoders/MapIndexDecoder.h"', '#include "map/MapIndexDecoder.h"'),
        ("MapIndex MapIndexDecoder::decode(", "MapIndexData MapIndexDecoder::decode("),
        ("MapIndex index;", "MapIndexData index;"),
    ],
    Path("src/assets/map/TerrainDecoder.h"): [
        ('#include "map/MapTile.h"', '#include "map/TerrainData.h"'),
        ("MapTileArray decode(", "TerrainData decode("),
    ],
    Path("src/assets/map/TerrainDecoder.cpp"): [
        ('#include "decoders/TerrainDecoder.h"', '#include "map/TerrainDecoder.h"'),
        ("MapTileArray TerrainDecoder::decode(", "TerrainData TerrainDecoder::decode("),
        ("MapTileArray tiles{};", "TerrainData tiles{};"),
    ],
    Path("src/assets/map/LocationSpawnDecoder.h"): [
        ('#include <vector>\n', ''),
        ('#include "map/MapRegion.h"', '#include "map/MapLocationData.h"'),
        ("std::vector<MapLocationSpawn> decode(", "MapLocationData decode("),
    ],
    Path("src/assets/map/LocationSpawnDecoder.cpp"): [
        ('#include "decoders/LocationSpawnDecoder.h"', '#include "map/LocationSpawnDecoder.h"'),
        ("std::vector<MapLocationSpawn> LocationSpawnDecoder::decode(",
         "MapLocationData LocationSpawnDecoder::decode("),
        ("std::vector<MapLocationSpawn> objects;", "MapLocationData objects;"),
    ],
}

for path, edits in replacements.items():
    text = path.read_text()
    for old, new in edits:
        count = text.count(old)
        if count != 1:
            raise SystemExit(
                f"{path}: expected one occurrence of {old!r}, found {count}"
            )
        text = text.replace(old, new)
    path.write_text(text)
PY

cat > src/assets/map/MapLoader.h <<'EOF'
#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string_view>
#include <vector>

#include "cache/Cache.h"
#include "cache/Store.h"
#include "map/LocationSpawnDecoder.h"
#include "map/MapAssembler.h"
#include "map/MapIndexData.h"
#include "map/MapIndexDecoder.h"
#include "map/MapLocationData.h"
#include "map/MapRegionResource.h"
#include "map/TerrainData.h"
#include "map/TerrainDecoder.h"

namespace eld::map {

class MapLoader {
public:
    explicit MapLoader(
        const eld::cache::Cache& cache
    );

    const TerrainData& terrain(
        std::uint16_t regionId
    ) const;

    const MapLocationData& locations(
        std::uint16_t regionId
    ) const;

    const MapRegionResource& resource(
        std::uint16_t regionId
    ) const;

    std::optional<MapRegionResource> find(
        std::uint16_t regionId
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t regionId
    ) const;

    std::size_t count() const;

    const MapIndexEntry* findEntry(
        std::uint16_t regionId
    ) const;

    const std::vector<MapIndexEntry>&
    entries() const;

private:
    static constexpr auto Index =
        eld::cache::IndexId::Maps;

    static constexpr auto ConfigIndex =
        eld::cache::IndexId::Config;

    static constexpr std::uint16_t VersionListArchiveId =
        5;

    static constexpr std::string_view MapIndexFile =
        "map_index";

    MapIndexData readMapIndex(
        const eld::cache::Cache& cache
    ) const;

    const MapIndexEntry& requireEntry(
        std::uint16_t regionId
    ) const;

    TerrainData loadTerrain(
        const MapIndexEntry& entry
    ) const;

    MapLocationData loadLocations(
        const MapIndexEntry& entry
    ) const;

    eld::cache::Store maps_;
    MapIndexData index_;

    MapIndexDecoder indexDecoder_;
    TerrainDecoder terrainDecoder_;
    LocationSpawnDecoder locationDecoder_;
    MapAssembler assembler_;

    mutable std::map<
        std::uint16_t,
        TerrainData
    > terrainCache_;

    mutable std::map<
        std::uint16_t,
        MapLocationData
    > locationCache_;

    mutable std::map<
        std::uint16_t,
        MapRegionResource
    > resourceCache_;
};

}
EOF

cat > src/assets/map/MapLoader.cpp <<'EOF'
#include "map/MapLoader.h"

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <string>

#include "archive/Archive.h"
#include "cache/File.h"

namespace eld::map {

MapLoader::MapLoader(
    const eld::cache::Cache& cache
)
    : maps_(cache.open(Index)) {
    index_ = readMapIndex(cache);
}

const TerrainData& MapLoader::terrain(
    std::uint16_t regionId
) const {
    const auto cached = terrainCache_.find(regionId);

    if (cached != terrainCache_.end()) {
        return cached->second;
    }

    const auto [inserted, wasInserted] =
        terrainCache_.emplace(
            regionId,
            loadTerrain(requireEntry(regionId))
        );

    (void)wasInserted;
    return inserted->second;
}

const MapLocationData& MapLoader::locations(
    std::uint16_t regionId
) const {
    const auto cached = locationCache_.find(regionId);

    if (cached != locationCache_.end()) {
        return cached->second;
    }

    const auto [inserted, wasInserted] =
        locationCache_.emplace(
            regionId,
            loadLocations(requireEntry(regionId))
        );

    (void)wasInserted;
    return inserted->second;
}

const MapRegionResource& MapLoader::resource(
    std::uint16_t regionId
) const {
    const auto cached = resourceCache_.find(regionId);

    if (cached != resourceCache_.end()) {
        return cached->second;
    }

    const auto [inserted, wasInserted] =
        resourceCache_.emplace(
            regionId,
            assembler_.assemble(
                requireEntry(regionId),
                terrain(regionId),
                locations(regionId)
            )
        );

    (void)wasInserted;
    return inserted->second;
}

std::optional<MapRegionResource> MapLoader::find(
    std::uint16_t regionId
) const {
    if (!contains(regionId)) {
        return std::nullopt;
    }

    return resource(regionId);
}

std::vector<std::uint16_t>
MapLoader::listIds() const {
    std::vector<std::uint16_t> ids;
    ids.reserve(index_.entries.size());

    for (const MapIndexEntry& entry : index_.entries) {
        ids.push_back(entry.regionId);
    }

    return ids;
}

bool MapLoader::contains(
    std::uint16_t regionId
) const {
    return findEntry(regionId) != nullptr;
}

std::size_t MapLoader::count() const {
    return index_.entries.size();
}

const MapIndexEntry* MapLoader::findEntry(
    std::uint16_t regionId
) const {
    const auto entry =
        std::lower_bound(
            index_.entries.begin(),
            index_.entries.end(),
            regionId,
            [](
                const MapIndexEntry& candidate,
                std::uint16_t id
            ) {
                return candidate.regionId < id;
            }
        );

    return
        entry != index_.entries.end() &&
        entry->regionId == regionId
            ? &*entry
            : nullptr;
}

const std::vector<MapIndexEntry>&
MapLoader::entries() const {
    return index_.entries;
}

MapIndexData MapLoader::readMapIndex(
    const eld::cache::Cache& cache
) const {
    const eld::archive::Archive archive =
        eld::archive::load(
            cache.open(ConfigIndex),
            VersionListArchiveId
        );

    const eld::archive::ArchiveFile& file =
        archive.get(MapIndexFile);

    try {
        return indexDecoder_.decode(file.payload);
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode map index: " +
            std::string(error.what())
        );
    }
}

const MapIndexEntry& MapLoader::requireEntry(
    std::uint16_t regionId
) const {
    const MapIndexEntry* entry = findEntry(regionId);

    if (entry == nullptr) {
        throw std::out_of_range(
            "Map region does not exist: " +
            std::to_string(regionId)
        );
    }

    return *entry;
}

TerrainData MapLoader::loadTerrain(
    const MapIndexEntry& entry
) const {
    const eld::cache::File file =
        maps_.get(entry.terrainFileId);

    const std::vector<std::uint8_t> payload =
        file.getBytes(
            eld::cache::CompressionState::Decompressed
        );

    try {
        return terrainDecoder_.decode(
            payload,
            entry.regionId
        );
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode map terrain for region " +
            std::to_string(entry.regionId) +
            ": " +
            error.what()
        );
    }
}

MapLocationData MapLoader::loadLocations(
    const MapIndexEntry& entry
) const {
    const eld::cache::File file =
        maps_.get(entry.locationFileId);

    const std::vector<std::uint8_t> payload =
        file.getBytes(
            eld::cache::CompressionState::Decompressed
        );

    try {
        return locationDecoder_.decode(payload);
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode map locations for region " +
            std::to_string(entry.regionId) +
            ": " +
            error.what()
        );
    }
}

}
EOF

python3 - <<'PY'
from pathlib import Path

source_root = Path("src")
source_files = [
    path
    for path in source_root.rglob("*")
    if path.suffix in {".h", ".cpp"} or path.name == "CMakeLists.txt"
]

common_replacements = [
    ('#include "repositories/MapRepository.h"', '#include "map/MapLoader.h"'),
    ('#include "map/MapIndex.h"', '#include "map/MapIndexData.h"'),
    ('#include "map/MapRegion.h"', '#include "map/MapRegionResource.h"'),
    ("eld::map::MapRepository", "eld::map::MapLoader"),
    ("mapRepository_", "mapLoader_"),
    ("eld::map::MapRegion", "eld::map::MapRegionResource"),
]

for path in source_files:
    text = path.read_text()
    changed = text

    for old, new in common_replacements:
        changed = changed.replace(old, new)

    if changed != text:
        path.write_text(changed)

scene_location_header = Path(
    "src/runtime/render/map/SceneLocationBuilder.h"
)

if scene_location_header.exists():
    text = scene_location_header.read_text()
    text = text.replace(
        '#include "map/MapRegionResource.h"',
        '#include "map/MapLocationData.h"'
    )
    scene_location_header.write_text(text)

map_view = Path("src/apps/elforge/views/map/MapView.cpp")

if map_view.exists():
    text = map_view.read_text()

    edits = [
        ("eld::map::MapRegionResource region;", "eld::map::TerrainData region;"),
        ("const eld::map::MapRegionResource *", "const eld::map::TerrainData *"),
        ("const eld::map::MapRegionResource &region", "const eld::map::TerrainData &region"),
        ("loader.loadTerrain(candidateId)", "loader.terrain(candidateId)"),
        ("loader_.get(regionId)", "loader_.resource(regionId)"),
    ]

    for old, new in edits:
        if old not in text:
            raise SystemExit(
                f"{map_view}: expected migration pattern {old!r}"
            )
        text = text.replace(old, new)

    map_view.write_text(text)

cmake = Path("src/assets/CMakeLists.txt")
text = cmake.read_text()

old_block = """    decoders/MapIndexDecoder.cpp
    decoders/TerrainDecoder.cpp
    decoders/LocationSpawnDecoder.cpp
    repositories/MapRepository.cpp
"""

new_block = """    map/MapIndexDecoder.cpp
    map/TerrainDecoder.cpp
    map/LocationSpawnDecoder.cpp
    map/MapAssembler.cpp
    map/MapLoader.cpp
"""

if text.count(old_block) != 1:
    raise SystemExit(
        "src/assets/CMakeLists.txt: old map source block was not found exactly once"
    )

cmake.write_text(text.replace(old_block, new_block))
PY

rm src/assets/content/map/MapIndex.h
rm src/assets/content/map/MapRegion.h
rm src/assets/repositories/MapRepository.h
rm src/assets/repositories/MapRepository.cpp

if [[ -d src/assets/content/map ]] && \
   [[ -z "$(find src/assets/content/map -mindepth 1 -print -quit)" ]]; then
    rmdir src/assets/content/map
fi

mapfile -t format_files < <(
    find src/assets/map \
        -type f \
        \( -name '*.h' -o -name '*.cpp' \) \
        -print
)

format_files+=(
    src/assets/AssetManager.h
    src/apps/elforge/explorer/CacheExplorer.h
    src/apps/elforge/explorer/CacheExplorer.cpp
    src/apps/elforge/explorer/CacheExplorerSelection.cpp
    src/apps/elforge/explorer/tree/CacheTreeBuilder.cpp
    src/apps/elforge/views/map/MapView.h
    src/apps/elforge/views/map/MapView.cpp
    src/apps/elforge/views/map/MapViewState.h
)

existing_format_files=()

for file in "${format_files[@]}"; do
    if [[ -f "$file" ]]; then
        existing_format_files+=("$file")
    fi
done

if command -v clang-format >/dev/null 2>&1; then
    clang-format -i "${existing_format_files[@]}"
fi

printf '\n===== ARCHITECTURE CHECK =====\n'

if rg -n \
    'MapRepository|repositories/MapRepository|decoders/(MapIndex|Terrain|LocationSpawn)Decoder|map/Map(Index|Region)\.h|\.loadTerrain\(|loader_\.get\(' \
    src \
    --glob '*.h' \
    --glob '*.cpp' \
    --glob 'CMakeLists.txt'; then
    printf 'Old map architecture references remain.\n' >&2
    exit 1
fi

rg -n \
    'class MapLoader|class MapAssembler|struct MapIndexData|struct TerrainData|using MapLocationData|struct MapRegionResource' \
    src/assets/map

printf '\n===== CONFIGURE =====\n'
cmake -S . -B build

printf '\n===== BUILD =====\n'
cmake --build build -j 4

printf '\n===== STATUS =====\n'
git status --short

printf '\nMap migration complete.\n'
