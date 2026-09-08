#include "model/ModelLoader.h"

#include <exception>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>

namespace eld::model {

ModelLoader::ModelLoader(
    const eld::cache::Cache& cache,
    const eld::texture::TextureLoader& textures
)
    : store_(
          cache.open(Index)
      ),
      textures_(textures) {
}


const ModelData& ModelLoader::data(
    std::uint16_t id
) const {
    const auto existing =
        dataCache_.find(id);

    if (existing != dataCache_.end()) {
        return existing->second;
    }

    const auto insertion =
        dataCache_.emplace(
            id,
            loadData(id)
        );

    return insertion.first->second;
}


ModelData ModelLoader::loadData(
    std::uint16_t id
) const {
    const eld::cache::File file =
        store_.get(id);

    try {
        ModelData decoded =
            decoder_.decode(
                file.getBytes()
            );

        decoded.id = id;
        return decoded;
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode model " +
            std::to_string(id) +
            ": " +
            error.what()
        );
    }
}


std::optional<ModelData> ModelLoader::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return data(id);
}


const ModelResource& ModelLoader::resource(
    std::uint16_t id
) const {
    const auto existing =
        resourceCache_.find(id);

    if (existing != resourceCache_.end()) {
        return existing->second;
    }

    const auto insertion =
        resourceCache_.emplace(
            id,
            assembleResource(id)
        );

    return insertion.first->second;
}


ModelResource ModelLoader::assembleResource(
    std::uint16_t id
) const {
    const ModelData& modelData =
        data(id);

    std::map<
        std::uint16_t,
        eld::texture::TextureResource
    > textures;

    for (
        const Face& face :
        modelData.faces
    ) {
        if (!face.textureId.has_value()) {
            continue;
        }

        const std::uint16_t textureId =
            *face.textureId;

        if (textures.contains(textureId)) {
            continue;
        }

        textures.emplace(
            textureId,
            textures_.resource(textureId)
        );
    }

    return assembler_.assemble(
        modelData,
        std::move(textures)
    );
}


std::vector<std::uint16_t>
ModelLoader::listIds() const {
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


bool ModelLoader::contains(
    std::uint16_t id
) const {
    return store_.contains(id);
}


std::size_t ModelLoader::count() const {
    return store_.count();
}

}
