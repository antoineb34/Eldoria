#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace eld::definition {

struct DefinitionRecord {
    std::uint16_t id = 0;

    std::size_t indexOffset = 0;
    std::size_t dataOffset = 0;

    std::uint16_t size = 0;
    std::vector<std::uint8_t> bytes;
};

class DefinitionTable {
public:
    DefinitionTable(
        std::vector<std::uint8_t> dataPayload,
        std::vector<std::uint8_t> indexPayload,
        std::vector<DefinitionRecord> records
    )
        : dataPayload_(std::move(dataPayload)),
          indexPayload_(std::move(indexPayload)),
          records_(std::move(records)) {
    }

    const DefinitionRecord& get(
        std::uint16_t id
    ) const {
        const DefinitionRecord* record =
            find(id);

        if (record == nullptr) {
            throw std::out_of_range(
                "Definition record does not exist"
            );
        }

        return *record;
    }

    const DefinitionRecord* find(
        std::uint16_t id
    ) const {
        if (
            static_cast<std::size_t>(id) >=
            records_.size()
        ) {
            return nullptr;
        }

        const DefinitionRecord& record =
            records_[id];

        return record.id == id
            ? &record
            : nullptr;
    }

    const std::vector<DefinitionRecord>&
    list() const {
        return records_;
    }

    bool contains(
        std::uint16_t id
    ) const {
        return find(id) != nullptr;
    }

    std::size_t count() const {
        return records_.size();
    }

    const std::vector<std::uint8_t>&
    getDataPayload() const {
        return dataPayload_;
    }

    const std::vector<std::uint8_t>&
    getIndexPayload() const {
        return indexPayload_;
    }

private:
    std::vector<std::uint8_t> dataPayload_;
    std::vector<std::uint8_t> indexPayload_;
    std::vector<DefinitionRecord> records_;
};

}
