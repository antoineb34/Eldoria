#include "CacheExplorerMode.h"

#include <imgui.h>
#include <array>
#include <fstream>
#include <iostream>
#include <string>

#include "../../../core/cache/ArchiveDecoder.h"
#include "../../../core/cache/ArchiveFileTable.h"
#include "../../../core/cache/KnownArchives.h"

namespace rf::tool {

CacheExplorerMode::CacheExplorerMode()
: configCache_(
"cache/main_file_cache.dat",
"cache/main_file_cache.idx0"
),
configLoader_(
"cache/main_file_cache.dat",
"cache/main_file_cache.idx0"
)
{
}

bool CacheExplorerMode::initialize() {

buildRawCacheTree();

std::vector<char> textureIndex =
    configLoader_.loadFileFromArchive(
        6,
        "index.dat"
    );

std::cout
    << "\ntexture archive index.dat size: "
    << textureIndex.size()
    << " bytes\n";

return true;

}

void CacheExplorerMode::handleEvent(
const SDL_Event& event
) {
(void)event;
}

void CacheExplorerMode::update() {
}

void CacheExplorerMode::render(
SDL_Renderer* renderer,
rf::render::DepthBuffer& depthBuffer,
int windowWidth,
int windowHeight
) {
(void)depthBuffer;
(void)windowWidth;
(void)windowHeight;

SDL_SetRenderDrawColor(
    renderer,
    28,
    56,
    60,
    255
);

SDL_RenderClear(
    renderer
);

}

void CacheExplorerMode::renderUi() {

ImGui::Text("Cache Explorer");
ImGui::Separator();

ImGui::BeginChild(
    "CacheTreePanel",
    ImVec2(320.0f, 0.0f),
    true
);

ImGui::Text("Raw Cache");
ImGui::Separator();

renderTreeNode(
    rootNode_
);

ImGui::EndChild();

ImGui::SameLine();

ImGui::BeginChild(
    "CacheDetailsPanel",
    ImVec2(0.0f, 0.0f),
    true
);

renderInspector();

ImGui::EndChild();

}

void CacheExplorerMode::buildRawCacheTree() {

rootNode_ = {
    "Raw Cache",
    "RuneForge cache filesystem",
    CacheNodeType::Root
};

CacheTreeNode idx0Node {
    "idx0",
    "configs / media / archives",
    CacheNodeType::Index
};

for (uint32_t archiveId = 0; archiveId < 20; archiveId++) {

    rf::cache::CacheArchive archive =
        configCache_.readArchive(
            archiveId
        );

    if (
        archive.entry.size == 0 ||
        archive.payload.empty()
    ) {
        continue;
    }

    rf::cache::DecodedArchive decoded =
        rf::cache::decodeArchiveContainer(
            archive.payload
        );

    rf::cache::ArchiveFileTable table =
        rf::cache::readArchiveFileTable(
            decoded.payload
        );

    CacheTreeNode archiveNode {
        "archive " + std::to_string(archiveId),
        "files: " + std::to_string(table.fileCount),
        CacheNodeType::Archive
    };

    archiveNode.archiveId = archiveId;
    archiveNode.compressedSize = decoded.compressedSize;
    archiveNode.uncompressedSize = decoded.uncompressedSize;

    for (int fileIndex = 0;
         fileIndex < static_cast<int>(table.files.size());
         fileIndex++) {

        const auto& file =
            table.files[fileIndex];

        CacheTreeNode fileNode =
            makeFileNode(
                archiveId,
                fileIndex,
                file
            );

        archiveNode.children.push_back(
            fileNode
        );
    }

    idx0Node.children.push_back(
        archiveNode
    );
}

rootNode_.children.push_back(
    idx0Node
);

}

CacheTreeNode CacheExplorerMode::makeFileNode(
uint32_t archiveId,
int fileIndex,
const rf::cache::ArchiveFileEntry& file
) const {
std::string knownName =
std::string(
rf::cache::findKnownArchiveName(
file.hash
)
);

std::string label =
    knownName.empty()
        ? "file " + std::to_string(fileIndex)
        : knownName;

CacheTreeNode fileNode {
    label,
    "hash: " + std::to_string(file.hash) +
    " size: " + std::to_string(file.uncompressedSize),
    CacheNodeType::File
};

fileNode.archiveId = archiveId;
fileNode.fileIndex = fileIndex;
fileNode.hash = file.hash;
fileNode.compressedSize = file.compressedSize;
fileNode.uncompressedSize = file.uncompressedSize;
fileNode.offset = file.offset;

return fileNode;

}

void CacheExplorerMode::renderTreeNode(
const CacheTreeNode& node
) {
ImGuiTreeNodeFlags flags =
ImGuiTreeNodeFlags_OpenOnArrow |
ImGuiTreeNodeFlags_SpanAvailWidth;

if (node.children.empty()) {
    flags |=
        ImGuiTreeNodeFlags_Leaf |
        ImGuiTreeNodeFlags_NoTreePushOnOpen;
}

bool open =
    ImGui::TreeNodeEx(
        node.label.c_str(),
        flags
    );

if (ImGui::IsItemClicked()) {

    selectedNode_ = node;
    hasSelection_ = true;

    if (
        selectedNode_.type == CacheNodeType::Archive &&
        selectedNode_.archiveId == 6
    ) {
        std::vector<char> indexFile =
            configLoader_.loadFileFromArchive(
                6,
                "index.dat"
            );

        std::cout
            << "\n===== texture archive index.dat =====\n"
            << "size: "
            << indexFile.size()
            << " bytes\n\n";

        for (
            size_t row = 0;
            row < indexFile.size() && row < 256;
            row += 16
        ) {
            std::printf(
                "%04zX: ",
                row
            );

            for (
                size_t col = 0;
                col < 16;
                col++
            ) {
                size_t index =
                    row + col;

                if (index >= indexFile.size()) {
                    break;
                }

                std::printf(
                    "%02X ",
                    static_cast<unsigned char>(
                        indexFile[index]
                    )
                );
            }

            std::printf("\n");
        }

        auto readU16 =
            [&](size_t offset) -> uint16_t {

                if (offset + 1 >= indexFile.size()) {
                    return 0;
                }

                return
                    (static_cast<uint8_t>(indexFile[offset]) << 8) |
                    static_cast<uint8_t>(indexFile[offset + 1]);
            };

        std::cout
            << "\n===== interpreted header =====\n"
            << "u16[0]: "
            << readU16(0)
            << "\n"
            << "u16[2]: "
            << readU16(2)
            << "\n"
            << "u8[4]: "
            << static_cast<int>(
                static_cast<uint8_t>(indexFile[4])
            )
            << "\n";

            std::cout
                << "\n===== palette guess =====\n";

            uint8_t paletteCount =
                static_cast<uint8_t>(
                    indexFile[4]
                );

            size_t paletteOffset = 5;

            for (
                int i = 1;
                i < paletteCount;
                i++
            ) {
                uint8_t r =
                    static_cast<uint8_t>(
                        indexFile[paletteOffset++]
                    );

                uint8_t g =
                    static_cast<uint8_t>(
                        indexFile[paletteOffset++]
                    );

                uint8_t b =
                    static_cast<uint8_t>(
                        indexFile[paletteOffset++]
                    );

                std::printf(
                    "palette[%d] = rgb(%u, %u, %u)\n",
                    i,
                    r,
                    g,
                    b
                );
            }

            size_t metaOffset =
                5 + ((paletteCount - 1) * 3);

            std::cout
                << "\n===== first texture metadata guess =====\n"
                << "meta offset: "
                << metaOffset
                << "\n"
                << "x offset: "
                << static_cast<int>(static_cast<uint8_t>(indexFile[metaOffset]))
                << "\n"
                << "y offset: "
                << static_cast<int>(static_cast<uint8_t>(indexFile[metaOffset + 1]))
                << "\n"
                << "width: "
                << readU16(metaOffset + 2)
                << "\n"
                << "height: "
                << readU16(metaOffset + 4)
                << "\n"
                << "type: "
                << static_cast<int>(static_cast<uint8_t>(indexFile[metaOffset + 6]))
                << "\n";
    }

    if (
        selectedNode_.type ==
        CacheNodeType::File
    ) {
        std::vector<char> fileData =
            configLoader_.loadFileByIndexFromArchive(
                selectedNode_.archiveId,
                selectedNode_.fileIndex
            );

        std::cout
            << "\n===== file payload =====\n"
            << "file: "
            << selectedNode_.label
            << "\n"
            << "size: "
            << fileData.size()
            << " bytes\n";

        size_t histogram[256] = {};

        uint8_t minValue = 255;
        uint8_t maxValue = 0;

        for (char byte : fileData) {

            uint8_t value =
                static_cast<uint8_t>(byte);

            histogram[value]++;

            if (value < minValue) {
                minValue = value;
            }

            if (value > maxValue) {
                maxValue = value;
            }
        }

        std::cout
            << "min byte: "
            << static_cast<int>(minValue)
            << "\n"
            << "max byte: "
            << static_cast<int>(maxValue)
            << "\n\n";

        std::cout
            << "first 64 bytes:\n";

        for (
            size_t i = 0;
            i < 64 && i < fileData.size();
            i++
        ) {
            std::printf(
                "%02X ",
                static_cast<uint8_t>(
                    fileData[i]
                )
            );
        }

        std::printf("\n\n");

        std::cout
            << "used byte values:\n";

        for (int i = 0; i < 256; i++) {

            if (histogram[i] > 0) {

                std::printf(
                    "%02X (%zu)\n",
                    i,
                    histogram[i]
                );
            }
        }

        if (
            selectedNode_.archiveId == 6 &&
            selectedNode_.fileIndex == 0 &&
            fileData.size() >= 16386
        ) {
            std::vector<char> indexFile =
                configLoader_.loadFileFromArchive(
                    6,
                    "index.dat"
                );

            std::vector<std::array<uint8_t, 3>> palette;

            palette.push_back({0, 0, 0});

            uint8_t paletteCount =
                static_cast<uint8_t>(
                    indexFile[4]
                );

            size_t paletteOffset = 5;

            for (int i = 1; i < paletteCount; i++) {
                uint8_t r =
                    static_cast<uint8_t>(
                        indexFile[paletteOffset++]
                    );

                uint8_t g =
                    static_cast<uint8_t>(
                        indexFile[paletteOffset++]
                    );

                uint8_t b =
                    static_cast<uint8_t>(
                        indexFile[paletteOffset++]
                    );

                palette.push_back({r, g, b});
            }

            std::ofstream out(
                "texture_0.ppm",
                std::ios::binary
            );

            out
                << "P6\n"
                << "128 128\n"
                << "255\n";

            size_t pixelOffset = 2;

            for (size_t i = 0; i < 128 * 128; i++) {
                uint8_t paletteIndex =
                    static_cast<uint8_t>(
                        fileData[pixelOffset + i]
                    );

                if (paletteIndex >= palette.size()) {
                    paletteIndex = 0;
                }

                const auto& color =
                    palette[paletteIndex];

                out.put(static_cast<char>(color[0]));
                out.put(static_cast<char>(color[1]));
                out.put(static_cast<char>(color[2]));
            }

            std::cout
                << "\nexported texture_0.ppm\n";
        }
    }
}

if (
    ImGui::IsItemHovered() &&
    !node.detail.empty()
) {
    ImGui::SetTooltip(
        "%s",
        node.detail.c_str()
    );
}

if (
    open &&
    !node.children.empty()
) {
    for (const CacheTreeNode& child : node.children) {
        renderTreeNode(
            child
        );
    }

    ImGui::TreePop();
}

}

void CacheExplorerMode::renderInspector() {

ImGui::Text("Inspector");
ImGui::Separator();

if (!hasSelection_) {
    ImGui::Text("Select an archive or file from the tree.");
    return;
}

ImGui::Text(
    "Label: %s",
    selectedNode_.label.c_str()
);

ImGui::Text(
    "Detail: %s",
    selectedNode_.detail.c_str()
);

ImGui::Text(
    "Node type: %d",
    static_cast<int>(selectedNode_.type)
);

ImGui::Separator();

ImGui::Text(
    "Archive ID: %u",
    selectedNode_.archiveId
);

if (
    selectedNode_.type ==
    CacheNodeType::Archive
) {
    if (
        selectedNode_.archiveId == 6
    ) {
        ImGui::Separator();

        ImGui::Text(
            "Texture archive detected"
        );

        std::vector<char> indexFile =
            configLoader_.loadFileFromArchive(
                6,
                "index.dat"
            );

        ImGui::Text(
            "index.dat size: %zu bytes",
            indexFile.size()
        );

        if (
            !indexFile.empty()
        ) {
            ImGui::Separator();

            ImGui::Text("First 32 bytes:");

            ImGui::Separator();

            ImGui::Text("Hex dump");

            for (
                size_t row = 0;
                row < indexFile.size() && row < 128;
                row += 16
            ) {
                std::string line;

                char offsetBuffer[16];

                std::snprintf(
                    offsetBuffer,
                    sizeof(offsetBuffer),
                    "%04zX: ",
                    row
                );

                line += offsetBuffer;

                for (
                    size_t col = 0;
                    col < 16;
                    col++
                ) {
                    size_t index =
                        row + col;

                    if (index >= indexFile.size()) {
                        break;
                    }

                    char byteBuffer[8];

                    std::snprintf(
                        byteBuffer,
                        sizeof(byteBuffer),
                        "%02X ",
                        static_cast<uint8_t>(
                            indexFile[index]
                        )
                    );

                    line += byteBuffer;
                }

                ImGui::Text(
                    "%s",
                    line.c_str()
                );
            }
        }
    }
}

if (selectedNode_.type != CacheNodeType::File) {
    return;
}

ImGui::Text(
    "File index: %d",
    selectedNode_.fileIndex
);

ImGui::Text(
    "Hash: %u",
    selectedNode_.hash
);

ImGui::Text(
    "Compressed size: %u",
    selectedNode_.compressedSize
);

ImGui::Text(
    "Uncompressed size: %u",
    selectedNode_.uncompressedSize
);

ImGui::Text(
    "Offset: %u",
    selectedNode_.offset
);

}

}
