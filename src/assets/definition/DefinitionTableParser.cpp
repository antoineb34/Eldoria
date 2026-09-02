#include "DefinitionTableParser.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <utility>
#include <vector>

#include "binary/ByteReader.h"

namespace eld::definition {

std::optional<DefinitionTable>
DefinitionTableParser::parse(
    const std::vector<std::uint8_t>& dataPayload,
    const std::vector<std::uint8_t>& indexPayload
) const {
    try {
        eld::binary::ByteReader dataReader(
            dataPayload
        );

        eld::binary::ByteReader indexReader(
            indexPayload
        );

        const std::uint16_t dataCount =
            dataReader.readU16();

        const std::uint16_t indexCount =
            indexReader.readU16();

        if (dataCount != indexCount) {
            return std::nullopt;
        }

        std::vector<DefinitionRecord> records;

        records.reserve(
            dataCount
        );

        for (
            std::uint16_t id = 0;
            id < dataCount;
            id++
        ) {
            const std::size_t indexOffset =
                indexReader.position();

            const std::uint16_t size =
                indexReader.readU16();

            if (!dataReader.canRead(size)) {
                return std::nullopt;
            }

            const std::size_t dataOffset =
                dataReader.position();

            records.push_back(
                DefinitionRecord{
                    .id = id,
                    .indexOffset = indexOffset,
                    .dataOffset = dataOffset,
                    .size = size,
                    .bytes =
                        dataReader.readBytes(
                            size
                        )
                }
            );
        }

        if (
            !dataReader.atEnd() ||
            !indexReader.atEnd()
        ) {
            return std::nullopt;
        }

        return DefinitionTable(
            dataPayload,
            indexPayload,
            std::move(records)
        );
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

}
