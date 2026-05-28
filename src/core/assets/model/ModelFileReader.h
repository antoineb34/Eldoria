#pragma once

#include <optional>
#include <vector>
#include <cstdint>

#include "ModelFile.h"

namespace rf::model {

class ModelFileReader {
public:
    std::optional<ModelFile> read(
        const std::vector<uint8_t>& payload
    ) const;

private:
    bool validatePayload(
        const std::vector<uint8_t>& payload
    ) const;

    ModelFooter readFooter(
        const std::vector<uint8_t>& payload
    ) const;

    ModelLayout calculateLayout(
        const ModelFooter& footer
    ) const;
};

}
