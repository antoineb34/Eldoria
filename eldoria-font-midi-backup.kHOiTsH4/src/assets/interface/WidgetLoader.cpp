#include "interface/WidgetLoader.h"

#include <stdexcept>

#include "archive/Archive.h"

namespace eld::interface {

WidgetLoader::WidgetLoader(const eld::cache::Cache &cache) : assembler_(*this) {
  const eld::archive::Archive archive =
      eld::archive::load(cache.open(Index), ArchiveId);

  const eld::archive::ArchiveFile &dataFile = archive.get(DataFile);

  widgets_ = decoder_.decode(dataFile.payload);
}

WidgetData WidgetLoader::loadData(std::uint16_t id) const {
  const std::optional<WidgetData> widget = find(id);

  if (!widget.has_value()) {
    throw std::out_of_range("Interface widget does not exist");
  }

  return *widget;
}

std::optional<WidgetData> WidgetLoader::find(std::uint16_t id) const {
  for (const WidgetData &widget : widgets_) {
    if (widget.id == id) {
      return widget;
    }
  }

  return std::nullopt;
}

const std::vector<WidgetData> &WidgetLoader::list() const { return widgets_; }

std::vector<std::uint16_t> WidgetLoader::listIds() const {
  std::vector<std::uint16_t> ids;

  ids.reserve(widgets_.size());

  for (const WidgetData &widget : widgets_) {
    ids.push_back(widget.id);
  }

  return ids;
}

bool WidgetLoader::contains(std::uint16_t id) const {
  return find(id).has_value();
}

std::size_t WidgetLoader::count() const { return widgets_.size(); }

const WidgetData &WidgetLoader::data(std::uint16_t id) const {
  const auto cached = dataCache_.find(id);

  if (cached != dataCache_.end()) {
    return cached->second;
  }

  const auto entry = dataCache_.emplace(id, loadData(id));

  return entry.first->second;
}

const WidgetResource &WidgetLoader::resource(std::uint16_t id) const {
  const auto cached = resourceCache_.find(id);

  if (cached != resourceCache_.end()) {
    return cached->second;
  }

  const auto entry = resourceCache_.emplace(id, assembler_.assemble(id));

  return entry.first->second;
}

} // namespace eld::interface
