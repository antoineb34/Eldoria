#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "Interface.h"
#include "InterfaceFile.h"
#include "InterfaceFileParser.h"
#include "cache/Store.h"

namespace eld::interface {

using InterfaceWidget = InterfaceFileWidget;

class InterfaceRepository {
public:
    InterfaceRepository(
        eld::cache::Store store,
        std::uint16_t archiveId
    );

    const Interface& get() const;

    const InterfaceFile& getFile() const;

    const InterfaceWidget& getWidget(
        std::uint16_t id
    ) const;

    const InterfaceWidget* findWidget(
        std::uint16_t id
    ) const;

    const InterfaceWidget& get(
        std::uint16_t id
    ) const;

    const InterfaceWidget* find(
        std::uint16_t id
    ) const;

    const std::vector<InterfaceWidget>& list() const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    Interface interface_;

    InterfaceFileParser parser_;
};

}
