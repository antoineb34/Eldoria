#include "identity_kit/IdentityKitLoader.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

#include "binary/ByteReader.h"

namespace eld::identity_kit {

IdentityKitLoader::IdentityKitLoader(const eld::cache::Cache &cache)
    : archive_(eld::archive::load(cache.open(Index), ArchiveId)) {}

IdentityKitData IdentityKitLoader::loadData(std::uint16_t id) const {
  const eld::archive::ArchiveFile &dataFile = archive_.get(DataFile);

  const eld::archive::ArchiveFile &indexFile = archive_.get(IndexFile);

  eld::binary::ByteReader dataReader(dataFile.payload);

  eld::binary::ByteReader indexReader(indexFile.payload);

  const auto dataCount = dataReader.readU16();
  const auto indexCount = indexReader.readU16();

  if (dataCount != indexCount) {
    throw std::runtime_error("Identity-kit data/index count mismatch");
  }

  if (id >= dataCount) {
    throw std::out_of_range("Identity-kit does not exist");
  }

  for (std::uint16_t currentId = 0; currentId < id; ++currentId) {
    const auto size = indexReader.readU16();

    if (!dataReader.canRead(size)) {
      throw std::runtime_error("Identity-kit asset exceeds data file");
    }

    dataReader.readBytes(size);
  }

  const auto size = indexReader.readU16();

  if (!dataReader.canRead(size)) {
    throw std::runtime_error("Identity-kit asset exceeds data file");
  }

  const std::vector<std::uint8_t> data = dataReader.readBytes(size);

  try {
    IdentityKitData kit = decoder_.decode(data);

    kit.id = id;

    return kit;
  } catch (const std::exception &error) {
    throw std::runtime_error("Failed to decode identity-kit " +
                             std::to_string(id) + ": " + error.what());
  }
}

std::optional<IdentityKitData> IdentityKitLoader::find(std::uint16_t id) const {
  if (!contains(id)) {
    return std::nullopt;
  }

  return data(id);
}

std::vector<std::uint16_t> IdentityKitLoader::listIds() const {
  std::vector<std::uint16_t> ids;

  ids.reserve(count());

  for (std::uint16_t id = 0; id < count(); ++id) {
    ids.push_back(id);
  }

  return ids;
}

bool IdentityKitLoader::contains(std::uint16_t id) const {
  return id < count();
}

std::size_t IdentityKitLoader::count() const {
  const eld::archive::ArchiveFile &dataFile = archive_.get(DataFile);

  const eld::archive::ArchiveFile &indexFile = archive_.get(IndexFile);

  eld::binary::ByteReader dataReader(dataFile.payload);

  eld::binary::ByteReader indexReader(indexFile.payload);

  const auto dataCount = dataReader.readU16();
  const auto indexCount = indexReader.readU16();

  if (dataCount != indexCount) {
    throw std::runtime_error("Identity-kit data/index count mismatch");
  }

  return dataCount;
}

const IdentityKitData &IdentityKitLoader::data(std::uint16_t id) const {
  const auto cached = dataCache_.find(id);

  if (cached != dataCache_.end()) {
    return cached->second;
  }

  const auto entry = dataCache_.emplace(id, loadData(id));

  return entry.first->second;
}

const IdentityKitResource &IdentityKitLoader::resource(std::uint16_t id) const {
  const auto cached = resourceCache_.find(id);

  if (cached != resourceCache_.end()) {
    return cached->second;
  }

  const auto entry = resourceCache_.emplace(id, assembler_.assemble(data(id)));

  return entry.first->second;
}

} // namespace eld::identity_kit
