#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "model/ModelMesh.h"

namespace eld::graphics {

class ModelSkinGroups {
public:
    static ModelSkinGroups build(
        const eld::model::ModelMesh& mesh
    );

    const std::vector<std::size_t>& vertices(
        std::uint8_t groupId
    ) const;

    const std::vector<std::size_t>& faces(
        std::uint8_t groupId
    ) const;

private:
    std::array<std::vector<std::size_t>, 256>
        verticesBySkin_;

    std::array<std::vector<std::size_t>, 256>
        facesBySkin_;
};

}
