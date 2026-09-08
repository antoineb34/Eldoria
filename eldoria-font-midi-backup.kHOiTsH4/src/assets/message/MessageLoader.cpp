#include "message/MessageLoader.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

#include "binary/ByteReader.h"

namespace eld::message {

MessageLoader::MessageLoader(const eld::cache::Cache &cache)
    : archive_(eld::archive::load(cache.open(Index), ArchiveId)) {}

MessageData MessageLoader::loadData(std::uint16_t id) const {
  const eld::archive::ArchiveFile &dataFile = archive_.get(DataFile);

  const eld::archive::ArchiveFile &indexFile = archive_.get(IndexFile);

  eld::binary::ByteReader dataReader(dataFile.payload);

  eld::binary::ByteReader indexReader(indexFile.payload);

  const auto dataCount = dataReader.readU16();
  const auto indexCount = indexReader.readU16();

  if (dataCount != indexCount) {
    throw std::runtime_error("Message data/index count mismatch");
  }

  if (id >= dataCount) {
    throw std::out_of_range("Message does not exist");
  }

  for (std::uint16_t currentId = 0; currentId < id; ++currentId) {
    const auto size = indexReader.readU16();

    if (!dataReader.canRead(size)) {
      throw std::runtime_error("Message asset exceeds data file");
    }

    dataReader.readBytes(size);
  }

  const auto size = indexReader.readU16();

  if (!dataReader.canRead(size)) {
    throw std::runtime_error("Message asset exceeds data file");
  }

  const std::vector<std::uint8_t> data = dataReader.readBytes(size);

  try {
    MessageData asset = decoder_.decode(data);

    asset.id = id;

    return asset;
  } catch (const std::exception &error) {
    throw std::runtime_error("Failed to decode message " + std::to_string(id) +
                             ": " + error.what());
  }
}

std::optional<MessageData> MessageLoader::find(std::uint16_t id) const {
  if (!contains(id)) {
    return std::nullopt;
  }

  return data(id);
}

std::vector<std::uint16_t> MessageLoader::listIds() const {
  const auto assetCount = count();

  std::vector<std::uint16_t> ids;

  ids.reserve(assetCount);

  for (std::uint16_t id = 0; id < assetCount; ++id) {
    ids.push_back(id);
  }

  return ids;
}

bool MessageLoader::contains(std::uint16_t id) const { return id < count(); }

std::size_t MessageLoader::count() const {
  const eld::archive::ArchiveFile &dataFile = archive_.get(DataFile);

  const eld::archive::ArchiveFile &indexFile = archive_.get(IndexFile);

  eld::binary::ByteReader dataReader(dataFile.payload);

  eld::binary::ByteReader indexReader(indexFile.payload);

  const auto dataCount = dataReader.readU16();
  const auto indexCount = indexReader.readU16();

  if (dataCount != indexCount) {
    throw std::runtime_error("Message data/index count mismatch");
  }

  return dataCount;
}

const MessageData &MessageLoader::data(std::uint16_t id) const {
  const auto cached = dataCache_.find(id);

  if (cached != dataCache_.end()) {
    return cached->second;
  }

  const auto entry = dataCache_.emplace(id, loadData(id));

  return entry.first->second;
}

} // namespace eld::message
