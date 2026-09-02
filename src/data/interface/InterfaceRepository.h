#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "InterfaceFile.h"
#include "InterfaceFileParser.h"
#include "InterfaceWidget.h"
#include "cache/Store.h"

namespace eld::interface {

class InterfaceRepository {
public:
    InterfaceRepository(
        eld::cache::Store store,
        std::uint16_t archiveId
    );

    const InterfaceFile& file() const;

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
    InterfaceFile file_;
    InterfaceFileParser parser_;
};

}
