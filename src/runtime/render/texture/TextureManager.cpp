#include "TextureManager.h"

#include <stdexcept>
#include <utility>

namespace eld::render
{

TextureHandle TextureManager::create(
    TextureResource resource)
{
    for (std::uint32_t index = 0;
         index < slots_.size();
         ++index)
    {
        Slot& slot = slots_[index];

        if (!slot.resource.has_value())
        {
            slot.resource = std::move(resource);

            return {
                .index = index,
                .generation = slot.generation
            };
        }
    }

    Slot slot;
    slot.resource = std::move(resource);

    slots_.push_back(
        std::move(slot));

    const std::uint32_t index =
        static_cast<std::uint32_t>(
            slots_.size() - 1);

    return {
        .index = index,
        .generation = slots_[index].generation
    };
}

bool TextureManager::isValid(
    TextureHandle handle) const
{
    if (handle.index >= slots_.size())
        return false;

    const Slot& slot =
        slots_[handle.index];

    return
        slot.resource.has_value() &&
        slot.generation == handle.generation;
}

const TextureResource&
TextureManager::get(
    TextureHandle handle) const
{
    if (!isValid(handle))
    {
        throw std::runtime_error(
            "Invalid texture handle");
    }

    return *slots_[handle.index].resource;
}

void TextureManager::destroy(
    TextureHandle handle)
{
    if (!isValid(handle))
    {
        throw std::runtime_error(
            "Invalid texture handle");
    }

    Slot& slot =
        slots_[handle.index];

    slot.resource.reset();

    ++slot.generation;

    if (slot.generation == 0)
        ++slot.generation;
}

}
