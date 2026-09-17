#include "font/FontLoader.h"

#include <exception>
#include <stdexcept>
#include <string>

#include "archive/ArchiveHashes.h"

namespace eld::font {

FontLoader::FontLoader(const eld::cache::Cache &cache)
    : archive_(eld::archive::load(cache.open(Index), ArchiveId)) {}

FontData FontLoader::loadData(std::uint16_t id) const {
  const eld::archive::ArchiveFile &dataFile = archive_.get(id);

  if (dataFile.nameHash == eld::archive::hashName(IndexFile)) {
    throw std::out_of_range("Font does not exist");
  }

  const eld::archive::ArchiveFile &indexFile = archive_.get(IndexFile);

  try {
    FontData font = decoder_.decode(dataFile.payload, indexFile.payload);

    font.id = id;

    const std::optional<std::string_view> name =
        eld::archive::findName(dataFile.nameHash);

    if (name.has_value()) {
      font.name = std::string(*name);
    }

    return font;
  } catch (const std::exception &error) {
    throw std::runtime_error("Failed to decode font " + std::to_string(id) +
                             ": " + error.what());
  }
}

std::optional<FontData> FontLoader::find(std::uint16_t id) const {
  if (!contains(id)) {
    return std::nullopt;
  }

  return data(id);
}

std::vector<std::uint16_t> FontLoader::listIds() const {
  const std::uint32_t indexHash = eld::archive::hashName(IndexFile);

  std::vector<std::uint16_t> ids;

  ids.reserve(archive_.count());

  for (const eld::archive::ArchiveFile &file : archive_.list()) {
    if (file.nameHash == indexHash) {
      continue;
    }

    ids.push_back(file.id);
  }

  return ids;
}

bool FontLoader::contains(std::uint16_t id) const {
  const eld::archive::ArchiveFile *file = archive_.find(id);

  return file != nullptr && file->nameHash != eld::archive::hashName(IndexFile);
}

std::size_t FontLoader::count() const { return listIds().size(); }

const FontData &FontLoader::data(std::uint16_t id) const {
  const auto cached = dataCache_.find(id);

  if (cached != dataCache_.end()) {
    return cached->second;
  }

  const auto entry = dataCache_.emplace(id, loadData(id));

  return entry.first->second;
}

} // namespace eld::font
