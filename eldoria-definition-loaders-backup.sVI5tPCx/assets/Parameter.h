#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace eld::parameter {

struct Parameter {
    std::uint16_t id = 0;

    std::optional<char> type;
    std::int32_t defaultInteger = 0;
    std::string defaultString;

    bool autoDisable = true;

    bool isString() const {
        return type.has_value() &&
            *type == 's';
    }
};

}
