#!/usr/bin/env bash
(
set -euo pipefail

cd "${HOME}/projects/Eldoria" || exit 1

test "$(git branch --show-current)" = "feature/asset-manager" || {
    echo "Expected feature/asset-manager branch"
    exit 1
}

required=(
    src/assets/Animation.h
    src/assets/AnimationFrameTable.h
    src/assets/AnimationFrameTable.cpp
    src/assets/decoders/AnimationDecoder.h
    src/assets/decoders/AnimationDecoder.cpp
    src/assets/repositories/AnimationRepository.h
    src/assets/repositories/AnimationRepository.cpp
    src/assets/Sequence.h
    src/assets/decoders/SequenceDecoder.h
    src/assets/decoders/SequenceDecoder.cpp
    src/assets/repositories/SequenceRepository.h
    src/assets/repositories/SequenceRepository.cpp
    src/runtime/render/animation/AnimationPlayer.h
    src/runtime/render/animation/AnimationPlayer.cpp
)

for file in "${required[@]}"; do
    test -f "$file" || {
        echo "Missing: $file"
        exit 1
    }
done

for directory in animation sequence; do
    test ! -e "src/assets/$directory" || {
        echo "Already exists: src/assets/$directory"
        exit 1
    }
done

backup="$(mktemp -d "$PWD/eldoria-animation-sequence-backup.XXXXXX")"
cp -a src "$backup/"
echo "Backup: $backup"

mkdir -p src/assets/animation src/assets/sequence

mv src/assets/Animation.h \
    src/assets/animation/AnimationData.h
mv src/assets/decoders/AnimationDecoder.h \
    src/assets/animation/AnimationDecoder.h
mv src/assets/decoders/AnimationDecoder.cpp \
    src/assets/animation/AnimationDecoder.cpp
mv src/assets/repositories/AnimationRepository.h \
    src/assets/animation/AnimationPipeline.h
mv src/assets/repositories/AnimationRepository.cpp \
    src/assets/animation/AnimationPipeline.cpp
mv src/assets/AnimationFrameTable.h \
    src/assets/animation/AnimationFrameTable.h
mv src/assets/AnimationFrameTable.cpp \
    src/assets/animation/AnimationFrameTable.cpp

mv src/assets/Sequence.h \
    src/assets/sequence/SequenceData.h
mv src/assets/decoders/SequenceDecoder.h \
    src/assets/sequence/SequenceDecoder.h
mv src/assets/decoders/SequenceDecoder.cpp \
    src/assets/sequence/SequenceDecoder.cpp
mv src/assets/repositories/SequenceRepository.h \
    src/assets/sequence/SequencePipeline.h
mv src/assets/repositories/SequenceRepository.cpp \
    src/assets/sequence/SequencePipeline.cpp

python3 - <<'PY'
from pathlib import Path
import re

root = Path("src")

include_replacements = {
    '#include "Animation.h"':
        '#include "animation/AnimationData.h"',
    '#include "decoders/AnimationDecoder.h"':
        '#include "animation/AnimationDecoder.h"',
    '#include "repositories/AnimationRepository.h"':
        '#include "animation/AnimationPipeline.h"',
    '#include "AnimationFrameTable.h"':
        '#include "animation/AnimationFrameTable.h"',
    '#include "Sequence.h"':
        '#include "sequence/SequenceResource.h"',
    '#include "decoders/SequenceDecoder.h"':
        '#include "sequence/SequenceDecoder.h"',
    '#include "repositories/SequenceRepository.h"':
        '#include "sequence/SequencePipeline.h"',
}

for path in root.rglob("*"):
    if not path.is_file():
        continue

    if path.suffix not in {".h", ".cpp"} and path.name != "CMakeLists.txt":
        continue

    text = path.read_text()
    original = text

    for old, new in include_replacements.items():
        text = text.replace(old, new)

    text = re.sub(
        r"\beld::animation::AnimationFrameView\b",
        "eld::animation::AnimationFrameResource",
        text,
    )
    text = re.sub(
        r"\beld::animation::AnimationFrame\b",
        "eld::animation::AnimationFrameData",
        text,
    )
    text = re.sub(
        r"\beld::animation::Animation\b",
        "eld::animation::AnimationData",
        text,
    )
    text = re.sub(
        r"\beld::sequence::SequenceFrame\b",
        "eld::sequence::SequenceFrameData",
        text,
    )
    text = re.sub(
        r"\beld::sequence::Sequence\b",
        "eld::sequence::SequenceResource",
        text,
    )

    text = text.replace(
        "AnimationRepository",
        "AnimationPipeline",
    )
    text = text.replace(
        "animationRepository",
        "animationPipeline",
    )
    text = text.replace(
        "SequenceRepository",
        "SequencePipeline",
    )
    text = text.replace(
        "sequenceRepository",
        "sequencePipeline",
    )

    text = text.replace(
        "animationPipeline_.get(",
        "animationPipeline_.data(",
    )
    text = text.replace(
        "animations_->get(",
        "animations_->data(",
    )
    text = text.replace(
        "animations.get(",
        "animations.data(",
    )
    text = text.replace(
        "sequencePipeline_.get(",
        "sequencePipeline_.resource(",
    )
    text = text.replace(
        "sequences_->get(",
        "sequences_->resource(",
    )
    text = text.replace(
        "sequences.get(",
        "sequences.resource(",
    )

    if text != original:
        path.write_text(text)


Path("src/assets/animation/AnimationData.h").write_text(
r'''#pragma once

#include <cstdint>
#include <vector>

namespace eld::animation {

enum class TransformType : std::uint8_t {
    Pivot = 0,
    Translate = 1,
    Rotate = 2,
    Scale = 3,
    Unknown4 = 4,
    Alpha = 5
};


struct SkeletonSlot {
    TransformType type = TransformType::Pivot;
    std::vector<std::uint8_t> groups;
};


struct FrameTransform {
    std::uint16_t slot = 0;

    int x = 0;
    int y = 0;
    int z = 0;
};


struct AnimationFrameData {
    std::uint16_t id = 0;
    std::uint8_t delay = 0;

    std::vector<FrameTransform> transforms;
};


struct AnimationData {
    std::uint16_t id = 0;

    std::vector<SkeletonSlot> skeleton;
    std::vector<AnimationFrameData> frames;
};

}
''')


for name in (
    "src/assets/animation/AnimationDecoder.h",
    "src/assets/animation/AnimationDecoder.cpp",
):
    path = Path(name)
    text = path.read_text()
    text = re.sub(r"\bAnimation\b", "AnimationData", text)
    text = re.sub(
        r"\bAnimationFrame\b",
        "AnimationFrameData",
        text,
    )
    path.write_text(text)


path = Path("src/assets/sequence/SequenceData.h")
text = path.read_text()
text = re.sub(
    r"\bstruct SequenceFrame\b",
    "struct SequenceFrameData",
    text,
)
text = re.sub(
    r"\bstruct Sequence\b",
    "struct SequenceData",
    text,
)
text = re.sub(
    r"\bSequenceFrame\b",
    "SequenceFrameData",
    text,
)
path.write_text(text)


for name in (
    "src/assets/sequence/SequenceDecoder.h",
    "src/assets/sequence/SequenceDecoder.cpp",
):
    path = Path(name)
    text = path.read_text()
    text = text.replace(
        '#include "sequence/SequenceResource.h"',
        '#include "sequence/SequenceData.h"',
    )
    text = re.sub(r"\bSequence\b", "SequenceData", text)
    text = re.sub(r"\bSequenceFrame\b", "SequenceFrameData", text)
    path.write_text(text)


Path("src/assets/animation/AnimationPipeline.h").write_text(
r'''#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <vector>

#include "animation/AnimationData.h"
#include "animation/AnimationDecoder.h"
#include "cache/Cache.h"

namespace eld::animation {

class AnimationPipeline {
public:
    explicit AnimationPipeline(
        const eld::cache::Cache& cache
    );

    AnimationPipeline(const AnimationPipeline&) = delete;
    AnimationPipeline& operator=(const AnimationPipeline&) = delete;
    AnimationPipeline(AnimationPipeline&&) = delete;
    AnimationPipeline& operator=(AnimationPipeline&&) = delete;

    const AnimationData& data(
        std::uint16_t id
    ) const;

    std::optional<AnimationData> find(
        std::uint16_t id
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    static constexpr auto Index =
        eld::cache::IndexId::Animations;

    AnimationData loadData(
        std::uint16_t id
    ) const;

    eld::cache::Store store_;
    AnimationDecoder decoder_;

    mutable std::map<
        std::uint16_t,
        AnimationData
    > dataCache_;

};

}
''')


Path("src/assets/animation/AnimationPipeline.cpp").write_text(
r'''#include "animation/AnimationPipeline.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

namespace eld::animation {

AnimationPipeline::AnimationPipeline(
    const eld::cache::Cache& cache
)
    : store_(
          cache.open(Index)
      ) {
}


const AnimationData& AnimationPipeline::data(
    std::uint16_t id
) const {
    const auto existing =
        dataCache_.find(id);

    if (existing != dataCache_.end()) {
        return existing->second;
    }

    const auto inserted =
        dataCache_.emplace(
            id,
            loadData(id)
        );

    return inserted.first->second;
}


AnimationData AnimationPipeline::loadData(
    std::uint16_t id
) const {
    const eld::cache::File file =
        store_.get(id);

    try {
        AnimationData animation =
            decoder_.decode(
                file.getBytes()
            );

        animation.id = id;
        return animation;
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode animation " +
            std::to_string(id) +
            ": " +
            error.what()
        );
    }
}


std::optional<AnimationData>
AnimationPipeline::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return data(id);
}


std::vector<std::uint16_t>
AnimationPipeline::listIds() const {
    const std::vector<eld::cache::FileEntry> entries =
        store_.list();

    std::vector<std::uint16_t> ids;
    ids.reserve(entries.size());

    for (
        const eld::cache::FileEntry& entry :
        entries
    ) {
        ids.push_back(entry.fileId);
    }

    return ids;
}


bool AnimationPipeline::contains(
    std::uint16_t id
) const {
    return store_.contains(id);
}


std::size_t AnimationPipeline::count() const {
    return store_.count();
}

}
''')


Path("src/assets/animation/AnimationFrameResource.h").write_text(
r'''#pragma once

#include <span>

#include "animation/AnimationData.h"

namespace eld::animation {

struct AnimationFrameResource {
    const AnimationFrameData& frame;
    std::span<const SkeletonSlot> skeleton;
};

}
''')


Path("src/assets/animation/AnimationFrameTable.h").write_text(
r'''#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>

#include "animation/AnimationFrameResource.h"

namespace eld::animation {

class AnimationPipeline;


class AnimationFrameTable {
public:
    explicit AnimationFrameTable(
        const AnimationPipeline& animations
    );

    std::optional<AnimationFrameResource> find(
        std::uint16_t frameId
    ) const;

    bool contains(
        std::uint16_t frameId
    ) const;

private:
    struct FrameLocation {
        std::uint16_t animationId = 0;
        std::size_t frameIndex = 0;
    };

    void build() const;

    const AnimationPipeline* animations_;

    mutable std::map<
        std::uint16_t,
        FrameLocation
    > frames_;

    mutable bool built_ = false;
};

}
''')


Path("src/assets/animation/AnimationFrameTable.cpp").write_text(
r'''#include "animation/AnimationFrameTable.h"

#include <stdexcept>
#include <string>
#include <utility>

#include "animation/AnimationPipeline.h"

namespace eld::animation {

AnimationFrameTable::AnimationFrameTable(
    const AnimationPipeline& animations
)
    : animations_(&animations) {
}


void AnimationFrameTable::build() const {
    if (built_) {
        return;
    }

    std::map<
        std::uint16_t,
        FrameLocation
    > frames;

    for (
        const std::uint16_t animationId :
        animations_->listIds()
    ) {
        const AnimationData& animation =
            animations_->data(animationId);

        for (
            std::size_t frameIndex = 0;
            frameIndex < animation.frames.size();
            ++frameIndex
        ) {
            const std::uint16_t frameId =
                animation.frames[
                    frameIndex
                ].id;

            const auto [entry, inserted] =
                frames.emplace(
                    frameId,
                    FrameLocation{
                        animationId,
                        frameIndex
                    }
                );

            if (!inserted) {
                throw std::runtime_error(
                    "Duplicate animation frame " +
                    std::to_string(frameId)
                );
            }
        }
    }

    frames_ = std::move(frames);
    built_ = true;
}


std::optional<AnimationFrameResource>
AnimationFrameTable::find(
    std::uint16_t frameId
) const {
    build();

    const auto location =
        frames_.find(frameId);

    if (location == frames_.end()) {
        return std::nullopt;
    }

    const AnimationData& animation =
        animations_->data(
            location->second.animationId
        );

    if (
        location->second.frameIndex >=
        animation.frames.size()
    ) {
        return std::nullopt;
    }

    return AnimationFrameResource{
        animation.frames[
            location->second.frameIndex
        ],
        animation.skeleton
    };
}


bool AnimationFrameTable::contains(
    std::uint16_t frameId
) const {
    build();
    return frames_.contains(frameId);
}

}
''')


Path("src/assets/sequence/SequenceResource.h").write_text(
r'''#pragma once

#include <optional>
#include <span>
#include <vector>

#include "animation/AnimationFrameResource.h"
#include "sequence/SequenceData.h"

namespace eld::sequence {

struct AnimationFrameHandle {
    // Borrows immutable data. The animation pipeline must outlive this handle.
    const eld::animation::AnimationFrameData* frame = nullptr;
    std::span<const eld::animation::SkeletonSlot> skeleton;

    eld::animation::AnimationFrameResource resource() const {
        return {
            *frame,
            skeleton
        };
    }
};


struct ResolvedSequenceFrame {
    AnimationFrameHandle primary;

    std::optional<
        AnimationFrameHandle
    > secondary;
};


struct SequenceResource {
    SequenceData data;
    std::vector<ResolvedSequenceFrame> resolvedFrames;
};

}
''')


Path("src/assets/sequence/SequenceAssembler.h").write_text(
r'''#pragma once

#include "animation/AnimationFrameTable.h"
#include "sequence/SequenceData.h"
#include "sequence/SequenceResource.h"

namespace eld::sequence {

class SequenceAssembler {
public:
    SequenceAssembler() = default;

    explicit SequenceAssembler(
        const eld::animation::AnimationFrameTable& frames
    );

    SequenceResource assemble(
        SequenceData data
    ) const;

private:
    const eld::animation::AnimationFrameTable*
        frames_ = nullptr;
};

}
''')


Path("src/assets/sequence/SequenceAssembler.cpp").write_text(
r'''#include "sequence/SequenceAssembler.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace eld::sequence {

SequenceAssembler::SequenceAssembler(
    const eld::animation::AnimationFrameTable& frames
)
    : frames_(&frames) {
}


SequenceResource SequenceAssembler::assemble(
    SequenceData data
) const {
    if (frames_ == nullptr) {
        throw std::logic_error(
            "Sequence assembler requires an animation frame table"
        );
    }

    SequenceResource resource{
        .data = std::move(data)
    };

    resource.resolvedFrames.reserve(
        resource.data.frames.size()
    );

    for (
        const SequenceFrameData& frame :
        resource.data.frames
    ) {
        const auto primary =
            frames_->find(
                frame.primaryFrameId
            );

        if (!primary.has_value()) {
            throw std::runtime_error(
                "Sequence " +
                std::to_string(resource.data.id) +
                " references missing primary frame " +
                std::to_string(frame.primaryFrameId)
            );
        }

        std::optional<AnimationFrameHandle> secondary;

        if (frame.secondaryFrameId.has_value()) {
            const auto resolvedSecondary =
                frames_->find(
                    *frame.secondaryFrameId
                );

            if (!resolvedSecondary.has_value()) {
                throw std::runtime_error(
                    "Sequence " +
                    std::to_string(resource.data.id) +
                    " references missing secondary frame " +
                    std::to_string(
                        *frame.secondaryFrameId
                    )
                );
            }

            secondary.emplace(
                AnimationFrameHandle{
                    &resolvedSecondary->frame,
                    resolvedSecondary->skeleton
                }
            );
        }

        resource.resolvedFrames.push_back({
            AnimationFrameHandle{
                &primary->frame,
                primary->skeleton
            },
            secondary
        });
    }

    return resource;
}

}
''')


Path("src/assets/sequence/SequencePipeline.h").write_text(
r'''#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string_view>
#include <vector>

#include "animation/AnimationPipeline.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "sequence/SequenceAssembler.h"
#include "sequence/SequenceData.h"
#include "sequence/SequenceDecoder.h"
#include "sequence/SequenceResource.h"

namespace eld::sequence {

class SequencePipeline {
public:
    explicit SequencePipeline(
        const eld::cache::Cache& cache
    );

    SequencePipeline(
        const eld::cache::Cache& cache,
        const eld::animation::AnimationFrameTable& frames
    );

    const SequenceData& data(
        std::uint16_t id
    ) const;

    const SequenceResource& resource(
        std::uint16_t id
    ) const;

    std::optional<SequenceResource> find(
        std::uint16_t id
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    static constexpr auto Index =
        eld::cache::IndexId::Config;

    static constexpr std::uint16_t ArchiveId = 2;

    static constexpr std::string_view DataFile =
        "seq.dat";

    static constexpr std::string_view IndexFile =
        "seq.idx";

    SequenceData loadData(
        std::uint16_t id
    ) const;

    eld::archive::Archive archive_;

    SequenceDecoder decoder_;
    SequenceAssembler assembler_;

    mutable std::map<
        std::uint16_t,
        SequenceData
    > dataCache_;

    mutable std::map<
        std::uint16_t,
        SequenceResource
    > resourceCache_;
};

}
''')


Path("src/assets/sequence/SequencePipeline.cpp").write_text(
r'''#include "sequence/SequencePipeline.h"

#include <exception>
#include <stdexcept>
#include <string>

#include "binary/ByteReader.h"

namespace eld::sequence {

SequencePipeline::SequencePipeline(
    const eld::cache::Cache& cache
)
    : archive_(
          eld::archive::load(
              cache.open(Index),
              ArchiveId
          )
      ) {
}


SequencePipeline::SequencePipeline(
    const eld::cache::Cache& cache,
    const eld::animation::AnimationFrameTable& frames
)
    : SequencePipeline(cache) {
    assembler_ = SequenceAssembler(frames);
}


const SequenceData& SequencePipeline::data(
    std::uint16_t id
) const {
    const auto existing =
        dataCache_.find(id);

    if (existing != dataCache_.end()) {
        return existing->second;
    }

    const auto inserted =
        dataCache_.emplace(
            id,
            loadData(id)
        );

    return inserted.first->second;
}


SequenceData SequencePipeline::loadData(
    std::uint16_t id
) const {
    const eld::archive::ArchiveFile& dataFile =
        archive_.get(DataFile);

    const eld::archive::ArchiveFile& indexFile =
        archive_.get(IndexFile);

    eld::binary::ByteReader dataReader(
        dataFile.payload
    );

    eld::binary::ByteReader indexReader(
        indexFile.payload
    );

    const auto dataCount = dataReader.readU16();
    const auto indexCount = indexReader.readU16();

    if (dataCount != indexCount) {
        throw std::runtime_error(
            "sequence data/index count mismatch"
        );
    }

    if (id >= dataCount) {
        throw std::out_of_range(
            "sequence does not exist"
        );
    }

    for (
        std::uint16_t currentId = 0;
        currentId < id;
        ++currentId
    ) {
        const auto size = indexReader.readU16();

        if (!dataReader.canRead(size)) {
            throw std::runtime_error(
                "sequence asset exceeds data file"
            );
        }

        dataReader.readBytes(size);
    }

    const auto size = indexReader.readU16();

    if (!dataReader.canRead(size)) {
        throw std::runtime_error(
            "sequence asset exceeds data file"
        );
    }

    const std::vector<std::uint8_t> payload =
        dataReader.readBytes(size);

    try {
        SequenceData sequence =
            decoder_.decode(payload);

        sequence.id = id;
        return sequence;
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode sequence " +
            std::to_string(id) +
            ": " +
            error.what()
        );
    }
}


const SequenceResource&
SequencePipeline::resource(
    std::uint16_t id
) const {
    const auto existing =
        resourceCache_.find(id);

    if (existing != resourceCache_.end()) {
        return existing->second;
    }

    const auto inserted =
        resourceCache_.emplace(
            id,
            assembler_.assemble(
                data(id)
            )
        );

    return inserted.first->second;
}


std::optional<SequenceResource>
SequencePipeline::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return resource(id);
}


std::vector<std::uint16_t>
SequencePipeline::listIds() const {
    const std::size_t assetCount = count();

    std::vector<std::uint16_t> ids;
    ids.reserve(assetCount);

    for (
        std::size_t id = 0;
        id < assetCount;
        ++id
    ) {
        ids.push_back(
            static_cast<std::uint16_t>(id)
        );
    }

    return ids;
}


bool SequencePipeline::contains(
    std::uint16_t id
) const {
    return id < count();
}


std::size_t SequencePipeline::count() const {
    const eld::archive::ArchiveFile& dataFile =
        archive_.get(DataFile);

    const eld::archive::ArchiveFile& indexFile =
        archive_.get(IndexFile);

    eld::binary::ByteReader dataReader(
        dataFile.payload
    );

    eld::binary::ByteReader indexReader(
        indexFile.payload
    );

    const auto dataCount = dataReader.readU16();
    const auto indexCount = indexReader.readU16();

    if (dataCount != indexCount) {
        throw std::runtime_error(
            "sequence data/index count mismatch"
        );
    }

    return dataCount;
}

}
''')


Path("src/runtime/render/animation/AnimationPlayer.h").write_text(
r'''#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "sequence/SequenceResource.h"

namespace eld::render {

class AnimationPlayer {
public:
    static constexpr std::uint32_t
        ClientCycleMilliseconds = 20;

    AnimationPlayer() = default;

    void setSequence(
        const eld::sequence::SequenceResource& sequence
    );

    void clear();

    const eld::sequence::SequenceResource*
    sequence() const;

    const eld::sequence::SequenceFrameData*
    currentSequenceFrame() const;

    std::optional<eld::animation::AnimationFrameResource>
    currentFrame() const;

    std::size_t frameIndex() const;
    std::size_t frameCount() const;

    std::uint32_t
    currentFrameDurationMilliseconds() const;

    bool update(
        std::uint64_t deltaMilliseconds
    );

    bool stepForward();
    bool stepBackward();

    void play();
    void pause();
    void setPlaying(bool playing);
    void restart();

    bool isPlaying() const;

    void setLooping(bool looping);
    bool isLooping() const;
    bool looping() const { return looping_; }

    void setSpeed(float speed);
    float speed() const;

private:
    std::size_t loopStart() const;
    bool advanceFrame();

    std::optional<
        eld::sequence::SequenceResource
    > sequence_;

    std::size_t frameIndex_ = 0;
    double elapsedMilliseconds_ = 0.0;

    bool playing_ = true;
    bool looping_ = true;
    float speed_ = 1.0f;
};

}
''')


player_source = Path(
    "src/runtime/render/animation/AnimationPlayer.cpp"
).read_text()

player_source = player_source.replace(
    "    sequence_ = sequence;",
    "    sequence_.emplace(sequence);",
)

constructor = re.compile(
    r"AnimationPlayer::AnimationPlayer\(.*?\n\}\n\n",
    re.DOTALL,
)
player_source = constructor.sub("", player_source, count=1)
player_source = player_source.replace(
    "const eld::sequence::SequenceResource*\n"
    "AnimationPlayer::sequence() const",
    "const eld::sequence::SequenceResource*\n"
    "AnimationPlayer::sequence() const",
)
player_source = player_source.replace(
    "const eld::sequence::SequenceFrameData*\n"
    "AnimationPlayer::currentSequenceFrame() const",
    "const eld::sequence::SequenceFrameData*\n"
    "AnimationPlayer::currentSequenceFrame() const",
)

old_current = re.compile(
    r"std::optional<eld::animation::AnimationFrameResource>\n"
    r"AnimationPlayer::currentFrame\(\) const \{.*?\n\}\n",
    re.DOTALL,
)
new_current = r'''std::optional<eld::animation::AnimationFrameResource>
AnimationPlayer::currentFrame() const {
    if (
        !sequence_.has_value() ||
        frameIndex_ >= sequence_->resolvedFrames.size()
    ) {
        return std::nullopt;
    }

    return sequence_->resolvedFrames[
        frameIndex_
    ].primary.resource();
}
'''
player_source, count = old_current.subn(
    new_current,
    player_source,
    count=1,
)

if count != 1:
    raise SystemExit(
        "Could not update AnimationPlayer::currentFrame"
    )

Path(
    "src/runtime/render/animation/AnimationPlayer.cpp"
).write_text(player_source)


for path in root.rglob("*"):
    if not path.is_file() or path.suffix not in {".h", ".cpp"}:
        continue

    text = path.read_text()
    original = text

    if "sequence" not in path.parts or path.parts[1] != "assets":
        text = re.sub(
            r"(\bsequence_?(?:->|\.))(frames|frameStep|interleaveOrder|stretches|priority|shieldItemId|weaponItemId|maximumLoops|animatingPrecedence|walkingPrecedence|replayMode|packedData|id)\b",
            r"\1data.\2",
            text,
        )

    text = re.sub(
        r"animationPlayer_\(\s*"
        r"animationFrameTable_\s*\)",
        "animationPlayer_()",
        text,
    )
    text = re.sub(
        r"std::make_unique<eld::render::AnimationPlayer>\(\s*"
        r"animationFrameTable_\s*\)",
        "std::make_unique<eld::render::AnimationPlayer>()",
        text,
    )
    text = re.sub(
        r"eld::render::AnimationPlayer\s+(\w+)\(\s*"
        r"animationFrameTable_\s*\);",
        r"eld::render::AnimationPlayer \1;",
        text,
    )

    if text != original:
        path.write_text(text)


path = Path("src/apps/elforge/explorer/CacheExplorer.cpp")
text = path.read_text()
text = re.sub(
    r"sequencePipeline_\(\s*cache_\s*\)",
    "sequencePipeline_(cache_, animationFrameTable_)",
    text,
)
text = text.replace(
    "                animationFrameTable_,\n",
    "",
)
path.write_text(text)


path = Path("src/assets/AssetManager.cpp")
text = path.read_text()
text = re.sub(
    r"sequences\(\s*cache\s*\)",
    "sequences(cache, animationFrames)",
    text,
)
text = re.sub(r"animations\(\s*cache\s*\),", "animations(cache),\n      animationFrames(animations),", text)
path.write_text(text)

path = Path("src/assets/AssetManager.h")
text = path.read_text()
text = text.replace('#include "animation/AnimationPipeline.h"', '#include "animation/AnimationPipeline.h"\n#include "animation/AnimationFrameTable.h"')
text = re.sub(r"(eld::animation::AnimationPipeline\s+animations;)", r"\1\n    eld::animation::AnimationFrameTable animationFrames;", text)
path.write_text(text)

for name in (
    "src/apps/elforge/inspection/AnimationInspector.h",
    "src/apps/elforge/inspection/AnimationInspector.cpp",
):
    path = Path(name)
    text = path.read_text()
    text = text.replace(
        '#include "animation/AnimationFrameTable.h"\n',
        "",
    )
    text = re.sub(
        r"\s*const eld::animation::AnimationFrameTable& frames,",
        "",
        text,
    )
    text = re.sub(
        r"\s*const eld::animation::AnimationFrameTable\* frames_ = nullptr;",
        "",
        text,
    )
    text = re.sub(
        r"\s*frames_\(&frames\),",
        "",
        text,
    )
    text = re.sub(
        r"sequenceDurationMilliseconds\(\s*"
        r"const eld::animation::AnimationFrameTable& frames,\s*"
        r"const eld::sequence::SequenceResource& sequence\s*\)",
        "sequenceDurationMilliseconds(\n"
        "    const eld::sequence::SequenceResource& sequence\n"
        ")",
        text,
    )
    text = re.sub(
        r"eld::render::AnimationPlayer player\(\s*frames\s*\);",
        "eld::render::AnimationPlayer player;",
        text,
    )
    text = re.sub(
        r"sequenceDurationMilliseconds\(\s*\*frames_,\s*",
        "sequenceDurationMilliseconds(",
        text,
    )
    path.write_text(text)


path = Path("src/assets/CMakeLists.txt")
text = path.read_text()

old_animation = (
    "    AnimationFrameTable.cpp\n"
    "    content/animation/AnimationPresentationCatalog.cpp\n"
    "    decoders/AnimationDecoder.cpp\n"
    "    repositories/AnimationPipeline.cpp\n"
)
new_animation = (
    "    animation/AnimationFrameTable.cpp\n"
    "    animation/AnimationDecoder.cpp\n"
    "    animation/AnimationPipeline.cpp\n"
    "    content/animation/AnimationPresentationCatalog.cpp\n"
)

text = text.replace(old_animation, new_animation)
text = text.replace(
    "    decoders/SequenceDecoder.cpp\n"
    "    repositories/SequencePipeline.cpp\n",
    "    sequence/SequenceDecoder.cpp\n"
    "    sequence/SequenceAssembler.cpp\n"
    "    sequence/SequencePipeline.cpp\n",
)
path.write_text(text)
PY

echo
echo "===== ARCHITECTURE CHECK ====="
if rg -n \
    'AnimationFrameView|AnimationRepository|SequenceRepository|eld::animation::Animation\b|eld::sequence::Sequence\b|#include "Animation.h"|#include "Sequence.h"|repositories/(Animation|Sequence)Pipeline|decoders/(Animation|Sequence)Decoder' \
    src/assets/CMakeLists.txt \
    src/assets \
    src/runtime \
    src/apps; then
    echo "Old animation/sequence references remain"
    exit 1
fi

echo
echo "===== DOMAIN FILES ====="
find src/assets/animation src/assets/sequence \
    -maxdepth 1 -type f | sort

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
