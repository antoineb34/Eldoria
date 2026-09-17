#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "font/FontData.h"
#include "font/FontDecoder.h"

namespace eld::font {

class FontLoader {
public:
  explicit FontLoader(const eld::cache::Cache &cache);

  const FontData &data(std::uint16_t id) const;
  std::optional<FontData> find(std::uint16_t id) const;

  std::vector<std::uint16_t> listIds() const;

  bool contains(std::uint16_t id) const;

  std::size_t count() const;

private:
  FontData loadData(std::uint16_t id) const;

  static constexpr auto Index = eld::cache::IndexId::Config;

  static constexpr std::uint16_t ArchiveId = 1;

  static constexpr std::string_view IndexFile = "index.dat";

  eld::archive::Archive archive_;
  FontDecoder decoder_;
  mutable std::unordered_map<std::uint16_t, FontData> dataCache_;
};

} // namespace eld::font
