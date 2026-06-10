#include "CacheInspectorPanel.h"

#include <array>
#include <cstdint>
#include <string>

#include <imgui.h>

#include "CacheState.h"

namespace eldoria::apps::elforge {

namespace {

const char* getNodeTypeName(
    CacheTreeNodeType type
) {
    switch (type) {
        case CacheTreeNodeType::Root:
            return "Root";

        case CacheTreeNodeType::Index:
            return "Index";

        case CacheTreeNodeType::File:
            return "File";

        case CacheTreeNodeType::Model:
            return "Model";

        case CacheTreeNodeType::Texture:
            return "Texture";
    }

    return "Unknown";
}

const char* getIndexName(
    int indexId
) {
    switch (static_cast<rf::cache::CacheIndex>(indexId)) {
        case rf::cache::CacheIndex::Config:
            return "Config";

        case rf::cache::CacheIndex::Model:
            return "Models";

        case rf::cache::CacheIndex::Animation:
            return "Animations";

        case rf::cache::CacheIndex::Midi:
            return "Midi";

        case rf::cache::CacheIndex::Map:
            return "Maps";
    }

    return "Unknown";
}

const char* getCompressionTypeName(
    rf::compression::CompressionType type
) {
    switch (type) {
        case rf::compression::CompressionType::Gzip:
            return "Gzip";

        case rf::compression::CompressionType::Bzip2:
            return "Bzip2";

        case rf::compression::CompressionType::Unknown:
            return "Unknown";
    }

    return "Unknown";
}

void renderIdLine(
    const char* label,
    int value
) {
    if (value < 0) {
        ImGui::Text(
            "%s: N/A",
            label
        );
        return;
    }

    ImGui::Text(
        "%s: %d",
        label,
        value
    );
}

std::string buildModelDebugText(
    const CacheState& state,
    const rf::model::ModelAsset& model
) {
    std::array<int, 12> priorityCounts {};
    std::array<int, 4> renderTypeCounts {};

    int alphaFaces = 0;
    int texturedFaces = 0;

    for (const rf::model::Face& face : model.faces) {
        if (face.priority < static_cast<int>(priorityCounts.size())) {
            priorityCounts[face.priority]++;
        }

        if (face.renderType < static_cast<int>(renderTypeCounts.size())) {
            renderTypeCounts[face.renderType]++;
        }

        if (face.alpha > 0) {
            alphaFaces++;
        }

        if (face.texturePointer >= 0) {
            texturedFaces++;
        }
    }

    std::string debug;

    debug += "Model Debug\n";
    debug += "===========\n";

    debug += "Selected: " +
        state.selection.label +
        "\n";

    debug += "Type: " +
        std::string(getNodeTypeName(state.selection.type)) +
        "\n";

    debug += "Index: " +
        std::to_string(state.selection.indexId) +
        "\n";

    debug += "Archive: " +
        std::to_string(state.selection.archiveId) +
        "\n";

    debug += "File: " +
        std::to_string(state.selection.fileId) +
        "\n\n";

    debug += "Vertices: " +
        std::to_string(model.vertices.size()) +
        "\n";

    debug += "Faces: " +
        std::to_string(model.faces.size()) +
        "\n";

    debug += "Texture UV mappings: " +
        std::to_string(model.textureUVMappings.size()) +
        "\n";

    debug += "Loaded textures: " +
        std::to_string(model.textures.size()) +
        "\n";

    debug += "Alpha faces: " +
        std::to_string(alphaFaces) +
        "\n";

    debug += "Textured faces: " +
        std::to_string(texturedFaces) +
        "\n\n";

    debug += "Priority Buckets\n";
    debug += "----------------\n";

    for (size_t i = 0; i < priorityCounts.size(); i++) {
        debug += "Priority " +
            std::to_string(i) +
            ": " +
            std::to_string(priorityCounts[i]) +
            "\n";
    }

    debug += "\nRender Type Buckets\n";
    debug += "-------------------\n";

    for (size_t i = 0; i < renderTypeCounts.size(); i++) {
        debug += "RenderType " +
            std::to_string(i) +
            ": " +
            std::to_string(renderTypeCounts[i]) +
            "\n";
    }

    return debug;
}

}

void CacheInspectorPanel::render(
    CacheState& state,
    float width,
    float height
) {
    ImGui::BeginChild(
        "CacheInspectorPanel",
        ImVec2(width, height),
        true
    );

    ImGui::TextUnformatted("INSPECTOR");
    ImGui::Separator();

    ImGui::Text(
        "Selected node: %s",
        state.selection.label.c_str()
    );

    ImGui::Spacing();

    ImGui::Text(
        "Type: %s",
        getNodeTypeName(
            state.selection.type
        )
    );

    if (state.selection.indexId >= 0) {
        ImGui::Text(
            "Selected index: %d (%s)",
            state.selection.indexId,
            getIndexName(state.selection.indexId)
        );
    }
    else {
        ImGui::TextUnformatted("Selected index: N/A");
    }

    renderIdLine(
        "Archive id",
        state.selection.archiveId
    );

    renderIdLine(
        "File id",
        state.selection.fileId
    );

    if (state.activeCacheFileDetails) {
        const rf::cache::CacheFileDetails& details =
            *state.activeCacheFileDetails;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("CACHE FILE");

        ImGui::Text(
            "Payload size: %zu bytes",
            details.payloadSize
        );

        ImGui::Text(
            "Cache entry size: %d bytes",
            details.cacheEntrySize
        );

        ImGui::Text(
            "First sector: %d",
            details.firstSector
        );

        ImGui::Text(
            "Compression: %s",
            getCompressionTypeName(details.compressionType)
        );

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("ARCHIVE");

        if (!details.isArchive) {
            ImGui::TextUnformatted("Archive file count: N/A");
        }
        else {
            ImGui::Text(
                "Archive file count: %zu",
                details.archiveFiles.size()
            );

            ImGui::Spacing();

            for (const rf::cache::ArchiveFileDetails& archiveFile :
                details.archiveFiles) {
                ImGui::Text(
                    "#%d hash 0x%08X",
                    archiveFile.index,
                    static_cast<unsigned int>(archiveFile.hash)
                );

                ImGui::Text(
                    "  size: %u -> %u bytes",
                    archiveFile.compressedSize,
                    archiveFile.uncompressedSize
                );

                ImGui::Text(
                    "  payload: %zu bytes",
                    archiveFile.payloadSize
                );
            }
        }
    }

    if (state.activeModel) {
        const rf::model::ModelAsset& model =
            *state.activeModel;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextUnformatted("MODEL");

        if (ImGui::Button("Copy Model Debug")) {
            std::string debug =
                buildModelDebugText(
                    state,
                    model
                );

            ImGui::SetClipboardText(
                debug.c_str()
            );
        }

        ImGui::Text(
            "Vertices: %zu",
            model.vertices.size()
        );

        ImGui::Text(
            "Faces: %zu",
            model.faces.size()
        );

        ImGui::Text(
            "Texture UV mappings: %zu",
            model.textureUVMappings.size()
        );

        ImGui::Text(
            "Loaded textures: %zu",
            model.textures.size()
        );

        std::array<int, 12> priorityCounts {};

        for (const rf::model::Face& face : model.faces) {
            if (face.priority < static_cast<int>(priorityCounts.size())) {
                priorityCounts[face.priority]++;
            }
        }

        ImGui::Spacing();
        ImGui::TextUnformatted("Priority Buckets");

        for (size_t i = 0; i < priorityCounts.size(); i++) {
            ImGui::Text(
                "%zu: %d",
                i,
                priorityCounts[i]
            );
        }
    }

    ImGui::EndChild();
}

}
