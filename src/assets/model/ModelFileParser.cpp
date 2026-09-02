#include "ModelFileParser.h"

#include <cstddef>
#include <cstdint>
#include <exception>

#include "binary/ByteReader.h"

namespace eld::model {

namespace {

constexpr std::size_t ModelFooterSize = 18;

constexpr std::uint32_t OptionalChunkPresent = 1;
constexpr std::uint32_t PerFacePriority = 255;

constexpr std::size_t FaceColorSize = 2;
constexpr std::size_t TextureTriangleSize = 6;


ModelUnit makeUnit(
    std::size_t index,
    std::size_t offset,
    std::size_t size
) {
    return ModelUnit{
        index,
        offset,
        size
    };
}

template <typename TUnit>
void setSectionSpan(
    ModelSection<TUnit>& section,
    int offset,
    std::size_t size
) {
    section.offset =
        static_cast<std::size_t>(
            offset
        );

    section.size =
        size;
}

template <typename TUnit>
void setSectionSpan(
    ModelSection<TUnit>& section,
    int offset,
    std::uint32_t size
) {
    setSectionSpan(
        section,
        offset,
        static_cast<std::size_t>(
            size
        )
    );
}

VertexDeltaUnit readVertexDeltaUnit(
    eld::binary::ByteReader& reader,
    std::size_t index
) {
    const std::size_t start =
        reader.position();

    const int delta =
        reader.readSignedSmart();

    const std::size_t end =
        reader.position();

    return VertexDeltaUnit{
        makeUnit(
            index,
            start,
            end - start
        ),
        delta
    };
}

TriangleIndexDeltaUnit readTriangleIndexDeltaUnit(
    eld::binary::ByteReader& reader,
    std::size_t index
) {
    const std::size_t start =
        reader.position();

    const int delta =
        reader.readSignedSmart();

    const std::size_t end =
        reader.position();

    return TriangleIndexDeltaUnit{
        makeUnit(
            index,
            start,
            end - start
        ),
        delta
    };
}

}

std::optional<ModelFile> ModelFileParser::parse(
    const std::vector<std::uint8_t>& payload
) const {
    if (!validatePayload(payload)) {
        return std::nullopt;
    }

    try {
        const ModelFooter footer =
            readFooter(
                payload
            );

        const ModelLayout layout =
            calculateLayout(
                footer
            );

        if (
            !validateLayout(
                payload,
                footer,
                layout
            )
        ) {
            return std::nullopt;
        }

        ModelFile file{
            .payload = payload,
            .footer = footer,
            .layout = layout,
            .sections = {}
        };

        file.sections =
            parseSections(
                file
            );

        return file;
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

bool ModelFileParser::validatePayload(
    const std::vector<std::uint8_t>& payload
) const {
    return payload.size() >= ModelFooterSize;
}

ModelFooter ModelFileParser::readFooter(
    const std::vector<std::uint8_t>& payload
) const {
    eld::binary::ByteReader reader(
        payload
    );

    reader.setPosition(
        payload.size() - ModelFooterSize
    );

    ModelFooter footer{};

    footer.vertexCount =
        reader.readU16();

    footer.triangleCount =
        reader.readU16();

    footer.textureTriangleCount =
        reader.readU8();

    footer.textureFlag =
        reader.readU8();

    footer.priorityFlag =
        reader.readU8();

    footer.alphaFlag =
        reader.readU8();

    footer.triangleSkinFlag =
        reader.readU8();

    footer.vertexSkinFlag =
        reader.readU8();

    footer.xDataLength =
        reader.readU16();

    footer.yDataLength =
        reader.readU16();

    footer.zDataLength =
        reader.readU16();

    footer.triangleDataLength =
        reader.readU16();

    return footer;
}

ModelLayout ModelFileParser::calculateLayout(
    const ModelFooter& footer
) const {
    ModelLayout layout{};

    int offset = 0;

    layout.vertexFlagsOffset =
        offset;

    offset +=
        static_cast<int>(
            footer.vertexCount
        );

    layout.triangleTypesOffset =
        offset;

    offset +=
        static_cast<int>(
            footer.triangleCount
        );

    layout.trianglePrioritiesOffset =
        offset;

    if (
        footer.priorityFlag ==
        PerFacePriority
    ) {
        offset +=
            static_cast<int>(
                footer.triangleCount
            );
    }

    layout.triangleSkinsOffset =
        offset;

    if (
        footer.triangleSkinFlag ==
        OptionalChunkPresent
    ) {
        offset +=
            static_cast<int>(
                footer.triangleCount
            );
    }

    layout.texturePointersOffset =
        offset;

    if (
        footer.textureFlag ==
        OptionalChunkPresent
    ) {
        offset +=
            static_cast<int>(
                footer.triangleCount
            );
    }

    layout.vertexSkinsOffset =
        offset;

    if (
        footer.vertexSkinFlag ==
        OptionalChunkPresent
    ) {
        offset +=
            static_cast<int>(
                footer.vertexCount
            );
    }

    layout.triangleAlphasOffset =
        offset;

    if (
        footer.alphaFlag ==
        OptionalChunkPresent
    ) {
        offset +=
            static_cast<int>(
                footer.triangleCount
            );
    }

    layout.triangleDataOffset =
        offset;

    offset +=
        static_cast<int>(
            footer.triangleDataLength
        );

    layout.triangleColorsOffset =
        offset;

    offset +=
        static_cast<int>(
            footer.triangleCount *
            FaceColorSize
        );

    layout.textureDataOffset =
        offset;

    offset +=
        static_cast<int>(
            footer.textureTriangleCount *
            TextureTriangleSize
        );

    layout.xDataOffset =
        offset;

    offset +=
        static_cast<int>(
            footer.xDataLength
        );

    layout.yDataOffset =
        offset;

    offset +=
        static_cast<int>(
            footer.yDataLength
        );

    layout.zDataOffset =
        offset;

    return layout;
}

bool ModelFileParser::validateLayout(
    const std::vector<std::uint8_t>& payload,
    const ModelFooter& footer,
    const ModelLayout& layout
) const {
    const std::size_t encodedDataSize =
        payload.size() - ModelFooterSize;

    const std::size_t usedDataSize =
        static_cast<std::size_t>(
            layout.zDataOffset
        ) +
        static_cast<std::size_t>(
            footer.zDataLength
        );

    return usedDataSize == encodedDataSize;
}

ModelSections ModelFileParser::parseSections(
    const ModelFile& file
) const {
    ModelSections sections{};

    sections.vertexFlags =
        parseVertexFlags(
            file
        );

    sections.vertexSkins =
        parseVertexSkins(
            file
        );

    sections.xData =
        parseVertexDeltas(
            file,
            file.layout.xDataOffset,
            file.footer.xDataLength
        );

    sections.yData =
        parseVertexDeltas(
            file,
            file.layout.yDataOffset,
            file.footer.yDataLength
        );

    sections.zData =
        parseVertexDeltas(
            file,
            file.layout.zDataOffset,
            file.footer.zDataLength
        );

    sections.triangleTypes =
        parseTriangleTypes(
            file
        );

    sections.trianglePriorities =
        parseTrianglePriorities(
            file
        );

    sections.triangleSkins =
        parseTriangleSkins(
            file
        );

    sections.texturePointers =
        parseTexturePointers(
            file
        );

    sections.triangleAlphas =
        parseTriangleAlphas(
            file
        );

    sections.triangleColors =
        parseTriangleColors(
            file
        );

    sections.triangleData =
        parseTriangleData(
            file
        );

    sections.textureData =
        parseTextureData(
            file
        );

    return sections;
}

ModelSection<VertexFlagUnit> ModelFileParser::parseVertexFlags(
    const ModelFile& file
) const {
    ModelSection<VertexFlagUnit> section{};

    setSectionSpan(
        section,
        file.layout.vertexFlagsOffset,
        file.footer.vertexCount
    );

    section.units.reserve(
        file.footer.vertexCount
    );

    for (
        std::uint32_t i = 0;
        i < file.footer.vertexCount;
        i++
    ) {
        const std::size_t offset =
            section.offset +
            static_cast<std::size_t>(
                i
            );

        section.units.push_back(
            VertexFlagUnit{
                makeUnit(
                    i,
                    offset,
                    1
                ),
                file.payload.at(
                    offset
                )
            }
        );
    }

    return section;
}

ModelSection<VertexSkinUnit> ModelFileParser::parseVertexSkins(
    const ModelFile& file
) const {
    ModelSection<VertexSkinUnit> section{};

    const std::size_t size =
        file.footer.vertexSkinFlag == OptionalChunkPresent
            ? static_cast<std::size_t>(
                file.footer.vertexCount
            )
            : 0;

    setSectionSpan(
        section,
        file.layout.vertexSkinsOffset,
        size
    );

    if (!section.present()) {
        return section;
    }

    section.units.reserve(
        file.footer.vertexCount
    );

    for (
        std::uint32_t i = 0;
        i < file.footer.vertexCount;
        i++
    ) {
        const std::size_t offset =
            section.offset +
            static_cast<std::size_t>(
                i
            );

        section.units.push_back(
            VertexSkinUnit{
                makeUnit(
                    i,
                    offset,
                    1
                ),
                file.payload.at(
                    offset
                )
            }
        );
    }

    return section;
}

ModelSection<VertexDeltaUnit> ModelFileParser::parseVertexDeltas(
    const ModelFile& file,
    int offset,
    std::uint32_t dataLength
) const {
    ModelSection<VertexDeltaUnit> section{};

    setSectionSpan(
        section,
        offset,
        dataLength
    );

    if (!section.present()) {
        return section;
    }

    eld::binary::ByteReader reader(
        file.payload
    );

    reader.setPosition(
        section.offset
    );

    while (
        reader.position() <
        section.end()
    ) {
        section.units.push_back(
            readVertexDeltaUnit(
                reader,
                section.units.size()
            )
        );
    }

    return section;
}

ModelSection<TriangleTypeUnit> ModelFileParser::parseTriangleTypes(
    const ModelFile& file
) const {
    ModelSection<TriangleTypeUnit> section{};

    setSectionSpan(
        section,
        file.layout.triangleTypesOffset,
        file.footer.triangleCount
    );

    section.units.reserve(
        file.footer.triangleCount
    );

    for (
        std::uint32_t i = 0;
        i < file.footer.triangleCount;
        i++
    ) {
        const std::size_t offset =
            section.offset +
            static_cast<std::size_t>(
                i
            );

        section.units.push_back(
            TriangleTypeUnit{
                makeUnit(
                    i,
                    offset,
                    1
                ),
                file.payload.at(
                    offset
                )
            }
        );
    }

    return section;
}

ModelSection<TrianglePriorityUnit> ModelFileParser::parseTrianglePriorities(
    const ModelFile& file
) const {
    ModelSection<TrianglePriorityUnit> section{};

    const std::size_t size =
        file.footer.priorityFlag == PerFacePriority
            ? static_cast<std::size_t>(
                file.footer.triangleCount
            )
            : 0;

    setSectionSpan(
        section,
        file.layout.trianglePrioritiesOffset,
        size
    );

    if (!section.present()) {
        return section;
    }

    section.units.reserve(
        file.footer.triangleCount
    );

    for (
        std::uint32_t i = 0;
        i < file.footer.triangleCount;
        i++
    ) {
        const std::size_t offset =
            section.offset +
            static_cast<std::size_t>(
                i
            );

        section.units.push_back(
            TrianglePriorityUnit{
                makeUnit(
                    i,
                    offset,
                    1
                ),
                file.payload.at(
                    offset
                )
            }
        );
    }

    return section;
}

ModelSection<TriangleSkinUnit> ModelFileParser::parseTriangleSkins(
    const ModelFile& file
) const {
    ModelSection<TriangleSkinUnit> section{};

    const std::size_t size =
        file.footer.triangleSkinFlag == OptionalChunkPresent
            ? static_cast<std::size_t>(
                file.footer.triangleCount
            )
            : 0;

    setSectionSpan(
        section,
        file.layout.triangleSkinsOffset,
        size
    );

    if (!section.present()) {
        return section;
    }

    section.units.reserve(
        file.footer.triangleCount
    );

    for (
        std::uint32_t i = 0;
        i < file.footer.triangleCount;
        i++
    ) {
        const std::size_t offset =
            section.offset +
            static_cast<std::size_t>(
                i
            );

        section.units.push_back(
            TriangleSkinUnit{
                makeUnit(
                    i,
                    offset,
                    1
                ),
                file.payload.at(
                    offset
                )
            }
        );
    }

    return section;
}

ModelSection<TexturePointerUnit> ModelFileParser::parseTexturePointers(
    const ModelFile& file
) const {
    ModelSection<TexturePointerUnit> section{};

    const std::size_t size =
        file.footer.textureFlag == OptionalChunkPresent
            ? static_cast<std::size_t>(
                file.footer.triangleCount
            )
            : 0;

    setSectionSpan(
        section,
        file.layout.texturePointersOffset,
        size
    );

    if (!section.present()) {
        return section;
    }

    section.units.reserve(
        file.footer.triangleCount
    );

    for (
        std::uint32_t i = 0;
        i < file.footer.triangleCount;
        i++
    ) {
        const std::size_t offset =
            section.offset +
            static_cast<std::size_t>(
                i
            );

        section.units.push_back(
            TexturePointerUnit{
                makeUnit(
                    i,
                    offset,
                    1
                ),
                file.payload.at(
                    offset
                )
            }
        );
    }

    return section;
}

ModelSection<TriangleAlphaUnit> ModelFileParser::parseTriangleAlphas(
    const ModelFile& file
) const {
    ModelSection<TriangleAlphaUnit> section{};

    const std::size_t size =
        file.footer.alphaFlag == OptionalChunkPresent
            ? static_cast<std::size_t>(
                file.footer.triangleCount
            )
            : 0;

    setSectionSpan(
        section,
        file.layout.triangleAlphasOffset,
        size
    );

    if (!section.present()) {
        return section;
    }

    section.units.reserve(
        file.footer.triangleCount
    );

    for (
        std::uint32_t i = 0;
        i < file.footer.triangleCount;
        i++
    ) {
        const std::size_t offset =
            section.offset +
            static_cast<std::size_t>(
                i
            );

        section.units.push_back(
            TriangleAlphaUnit{
                makeUnit(
                    i,
                    offset,
                    1
                ),
                file.payload.at(
                    offset
                )
            }
        );
    }

    return section;
}

ModelSection<TriangleColorUnit> ModelFileParser::parseTriangleColors(
    const ModelFile& file
) const {
    ModelSection<TriangleColorUnit> section{};

    setSectionSpan(
        section,
        file.layout.triangleColorsOffset,
        static_cast<std::size_t>(
            file.footer.triangleCount
        ) *
        FaceColorSize
    );

    if (!section.present()) {
        return section;
    }

    eld::binary::ByteReader reader(
        file.payload
    );

    reader.setPosition(
        section.offset
    );

    section.units.reserve(
        file.footer.triangleCount
    );

    for (
        std::uint32_t i = 0;
        i < file.footer.triangleCount;
        i++
    ) {
        const std::size_t start =
            reader.position();

        const std::uint16_t color =
            reader.readU16();

        const std::size_t end =
            reader.position();

        section.units.push_back(
            TriangleColorUnit{
                makeUnit(
                    i,
                    start,
                    end - start
                ),
                color
            }
        );
    }

    return section;
}

ModelSection<TriangleIndexDeltaUnit> ModelFileParser::parseTriangleData(
    const ModelFile& file
) const {
    ModelSection<TriangleIndexDeltaUnit> section{};

    setSectionSpan(
        section,
        file.layout.triangleDataOffset,
        file.footer.triangleDataLength
    );

    if (!section.present()) {
        return section;
    }

    eld::binary::ByteReader reader(
        file.payload
    );

    reader.setPosition(
        section.offset
    );

    while (
        reader.position() <
        section.end()
    ) {
        section.units.push_back(
            readTriangleIndexDeltaUnit(
                reader,
                section.units.size()
            )
        );
    }

    return section;
}

ModelSection<TextureTriangleUnit> ModelFileParser::parseTextureData(
    const ModelFile& file
) const {
    ModelSection<TextureTriangleUnit> section{};

    setSectionSpan(
        section,
        file.layout.textureDataOffset,
        static_cast<std::size_t>(
            file.footer.textureTriangleCount
        ) *
        TextureTriangleSize
    );

    if (!section.present()) {
        return section;
    }

    eld::binary::ByteReader reader(
        file.payload
    );

    reader.setPosition(
        section.offset
    );

    section.units.reserve(
        file.footer.textureTriangleCount
    );

    for (
        std::uint32_t i = 0;
        i < file.footer.textureTriangleCount;
        i++
    ) {
        const std::size_t start =
            reader.position();

        const std::uint16_t originVertex =
            reader.readU16();

        const std::uint16_t uVertex =
            reader.readU16();

        const std::uint16_t vVertex =
            reader.readU16();

        const std::size_t end =
            reader.position();

        section.units.push_back(
            TextureTriangleUnit{
                makeUnit(
                    i,
                    start,
                    end - start
                ),
                originVertex,
                uVertex,
                vVertex
            }
        );
    }

    return section;
}

}
