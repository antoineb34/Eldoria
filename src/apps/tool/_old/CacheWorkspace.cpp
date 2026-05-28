#include "CacheWorkspace.h"

#include <imgui.h>
#include <iostream>
#include <string>

#include "../../../core/cache/ArchiveDecoder.h"
#include "../../../core/cache/ArchiveFileTable.h"
#include "../../../core/cache/KnownArchives.h"

#include "../../../core/codecs/texture/TextureDecoder.h"
#include "../../../core/codecs/texture/TextureIndex.h"

namespace rf::tool {

CacheWorkspace::CacheWorkspace()
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

bool CacheWorkspace::initialize() {
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

void CacheWorkspace::handleEvent(
    const SDL_Event& event
) {
    (void)event;
}

void CacheWorkspace::update() {
}

void CacheWorkspace::render(
    SDL_Renderer* renderer,
    rf::render::DepthBuffer& depthBuffer,
    int viewportX,
    int viewportY,
    int viewportWidth,
    int viewportHeight
) {
    (void)depthBuffer;

    SDL_Rect clip {
        viewportX,
        viewportY,
        viewportWidth,
        viewportHeight
    };

    SDL_SetRenderClipRect(
        renderer,
        &clip
    );

    SDL_SetRenderDrawColor(
        renderer,
        28,
        56,
        60,
        255
    );

    SDL_FRect background {
        static_cast<float>(viewportX),
        static_cast<float>(viewportY),
        static_cast<float>(viewportWidth),
        static_cast<float>(viewportHeight)
    };

    SDL_RenderFillRect(
        renderer,
        &background
    );

    uploadPreviewTexture(
        renderer
    );

    SDL_SetRenderClipRect(
        renderer,
        nullptr
    );
}

void CacheWorkspace::renderUi() {
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

void CacheWorkspace::buildRawCacheTree() {
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

        for (
            int fileIndex = 0;
            fileIndex < static_cast<int>(table.files.size());
            fileIndex++
        ) {
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

CacheTreeNode CacheWorkspace::makeFileNode(
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

void CacheWorkspace::renderTreeNode(
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

        if (selectedNode_.type == CacheNodeType::Archive) {
            inspectSelectedArchive();
        }

        if (selectedNode_.type == CacheNodeType::File) {
            inspectSelectedFile();
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

void CacheWorkspace::renderInspector() {
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

    if (selectedNode_.type == CacheNodeType::File) {
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

    if (previewTexture_ != nullptr) {
        ImGui::Separator();

        ImGui::Text("Texture Preview");

        ImGui::Image(
            reinterpret_cast<ImTextureID>(previewTexture_),
            ImVec2(256.0f, 256.0f)
        );
    }
}

void CacheWorkspace::uploadPreviewTexture(
    SDL_Renderer* renderer
) {
    if (previewTextureData_.pixels.empty()) {
        return;
    }

    if (previewTexture_ != nullptr) {
        return;
    }

    previewTexture_ = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STATIC,
        previewTextureData_.width,
        previewTextureData_.height
    );

    if (previewTexture_ == nullptr) {
        std::cout
            << "Failed to create SDL texture: "
            << SDL_GetError()
            << "\n";

        return;
    }

    SDL_UpdateTexture(
        previewTexture_,
        nullptr,
        previewTextureData_.pixels.data(),
        previewTextureData_.width * 4
    );
}

std::vector<std::uint8_t> CacheWorkspace::toBytes(
    const std::vector<char>& data
) const {
    std::vector<std::uint8_t> bytes;
    bytes.reserve(data.size());

    for (char value : data) {
        bytes.push_back(
            static_cast<std::uint8_t>(value)
        );
    }

    return bytes;
}

void CacheWorkspace::inspectSelectedArchive() {
    if (selectedNode_.archiveId != 6) {
        return;
    }

    std::vector<char> rawIndex =
        configLoader_.loadFileFromArchive(
            6,
            "index.dat"
        );

    rf::texture::TextureIndex textureIndex =
        rf::texture::TextureIndexParser::parse(
            toBytes(rawIndex)
        );

    std::cout
        << "\n===== texture archive =====\n"
        << "index.dat size: "
        << rawIndex.size()
        << " bytes\n"
        << "canvas: "
        << textureIndex.canvasWidth
        << "x"
        << textureIndex.canvasHeight
        << "\n"
        << "palette entries: "
        << textureIndex.palette.size()
        << "\n"
        << "texture metadata entries: "
        << textureIndex.textures.size()
        << "\n";

    if (!textureIndex.textures.empty()) {
        const auto& meta =
            textureIndex.textures[0];

        std::cout
            << "texture 0: "
            << meta.width
            << "x"
            << meta.height
            << " type="
            << meta.type
            << "\n";
    }

    for (
        std::size_t i = 0;
        i < textureIndex.textures.size();
        i++
    ) {
        const auto& meta =
            textureIndex.textures[i];

        std::cout
            << "texture metadata ["
            << i
            << "] "
            << "xOff="
            << meta.xOffset
            << " yOff="
            << meta.yOffset
            << " width="
            << meta.width
            << " height="
            << meta.height
            << " type="
            << meta.type
            << "\n";
    }
}

void CacheWorkspace::inspectSelectedFile() {
    if (previewTexture_ != nullptr) {
        SDL_DestroyTexture(
            previewTexture_
        );

        previewTexture_ = nullptr;
    }

    previewTextureData_ = {};

    std::vector<char> rawFile =
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
        << rawFile.size()
        << " bytes\n";

    if (
        selectedNode_.archiveId != 6 ||
        selectedNode_.label == "index.dat"
    ) {
        return;
    }

    std::vector<char> rawIndex =
        configLoader_.loadFileFromArchive(
            6,
            "index.dat"
        );

    rf::texture::TextureIndex textureIndex =
        rf::texture::TextureIndexParser::parse(
            toBytes(rawIndex)
        );

    auto bytes =
        toBytes(rawFile);

    std::uint16_t metaIndex =
        (bytes[0] << 8) |
        bytes[1];

    std::cout
        << "metadata index: "
        << metaIndex
        << "\n";

    previewTextureData_ =
        rf::texture::TextureDecoder::decode(
            textureIndex,
            toBytes(rawFile),
            selectedNode_.fileIndex
        );

    std::cout
        << "\n===== decoded texture =====\n"
        << "size: "
        << previewTextureData_.width
        << "x"
        << previewTextureData_.height
        << "\n"
        << "rgba bytes: "
        << previewTextureData_.pixels.size()
        << "\n";
}

}
