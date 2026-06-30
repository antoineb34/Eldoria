#include "InterfaceRepository.h"

#include <optional>
#include <stdexcept>
#include <utility>

#include "archive/Archive.h"
#include "archive/ArchiveParser.h"
#include "cache/File.h"

namespace eld::interface {

InterfaceRepository::InterfaceRepository(
    eld::cache::Store store,
    std::uint16_t archiveId
) {
    const eld::cache::File cacheFile =
        store.get(archiveId);

    eld::archive::ArchiveParser archiveParser;

    std::optional<eld::archive::Archive> archive =
        archiveParser.parse(
            cacheFile.getBytes()
        );

    if (!archive.has_value()) {
        throw std::runtime_error(
            "Failed to parse interface archive"
        );
    }

    const eld::archive::ArchiveFile& dataFile =
        archive->get("data");

    std::optional<InterfaceFile> file =
        parser_.parse(dataFile.payload);

    if (!file.has_value()) {
        throw std::runtime_error(
            "Failed to parse interface file"
        );
    }

    interface_.file =
        std::move(*file);
}

const Interface& InterfaceRepository::get() const {
    return interface_;
}

const InterfaceFile& InterfaceRepository::getFile() const {
    return interface_.file;
}

const InterfaceWidget& InterfaceRepository::getWidget(
    std::uint16_t id
) const {
    const InterfaceWidget* widget =
        findWidget(id);

    if (widget == nullptr) {
        throw std::out_of_range(
            "Interface widget does not exist"
        );
    }

    return *widget;
}

const InterfaceWidget* InterfaceRepository::findWidget(
    std::uint16_t id
) const {
    for (const InterfaceWidget& widget : interface_.file.widgets) {
        if (widget.id == id) {
            return &widget;
        }
    }

    return nullptr;
}

const InterfaceWidget& InterfaceRepository::get(
    std::uint16_t id
) const {
    return getWidget(id);
}

const InterfaceWidget* InterfaceRepository::find(
    std::uint16_t id
) const {
    return findWidget(id);
}

const std::vector<InterfaceWidget>& InterfaceRepository::list() const {
    return interface_.file.widgets;
}

std::vector<std::uint16_t> InterfaceRepository::listIds() const {
    std::vector<std::uint16_t> ids;
    ids.reserve(interface_.file.widgets.size());

    for (const InterfaceWidget& widget : interface_.file.widgets) {
        ids.push_back(widget.id);
    }

    return ids;
}

bool InterfaceRepository::contains(
    std::uint16_t id
) const {
    return findWidget(id) != nullptr;
}

std::size_t InterfaceRepository::count() const {
    return interface_.file.widgets.size();
}

}
