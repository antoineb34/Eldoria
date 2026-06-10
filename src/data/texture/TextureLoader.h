#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>

#include "TextureAsset.h"
#include "TextureBuilder.h"
#include "TextureFile.h"
#include "TextureFileReader.h"

#include "cache/Cache.h"

namespace eld::texture {

class TextureLoader {
public:
    explicit TextureLoader(
        const eld::cache::Cache& cache
    );

    std::optional<TextureAsset> load(
        std::uint32_t id
    );

private:
    std::optional<TextureFile> getTextureFile(
        std::uint32_t id
    ) const;

private:
    const eld::cache::Cache& cache_;

    TextureFileReader fileReader_;
    TextureBuilder textureBuilder_;

    std::unordered_map<
        std::uint32_t,
        TextureAsset
    > textureCache_;
};

}
