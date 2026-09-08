#include "map/MapLoader.h"

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <string>

#include "archive/Archive.h"
#include "cache/File.h"

namespace eld::map {

MapLoader::MapLoader(const eld::cache::Cache &cache)
    : maps_(cache.open(Index)) {
  index_ = readMapIndex(cache);
}

const TerrainData &MapLoader::terrain(std::uint16_t regionId) const {
  const auto cached = terrainCache_.find(regionId);

  if (cached != terrainCache_.end()) {
    return cached->second;
  }

  const auto [inserted, wasInserted] =
      terrainCache_.emplace(regionId, loadTerrain(requireEntry(regionId)));

  (void)wasInserted;
  return inserted->second;
}

const MapLocationData &MapLoader::locations(std::uint16_t regionId) const {
  const auto cached = locationCache_.find(regionId);

  if (cached != locationCache_.end()) {
    return cached->second;
  }

  const auto [inserted, wasInserted] =
      locationCache_.emplace(regionId, loadLocations(requireEntry(regionId)));

  (void)wasInserted;
  return inserted->second;
}

const MapRegionResource &MapLoader::resource(std::uint16_t regionId) const {
  const auto cached = resourceCache_.find(regionId);

  if (cached != resourceCache_.end()) {
    return cached->second;
  }

  const auto [inserted, wasInserted] = resourceCache_.emplace(
      regionId, assembler_.assemble(requireEntry(regionId), terrain(regionId),
                                    locations(regionId)));

  (void)wasInserted;
  return inserted->second;
}

std::optional<MapRegionResource> MapLoader::find(std::uint16_t regionId) const {
  if (!contains(regionId)) {
    return std::nullopt;
  }

  return resource(regionId);
}

std::vector<std::uint16_t> MapLoader::listIds() const {
  std::vector<std::uint16_t> ids;
  ids.reserve(index_.entries.size());

  for (const MapIndexEntry &entry : index_.entries) {
    ids.push_back(entry.regionId);
  }

  return ids;
}

bool MapLoader::contains(std::uint16_t regionId) const {
  return findEntry(regionId) != nullptr;
}

std::size_t MapLoader::count() const { return index_.entries.size(); }

const MapIndexEntry *MapLoader::findEntry(std::uint16_t regionId) const {
  const auto entry =
      std::lower_bound(index_.entries.begin(), index_.entries.end(), regionId,
                       [](const MapIndexEntry &candidate, std::uint16_t id) {
                         return candidate.regionId < id;
                       });

  return entry != index_.entries.end() && entry->regionId == regionId ? &*entry
                                                                      : nullptr;
}

const std::vector<MapIndexEntry> &MapLoader::entries() const {
  return index_.entries;
}

MapIndexData MapLoader::readMapIndex(const eld::cache::Cache &cache) const {
  const eld::archive::Archive archive =
      eld::archive::load(cache.open(ConfigIndex), VersionListArchiveId);

  const eld::archive::ArchiveFile &file = archive.get(MapIndexFile);

  try {
    return indexDecoder_.decode(file.payload);
  } catch (const std::exception &error) {
    throw std::runtime_error("Failed to decode map index: " +
                             std::string(error.what()));
  }
}

const MapIndexEntry &MapLoader::requireEntry(std::uint16_t regionId) const {
  const MapIndexEntry *entry = findEntry(regionId);

  if (entry == nullptr) {
    throw std::out_of_range("Map region does not exist: " +
                            std::to_string(regionId));
  }

  return *entry;
}

TerrainData MapLoader::loadTerrain(const MapIndexEntry &entry) const {
  const eld::cache::File file = maps_.get(entry.terrainFileId);

  const std::vector<std::uint8_t> payload =
      file.getBytes(eld::cache::CompressionState::Decompressed);

  try {
    return terrainDecoder_.decode(payload, entry.regionId);
  } catch (const std::exception &error) {
    throw std::runtime_error("Failed to decode map terrain for region " +
                             std::to_string(entry.regionId) + ": " +
                             error.what());
  }
}

MapLocationData MapLoader::loadLocations(const MapIndexEntry &entry) const {
  const eld::cache::File file = maps_.get(entry.locationFileId);

  const std::vector<std::uint8_t> payload =
      file.getBytes(eld::cache::CompressionState::Decompressed);

  try {
    return locationDecoder_.decode(payload);
  } catch (const std::exception &error) {
    throw std::runtime_error("Failed to decode map locations for region " +
                             std::to_string(entry.regionId) + ": " +
                             error.what());
  }
}

} // namespace eld::map
