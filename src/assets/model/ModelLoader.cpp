#include "model/ModelLoader.h"

#include <exception>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace eld::model {

ModelLoader::ModelLoader(
    const eld::cache::Cache& cache
)
    : store_(
          cache.open(CacheIndex)
      ) {
}


const ModelData& ModelLoader::get(
    std::uint16_t id
) const {
    const auto existing =
        modelCache_.find(id);

    if (existing != modelCache_.end()) {
        return existing->second;
    }

    const auto inserted =
        modelCache_.emplace(
            id,
            load(id)
        );

    return inserted.first->second;
}


const ModelData* ModelLoader::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return nullptr;
    }

    return &get(id);
}


std::vector<std::uint16_t>
ModelLoader::listIds() const {
    const std::vector<eld::cache::FileEntry> entries =
        store_.list();

    std::vector<std::uint16_t> ids;
    ids.reserve(entries.size());

    for (const eld::cache::FileEntry& entry : entries) {
        ids.push_back(
            entry.fileId
        );
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


ModelData ModelLoader::load(
    std::uint16_t id
) const {
    const eld::cache::File file =
        store_.get(id);

    const std::vector<std::uint8_t> bytes =
        file.getBytes();

    // === MODEL FORMAT PROBE ===
    if (
        id == 0 ||
        id == 1 ||
        id == 2214 ||
        id == 2215 ||
        id == 4873
    ) {
        std::cout
            << "\nmodel raw "
            << id
            << ": bytes="
            << bytes.size()
            << "\n  tail: ";

        const std::size_t start =
            bytes.size() > 32
                ? bytes.size() - 32
                : 0;

        std::cout
            << std::hex
            << std::setfill('0');

        for (
            std::size_t i = start;
            i < bytes.size();
            ++i
        ) {
            std::cout
                << std::setw(2)
                << static_cast<unsigned>(
                    bytes[i]
                )
                << ' ';
        }

        std::cout
            << std::dec
            << std::setfill(' ')
            << "\n";
    }

    try {
        ModelData decoded =
            decoder_.decode(
                bytes
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

}
