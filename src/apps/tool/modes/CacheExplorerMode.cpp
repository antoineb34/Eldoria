#include "CacheExplorerMode.h"

#include <iomanip>
#include <iostream>
#include <string>

#include "../../../core/cache/ArchiveDecoder.h"
#include "../../../core/cache/ArchiveFileTable.h"
#include "../../../core/cache/KnownArchives.h"
#include "../../../core/cache/NameHash.h"

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

    inspectIndex0();

    std::vector<char> npcDat =
        configLoader_.loadFile(
            "npc.dat"
        );

    std::cout
        << "\nEXTRACTED npc.dat: "
        << npcDat.size()
        << " bytes\n";

    return true;
}

void CacheExplorerMode::handleEvent(
    const SDL_Event& event
) {
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

    SDL_SetRenderDrawColor(
        renderer,
        28,
        56,
        60,
        255
    );

    SDL_RenderClear(renderer);

    SDL_FRect panel {
        40.0f,
        40.0f,
        static_cast<float>(windowWidth) - 80.0f,
        static_cast<float>(windowHeight) - 80.0f
    };

    SDL_SetRenderDrawColor(
        renderer,
        220,
        240,
        230,
        255
    );

    SDL_RenderRect(
        renderer,
        &panel
    );
}

void CacheExplorerMode::inspectIndex0() {

    const auto& knownNames =
        rf::cache::KNOWN_ARCHIVES;

    auto findKnownName =
        [&](uint32_t hash) -> std::string {

            for (const std::string& name : knownNames) {

                if (rf::cache::hashName(name) == hash) {
                    return name;
                }
            }

            return "";
        };

    std::cout
        << "\n\n====================================================\n"
        << "CACHE EXPLORER — IDX0 ARCHIVE INSPECTION\n"
        << "====================================================\n";

    for (uint32_t archiveId = 0; archiveId < 20; archiveId++) {

        rf::cache::CacheArchive archive =
            configCache_.readArchive(archiveId);

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

        std::cout
            << "\n----------------------------------------------------\n"
            << "archive "
            << archiveId
            << "\n"
            << "----------------------------------------------------\n"
            << "size: "
            << archive.entry.size
            << "\n"
            << "first sector: "
            << archive.entry.firstSector
            << "\n"
            << "compressed: "
            << (decoded.compressed ? "yes" : "no")
            << "\n"
            << "compressed size: "
            << decoded.compressedSize
            << "\n"
            << "uncompressed size: "
            << decoded.uncompressedSize
            << "\n"
            << "decoded payload: "
            << decoded.payload.size()
            << "\n"
            << "file count: "
            << table.fileCount
            << "\n";

        for (int i = 0; i < table.files.size(); i++) {

            const auto& file =
                table.files[i];

            std::string knownName =
                findKnownName(file.hash);

            std::cout
                << "  file "
                << std::setw(2)
                << i
                << " | hash="
                << file.hash;

            if (!knownName.empty()) {
                std::cout
                    << " | name="
                    << knownName;
            }

            std::cout
                << " | uncompressed="
                << file.uncompressedSize
                << " | compressed="
                << file.compressedSize
                << " | offset="
                << file.offset
                << "\n";
        }
    }
}

}
