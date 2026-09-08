#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "cache/Cache.h"
#include "interface/WidgetAssembler.h"
#include "interface/WidgetData.h"
#include "interface/WidgetDecoder.h"
#include "interface/WidgetResource.h"

namespace eld::interface {

class WidgetLoader {
public:
  explicit WidgetLoader(const eld::cache::Cache &cache);

  const WidgetData &data(std::uint16_t id) const;

  const WidgetResource &resource(std::uint16_t id) const;

  std::optional<WidgetData> find(std::uint16_t id) const;

  const std::vector<WidgetData> &list() const;

  std::vector<std::uint16_t> listIds() const;

  bool contains(std::uint16_t id) const;

  std::size_t count() const;

private:
  WidgetData loadData(std::uint16_t id) const;

  static constexpr auto Index = eld::cache::IndexId::Config;

  static constexpr std::uint16_t ArchiveId = 3;

  static constexpr std::string_view DataFile = "data";

  std::vector<WidgetData> widgets_;
  WidgetDecoder decoder_;
  WidgetAssembler assembler_;

  mutable std::unordered_map<std::uint16_t, WidgetData> dataCache_;

  mutable std::unordered_map<std::uint16_t, WidgetResource> resourceCache_;
};

} // namespace eld::interface
