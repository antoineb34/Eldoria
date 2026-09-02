#include "ModelRepository.h"

#include <exception>
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
    eld::cache::File file =
        store_.get(
            id
        );

    try {
        Model model =
            decoder_.decode(
                file.getBytes()
            );

        model.id =
            id;

        return model;
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


std::optional<Model> ModelRepository::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return get(
        id
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

    for (
        const eld::cache::FileEntry& entry :
        entries
    ) {
        ids.push_back(
            entry.fileId
        );
    }

    return ids;
}


bool ModelRepository::contains(
    std::uint16_t id
) const {
    return store_.contains(
        id
    );
}


std::size_t ModelRepository::count() const {
    return store_.count();
}


}
