#include "CacheFileDetails.h"

#include "ArchiveReader.h"

namespace rf::cache {

CacheFileDetails inspectCacheFile(
    const CacheFile& file
) {
    CacheFileDetails details;

    details.index = file.index;
    details.fileId = file.id;
    details.payloadSize = file.payload.size();
    details.cacheEntrySize = file.entry.size;
    details.firstSector = file.entry.firstSector;
    details.compressionType =
        rf::compression::detectCompression(file.payload);

    auto archive = ArchiveReader::read(file.payload);

    if (!archive.has_value()) {
        return details;
    }

    details.isArchive = true;
    details.archiveFiles.reserve(archive->files.size());

    for (std::size_t i = 0; i < archive->files.size(); i++) {
        const ArchiveFile& archiveFile =
            archive->files[i];

        ArchiveFileDetails fileDetails;
        fileDetails.index =
            static_cast<int>(i);
        fileDetails.hash =
            archiveFile.hash;
        fileDetails.uncompressedSize =
            archiveFile.uncompressedSize;
        fileDetails.compressedSize =
            archiveFile.compressedSize;
        fileDetails.payloadSize =
            archiveFile.payload.size();

        details.archiveFiles.push_back(
            fileDetails
        );
    }

    return details;
}

}
