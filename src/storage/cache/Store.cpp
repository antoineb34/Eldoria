#include "Store.h"

#include <fstream>
#include <limits>
#include <stdexcept>
#include <utility>

namespace eld::cache {

Store::Store(
    std::filesystem::path dataPath,
    Index index
)
    : dataPath_(std::move(dataPath)),
      index_(std::move(index)) {
}

File Store::get(
    std::uint16_t fileId
) const {
    return reader_.readFile(
        dataPath_,
        index_,
        fileId
    );
}

std::optional<FileEntry> Store::find(
    std::uint16_t fileId
) const {
    const std::optional<IndexEntry> entry =
        reader_.findEntry(
            index_,
            fileId
        );

    if (!entry.has_value()) {
        return std::nullopt;
    }

    return FileEntry{
        .fileId = fileId,
        .indexEntry = *entry
    };
}

std::vector<FileEntry> Store::list() const {
    std::ifstream stream(
        index_.path,
        std::ios::binary | std::ios::ate
    );

    if (!stream.is_open()) {
        throw std::runtime_error(
            "Failed to open cache index: " +
            index_.path.string()
        );
    }

    const std::streamoff size =
        stream.tellg();

    if (size < 0) {
        throw std::runtime_error(
            "Failed to determine cache index size"
        );
    }

    if (
        size %
        static_cast<std::streamoff>(
            IndexEntry::TotalSize
        ) != 0
    ) {
        throw std::runtime_error(
            "Cache index has an invalid size: " +
            index_.path.string()
        );
    }

    const std::uint64_t entryCount =
        static_cast<std::uint64_t>(
            size
        ) /
        IndexEntry::TotalSize;

    if (
        entryCount >
        static_cast<std::uint64_t>(
            std::numeric_limits<std::uint16_t>::max()
        ) + 1
    ) {
        throw std::runtime_error(
            "Cache index contains unsupported file IDs"
        );
    }

    std::vector<FileEntry> entries;

    entries.reserve(
        static_cast<std::size_t>(
            entryCount
        )
    );

    for (
        std::uint64_t fileId = 0;
        fileId < entryCount;
        ++fileId
    ) {
        const std::optional<FileEntry> entry =
            find(
                static_cast<std::uint16_t>(
                    fileId
                )
            );

        if (entry.has_value()) {
            entries.push_back(
                *entry
            );
        }
    }

    return entries;
}

bool Store::contains(
    std::uint16_t fileId
) const {
    return find(
        fileId
    ).has_value();
}

std::size_t Store::count() const {
    return list().size();
}

std::uint16_t Store::create(
    FileData data
) {
    return writer_.createFile(
        dataPath_,
        index_,
        std::move(data)
    );
}

void Store::update(
    std::uint16_t fileId,
    FileData data
) {
    writer_.updateFile(
        dataPath_,
        index_,
        fileId,
        std::move(data)
    );
}

}
