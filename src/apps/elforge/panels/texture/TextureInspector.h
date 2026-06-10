#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "texture/TextureLoader.h"
#include "texture/TextureAsset.h"

namespace eldoria::apps::elforge {

struct TextureLoadResult {
    bool loaded = false;
    std::string message;
    std::optional<rf::texture::TextureAsset> asset;
};

class TextureInspector {
public:
    TextureInspector();

    void loadFromCache(
        rf::texture::TextureLoader& loader,
        std::uint32_t id
    );

    TextureLoadResult result;
    std::uint32_t lastRequestedId = 0;
};

}
