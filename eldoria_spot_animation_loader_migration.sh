#!/usr/bin/env bash
(
set -euo pipefail
cd "${HOME}/projects/Eldoria" || exit 1

if [[ "$(git branch --show-current)" != "feature/asset-manager" ]]; then
    echo "Expected feature/asset-manager; no files changed."
    exit 1
fi

required=(
    src/assets/SpotAnimation.h
    src/assets/decoders/SpotAnimationDecoder.h
    src/assets/decoders/SpotAnimationDecoder.cpp
    src/assets/repositories/SpotAnimationRepository.h
    src/assets/repositories/SpotAnimationRepository.cpp
)

for file in "${required[@]}"; do
    [[ -f "$file" ]] || { echo "Missing: $file; no files changed."; exit 1; }
done

[[ ! -e src/assets/spot_animation ]] || {
    echo "src/assets/spot_animation already exists; no files changed."
    exit 1
}

backup="$(mktemp -d "$PWD/eldoria-spot-animation-backup.XXXXXXXX")"
cp -a src "$backup/"
echo "Backup: $backup"

mkdir -p src/assets/spot_animation
mv src/assets/SpotAnimation.h src/assets/spot_animation/SpotAnimationData.h
mv src/assets/decoders/SpotAnimationDecoder.h src/assets/spot_animation/SpotAnimationDecoder.h
mv src/assets/decoders/SpotAnimationDecoder.cpp src/assets/spot_animation/SpotAnimationDecoder.cpp
mv src/assets/repositories/SpotAnimationRepository.h src/assets/spot_animation/SpotAnimationLoader.h
mv src/assets/repositories/SpotAnimationRepository.cpp src/assets/spot_animation/SpotAnimationLoader.cpp

python3 - <<'PY'
from pathlib import Path
import re

root = Path("src")

for path in root.rglob("*"):
    if not path.is_file() or path.is_symlink():
        continue
    if path.suffix not in {".h", ".hpp", ".cpp", ".cc", ".cxx"} and path.name != "CMakeLists.txt":
        continue

    text = path.read_text()
    text = text.replace('#include "SpotAnimation.h"', '#include "spot_animation/SpotAnimationData.h"')
    text = text.replace('#include "decoders/SpotAnimationDecoder.h"', '#include "spot_animation/SpotAnimationDecoder.h"')
    text = text.replace('#include "repositories/SpotAnimationRepository.h"', '#include "spot_animation/SpotAnimationLoader.h"')
    text = text.replace("SpotAnimationRepository", "SpotAnimationLoader")
    text = text.replace("spotAnimationRepository", "spotAnimationLoader")
    text = text.replace("eld::spot_animation::SpotAnimation", "eld::spot_animation::SpotAnimationData")
    text = text.replace("spotAnimations_->get(", "spotAnimations_->data(")

    if path.as_posix().endswith("CacheTreeBuilder.cpp"):
        text = re.sub(
            r"const eld::spot_animation::SpotAnimationData\s+definition\s*=\s*repository\.get\(id\);",
            "const eld::spot_animation::SpotAnimationData& definition =\n            repository.data(id);",
            text,
        )

    path.write_text(text)

data_path = Path("src/assets/spot_animation/SpotAnimationData.h")
text = data_path.read_text()
text = re.sub(r"\bstruct SpotAnimation\b", "struct SpotAnimationData", text)
data_path.write_text(text)

for name in (
    "src/assets/spot_animation/SpotAnimationDecoder.h",
    "src/assets/spot_animation/SpotAnimationDecoder.cpp",
):
    path = Path(name)
    text = path.read_text()
    text = text.replace('#include "spot_animation/SpotAnimationData.h"', '#include "spot_animation/SpotAnimationData.h"')
    text = text.replace('#include "spot_animation/SpotAnimationDecoder.h"', '#include "spot_animation/SpotAnimationDecoder.h"')
    text = re.sub(r"\bSpotAnimation\b", "SpotAnimationData", text)
    path.write_text(text)

Path("src/assets/spot_animation/SpotAnimationResource.h").write_text(r'''#pragma once

#include "model/ModelResource.h"
#include "sequence/SequenceResource.h"
#include "spot_animation/SpotAnimationData.h"

namespace eld::spot_animation {

struct SpotAnimationResource {
    SpotAnimationData data;

    const eld::model::ModelResource*
        model = nullptr;

    const eld::sequence::SequenceResource*
        sequence = nullptr;
};

}
''')

Path("src/assets/spot_animation/SpotAnimationAssembler.h").write_text(r'''#pragma once

#include "model/ModelLoader.h"
#include "sequence/SequenceLoader.h"
#include "spot_animation/SpotAnimationData.h"
#include "spot_animation/SpotAnimationResource.h"

namespace eld::spot_animation {

class SpotAnimationAssembler {
public:
    SpotAnimationAssembler() = default;

    SpotAnimationAssembler(
        const eld::model::ModelLoader& models,
        const eld::sequence::SequenceLoader& sequences
    );

    SpotAnimationResource assemble(
        SpotAnimationData data
    ) const;

private:
    const eld::model::ModelLoader* models_ = nullptr;
    const eld::sequence::SequenceLoader* sequences_ = nullptr;
};

}
''')

Path("src/assets/spot_animation/SpotAnimationAssembler.cpp").write_text(r'''#include "spot_animation/SpotAnimationAssembler.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace eld::spot_animation {

SpotAnimationAssembler::SpotAnimationAssembler(
    const eld::model::ModelLoader& models,
    const eld::sequence::SequenceLoader& sequences
)
    : models_(&models),
      sequences_(&sequences) {
}


SpotAnimationResource SpotAnimationAssembler::assemble(
    SpotAnimationData data
) const {
    if (models_ == nullptr || sequences_ == nullptr) {
        throw std::logic_error(
            "Spot animation assembler requires model and sequence loaders"
        );
    }

    SpotAnimationResource resource{
        .data = std::move(data),
        .model = nullptr,
        .sequence = nullptr
    };

    if (resource.data.modelId.has_value()) {
        if (!models_->contains(*resource.data.modelId)) {
            throw std::runtime_error(
                "Spot animation " + std::to_string(resource.data.id) +
                " references missing model " +
                std::to_string(*resource.data.modelId)
            );
        }

        resource.model = &models_->resource(*resource.data.modelId);
    }

    if (resource.data.sequenceId.has_value()) {
        if (!sequences_->contains(*resource.data.sequenceId)) {
            throw std::runtime_error(
                "Spot animation " + std::to_string(resource.data.id) +
                " references missing sequence " +
                std::to_string(*resource.data.sequenceId)
            );
        }

        resource.sequence = &sequences_->resource(*resource.data.sequenceId);
    }

    return resource;
}

}
''')

Path("src/assets/spot_animation/SpotAnimationLoader.h").write_text(r'''#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string_view>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "model/ModelLoader.h"
#include "sequence/SequenceLoader.h"
#include "spot_animation/SpotAnimationAssembler.h"
#include "spot_animation/SpotAnimationData.h"
#include "spot_animation/SpotAnimationDecoder.h"
#include "spot_animation/SpotAnimationResource.h"

namespace eld::spot_animation {

class SpotAnimationLoader {
public:
    explicit SpotAnimationLoader(
        const eld::cache::Cache& cache
    );

    SpotAnimationLoader(
        const eld::cache::Cache& cache,
        const eld::model::ModelLoader& models,
        const eld::sequence::SequenceLoader& sequences
    );

    const SpotAnimationData& data(
        std::uint16_t id
    ) const;

    std::optional<SpotAnimationData> find(
        std::uint16_t id
    ) const;

    const SpotAnimationResource& resource(
        std::uint16_t id
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    static constexpr auto Index = eld::cache::IndexId::Config;
    static constexpr std::uint16_t ArchiveId = 2;
    static constexpr std::string_view DataFile = "spotanim.dat";
    static constexpr std::string_view IndexFile = "spotanim.idx";

    SpotAnimationData loadData(
        std::uint16_t id
    ) const;

    eld::archive::Archive archive_;
    SpotAnimationDecoder decoder_;
    SpotAnimationAssembler assembler_;
    mutable std::map<
        std::uint16_t,
        SpotAnimationData
    > dataCache_;

    mutable std::map<
        std::uint16_t,
        SpotAnimationResource
    > resourceCache_;
};

}
''')

old_cpp = Path("src/assets/spot_animation/SpotAnimationLoader.cpp").read_text()
old_cpp = re.sub(r"\bSpotAnimation\b", "SpotAnimationData", old_cpp)
load_body = re.search(
    r"SpotAnimationData SpotAnimationLoader::get\(.*?\n\}",
    old_cpp,
    re.DOTALL,
)
if load_body is None:
    raise SystemExit("Could not preserve spot animation decoding implementation")
load_code = load_body.group(0).replace(
    "SpotAnimationData SpotAnimationLoader::get(",
    "SpotAnimationData SpotAnimationLoader::loadData(",
)

Path("src/assets/spot_animation/SpotAnimationLoader.cpp").write_text(r'''#include "spot_animation/SpotAnimationLoader.h"

namespace eld::spot_animation {

SpotAnimationLoader::SpotAnimationLoader(
    const eld::cache::Cache& cache
)
    : archive_(eld::archive::load(cache.open(Index), ArchiveId)) {
}

SpotAnimationLoader::SpotAnimationLoader(
    const eld::cache::Cache& cache,
    const eld::model::ModelLoader& models,
    const eld::sequence::SequenceLoader& sequences
)
    : SpotAnimationLoader(cache) {
    assembler_ = SpotAnimationAssembler(models, sequences);
}

const SpotAnimationData& SpotAnimationLoader::data(
    std::uint16_t id
) const {
    const auto existing = dataCache_.find(id);

    if (existing != dataCache_.end()) {
        return existing->second;
    }

    return dataCache_.emplace(
        id,
        loadData(id)
    ).first->second;
}

''' + load_code + r'''

std::optional<SpotAnimationData> SpotAnimationLoader::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return data(id);
}

const SpotAnimationResource& SpotAnimationLoader::resource(
    std::uint16_t id
) const {
    const auto existing = resourceCache_.find(id);

    if (existing != resourceCache_.end()) {
        return existing->second;
    }

    return resourceCache_.emplace(
        id,
        assembler_.assemble(data(id))
    ).first->second;
}

std::vector<std::uint16_t> SpotAnimationLoader::listIds() const {
    std::vector<std::uint16_t> ids;
    const std::size_t assetCount = count();

    ids.reserve(assetCount);

    for (std::size_t id = 0; id < assetCount; ++id) {
        ids.push_back(static_cast<std::uint16_t>(id));
    }
    return ids;
}

bool SpotAnimationLoader::contains(
    std::uint16_t id
) const {
    return id < count();
}

std::size_t SpotAnimationLoader::count() const {
    eld::binary::ByteReader dataReader(archive_.get(DataFile).payload);
    eld::binary::ByteReader indexReader(archive_.get(IndexFile).payload);
    const auto dataCount = dataReader.readU16();
    const auto indexCount = indexReader.readU16();
    if (dataCount != indexCount) {
        throw std::runtime_error("spot animation data/index count mismatch");
    }
    return dataCount;
}

}
''')

path = Path("src/assets/spot_animation/SpotAnimationLoader.cpp")
text = path.read_text()
text = text.replace('#include "spot_animation/SpotAnimationLoader.h"\n', '#include "spot_animation/SpotAnimationLoader.h"\n\n#include <exception>\n#include <stdexcept>\n#include <string>\n#include <vector>\n\n#include "binary/ByteReader.h"\n')
path.write_text(text)

path = Path("src/assets/AssetManager.cpp")
text = path.read_text().replace(
    "spotAnimations(cache)",
    "spotAnimations(cache, models, sequences)",
)
path.write_text(text)

path = Path("src/assets/CMakeLists.txt")
text = path.read_text()
text = text.replace(
    "    decoders/SpotAnimationDecoder.cpp\n    repositories/SpotAnimationLoader.cpp\n",
    "    spot_animation/SpotAnimationDecoder.cpp\n"
    "    spot_animation/SpotAnimationAssembler.cpp\n"
    "    spot_animation/SpotAnimationLoader.cpp\n",
)
path.write_text(text)
PY

echo
echo "===== ARCHITECTURE CHECK ====="
if rg -n 'SpotAnimationRepository|#include "SpotAnimation.h"|decoders/SpotAnimationDecoder|repositories/SpotAnimationLoader' src; then
    echo "Old spot-animation references remain."
    exit 1
fi

echo
echo "===== BUILD ====="
cmake -S . -B build
cmake --build build -j 4

echo
echo "===== RESULT ====="
git status --short
git --no-pager diff --stat
)
