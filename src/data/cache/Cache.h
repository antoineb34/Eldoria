#pragma once

#include <filesystem>
#include <optional>
#include <vector>

namespace rf::cache {

enum class CacheIndex {
    Config = 0,
    Model = 1,
    Animation = 2,
    Midi = 3,
    Map = 4
};

struct CacheIndexEntry {
    int size = 0;
    int firstSector = 0;
};

struct CacheFile {
    int id = 0;
    CacheIndex index = CacheIndex::Config;
    CacheIndexEntry entry;
    std::vector<unsigned char> payload;
};

class Cache {
public:
    Cache();
    explicit Cache(std::filesystem::path rootPath);

    bool isValid() const;

    std::optional<CacheFile> readFile(
        CacheIndex index,
        int fileId
    ) const;

    std::vector<CacheFile> listFiles(
        CacheIndex index
    ) const;

private:
    std::optional<CacheIndexEntry> readIndexEntry(
        CacheIndex index,
        int fileId
    ) const;

    std::filesystem::path indexPath(
        CacheIndex index
    ) const;

private:
    std::filesystem::path rootPath_;
};

}
