#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "message/MessageData.h"
#include "message/MessageDecoder.h"

namespace eld::message {

class MessageLoader {
public:
  explicit MessageLoader(const eld::cache::Cache &cache);

  const MessageData &data(std::uint16_t id) const;

  std::optional<MessageData> find(std::uint16_t id) const;

  std::vector<std::uint16_t> listIds() const;

  bool contains(std::uint16_t id) const;

  std::size_t count() const;

private:
  MessageData loadData(std::uint16_t id) const;

  static constexpr auto Index = eld::cache::IndexId::Config;

  static constexpr std::uint16_t ArchiveId = 2;

  static constexpr std::string_view DataFile = "mes.dat";

  static constexpr std::string_view IndexFile = "mes.idx";

  eld::archive::Archive archive_;
  MessageDecoder decoder_;

  mutable std::unordered_map<std::uint16_t, MessageData> dataCache_;
};

} // namespace eld::message
