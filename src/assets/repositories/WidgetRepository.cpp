#include "repositories/WidgetRepository.h"

#include <stdexcept>

#include "archive/Archive.h"

namespace eld::interface {

WidgetRepository::WidgetRepository(
    const eld::cache::Cache& cache
) {
    const eld::archive::Archive archive =
        eld::archive::load(
            cache.open(Index),
            ArchiveId
        );

    const eld::archive::ArchiveFile& dataFile =
        archive.get(
            DataFile
        );

    widgets_ =
        decoder_.decode(
            dataFile.payload
        );
}


const Widget&
WidgetRepository::get(
    std::uint16_t id
) const {
    const Widget* widget =
        find(id);

    if (widget == nullptr) {
        throw std::out_of_range(
            "Interface widget does not exist"
        );
    }

    return *widget;
}


const Widget*
WidgetRepository::find(
    std::uint16_t id
) const {
    for (
        const Widget& widget :
        widgets_
    ) {
        if (widget.id == id) {
            return &widget;
        }
    }

    return nullptr;
}


const std::vector<Widget>&
WidgetRepository::list() const {
    return widgets_;
}


std::vector<std::uint16_t>
WidgetRepository::listIds() const {
    std::vector<std::uint16_t> ids;

    ids.reserve(
        widgets_.size()
    );

    for (
        const Widget& widget :
        widgets_
    ) {
        ids.push_back(
            widget.id
        );
    }

    return ids;
}


bool WidgetRepository::contains(
    std::uint16_t id
) const {
    return find(id) != nullptr;
}


std::size_t WidgetRepository::count() const {
    return widgets_.size();
}

}
