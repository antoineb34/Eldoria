#include "MessageAnimationDefinitionParser.h"

#include <exception>

#include "binary/ByteReader.h"

namespace eld::definition {

std::optional<MessageAnimationDefinition>
MessageAnimationDefinitionParser::parse(
    const DefinitionRecord& record
) const {
    try {
        eld::binary::ByteReader reader(
            record.bytes
        );

        MessageAnimationDefinition definition;
        definition.id = record.id;

        if (
            reader.readU8() != 0 ||
            !reader.atEnd()
        ) {
            return std::nullopt;
        }

        return definition;
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

}
