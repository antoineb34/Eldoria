#include "CacheExplorerMode.h"

#include <imgui.h>

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
        node.children.empty()
            ? ImGuiTreeNodeFlags_Leaf |
              ImGuiTreeNodeFlags_NoTreePushOnOpen
            : 0;

    bool open =
        ImGui::TreeNodeEx(
            node.label.c_str(),
            flags
        );

    if (ImGui::IsItemClicked()) {
        selectedNode_ = node;
        hasSelection_ = true;
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

    ImGui::Separator();

    ImGui::Text(
        "Archive ID: %u",
        selectedNode_.archiveId
    );

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
