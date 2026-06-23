#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

#include "File.h"
#include "Index.h"
#include "Reader.h"
#include "Writer.h"

namespace eld::cache {

class Store {
public:
    Store(
        std::filesystem::path dataPath,
        Index index
    );

    File get(
        std::uint16_t fileId
    ) const;

    std::optional<FileEntry> find(
        std::uint16_t fileId
    ) const;

    std::vector<FileEntry> list() const;

    bool contains(
        std::uint16_t fileId
    ) const;

    std::size_t count() const;

    std::uint16_t create(
        FileData data
    );

    void update(
        std::uint16_t fileId,
        FileData data
    );

private:
    std::filesystem::path dataPath_;
    Index index_;

    Reader reader_;
    Writer writer_;
};

}
