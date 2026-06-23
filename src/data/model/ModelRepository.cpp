#include "ModelRepository.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace eld::model {

ModelRepository::ModelRepository(
    eld::cache::Store store
)
    : store_(std::move(store)) {
}

Model ModelRepository::get(
    std::uint16_t id
) const {
    ModelFile file =
        getFile(id);

    ModelSourceMap sourceMap;

    ModelMesh mesh =
        decoder_.decode(
            file,
            sourceMap
        );

    return Model{
        .id = id,
        .file = std::move(file),
        .mesh = std::move(mesh),
        .sourceMap = std::move(sourceMap)
    };
}

std::optional<Model> ModelRepository::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return get(id);
}

ModelFile ModelRepository::getFile(
    std::uint16_t id
) const {
    eld::cache::File cacheFile =
        store_.get(id);

    std::optional<ModelFile> file =
        parser_.parse(
            cacheFile.getBytes()
        );

    if (!file.has_value()) {
        throw std::runtime_error(
            "Failed to parse model " +
            std::to_string(id)
        );
    }

    return std::move(*file);
}

ModelMesh ModelRepository::getMesh(
    std::uint16_t id
) const {
    const ModelFile file =
        getFile(id);

    return decoder_.decode(
        file
    );
}

std::vector<std::uint16_t>
ModelRepository::listIds() const {
    const std::vector<eld::cache::FileEntry> entries =
        store_.list();

    std::vector<std::uint16_t> ids;

    ids.reserve(
        entries.size()
    );

    for (const eld::cache::FileEntry& entry : entries) {
        ids.push_back(
            entry.fileId
        );
    }

    return ids;
}

std::vector<std::uint16_t>
ModelRepository::filterIds(
    const ModelPredicate& predicate
) const {
    const std::vector<std::uint16_t> ids =
        listIds();

    std::vector<std::uint16_t> matchingIds;

    for (const std::uint16_t id : ids) {
        const Model model =
            get(id);

        if (predicate(model)) {
            matchingIds.push_back(id);
        }
    }

    return matchingIds;
}

bool ModelRepository::contains(
    std::uint16_t id
) const {
    return store_.contains(id);
}

std::size_t ModelRepository::count() const {
    return store_.count();
}

std::size_t ModelRepository::count(
    const ModelPredicate& predicate
) const {
    const std::vector<std::uint16_t> ids =
        listIds();

    std::size_t matchingCount = 0;

    for (const std::uint16_t id : ids) {
        if (predicate(get(id))) {
            matchingCount++;
        }
    }

    return matchingCount;
}

}
