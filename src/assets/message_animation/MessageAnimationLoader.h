#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "message_animation/MessageAnimationData.h"
#include "message_animation/MessageAnimationDecoder.h"

namespace eld::message_animation {

class MessageAnimationLoader {
public:
  explicit MessageAnimationLoader(const eld::cache::Cache &cache);

  const MessageAnimationData &data(std::uint16_t id) const;

  std::optional<MessageAnimationData> find(std::uint16_t id) const;

  std::vector<std::uint16_t> listIds() const;

  bool contains(std::uint16_t id) const;

  std::size_t count() const;

private:
  MessageAnimationData loadData(std::uint16_t id) const;

  static constexpr auto Index = eld::cache::IndexId::Config;

  static constexpr std::uint16_t ArchiveId = 2;

  static constexpr std::string_view DataFile = "mesanim.dat";

  static constexpr std::string_view IndexFile = "mesanim.idx";

  eld::archive::Archive archive_;
  MessageAnimationDecoder decoder_;

  mutable std::unordered_map<std::uint16_t, MessageAnimationData> dataCache_;
};

} // namespace eld::message_animation
