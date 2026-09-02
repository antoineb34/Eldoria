#include "Archive.h"

#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "ArchiveHashes.h"

namespace eld::archive {

Archive::Archive(
    std::vector<std::uint8_t> payload,
    ArchiveHeader header,
    ArchiveSections sections,
    std::vector<ArchiveFile> files
)
    : payload_(std::move(payload)),
      header_(header),
      sections_(sections),
      files_(std::move(files)) {
}

const std::vector<std::uint8_t>&
Archive::getPayload() const {
    return payload_;
}

const ArchiveHeader& Archive::getHeader() const {
    return header_;
}

const ArchiveSections& Archive::getSections() const {
    return sections_;
}

const ArchiveFile& Archive::get(
    std::uint16_t id
) const {
    const ArchiveFile* file =
        find(id);

    if (file == nullptr) {
        throw std::out_of_range(
            "Archive file does not exist"
        );
    }

    return *file;
}

const ArchiveFile& Archive::get(
    std::string_view name
) const {
    const ArchiveFile* file =
        find(name);

    if (file == nullptr) {
        throw std::out_of_range(
            "Archive file does not exist"
        );
    }

    return *file;
}

const ArchiveFile* Archive::find(
    std::uint16_t id
) const {
    for (const ArchiveFile& file : files_) {
        if (file.id == id) {
            return &file;
        }
    }

    return nullptr;
}

const ArchiveFile* Archive::find(
    std::string_view name
) const {
    const std::uint32_t nameHash =
        hashName(name);

    for (const ArchiveFile& file : files_) {
        if (file.nameHash == nameHash) {
            return &file;
        }
    }

    return nullptr;
}

const std::vector<ArchiveFile>&
Archive::list() const {
    return files_;
}

bool Archive::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}

bool Archive::contains(
    std::string_view name
) const {
    return find(name) != nullptr;
}

std::size_t Archive::count() const {
    return files_.size();
}

}
