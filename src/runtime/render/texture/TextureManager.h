#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "TextureHandle.h"
#include "TextureResource.h"

namespace eld::render
{

class TextureManager
{
public:
    TextureHandle create(
        TextureResource resource);

    const TextureResource& get(
        TextureHandle handle) const;

    bool isValid(
        TextureHandle handle) const;

    void destroy(
        TextureHandle handle);

private:
    struct Slot
    {
        std::optional<TextureResource> resource;
        std::uint32_t generation = 1;
    };

    std::vector<Slot> slots_;
};

}
