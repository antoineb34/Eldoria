#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "item/ItemData.h"
#include "item/ItemDecoder.h"

namespace eld::item {

class ItemLoader {
public:
  explicit ItemLoader(const eld::cache::Cache &cache);

  const ItemData &data(std::uint16_t id) const;

  std::optional<ItemData> find(std::uint16_t id) const;

  std::vector<std::uint16_t> listIds() const;

  bool contains(std::uint16_t id) const;

  std::size_t count() const;

private:
  ItemData loadData(std::uint16_t id) const;

  static constexpr auto Index = eld::cache::IndexId::Config;

  static constexpr std::uint16_t ArchiveId = 2;

  static constexpr std::string_view DataFile = "obj.dat";

  static constexpr std::string_view IndexFile = "obj.idx";

  eld::archive::Archive archive_;
  ItemDecoder decoder_;

  mutable std::unordered_map<std::uint16_t, ItemData> dataCache_;
};

} // namespace eld::item
