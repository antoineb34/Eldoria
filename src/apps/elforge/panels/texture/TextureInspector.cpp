#include "TextureInspector.h"

namespace eldoria::apps::elforge {

TextureInspector::TextureInspector() = default;

void TextureInspector::loadFromCache(
    rf::texture::TextureLoader& loader,
    std::uint32_t id
) {
    lastRequestedId = id;

    auto asset = loader.load(id);

    if (asset.has_value()) {
        result.loaded = true;
        result.message = "Texture " + std::to_string(id) + " loaded.";
        result.asset = std::move(asset);
    } else {
        result.loaded = false;
        result.message = "Texture " + std::to_string(id) + " not found in cache.";
        result.asset = std::nullopt;
    }
}

}
