#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>

#include "Animation.h"
#include "decoders/AnimationDecoder.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "cache/Store.h"
#include "decoders/ImageDecoder.h"
#include "decoders/ModelDecoder.h"
#include "ModelResource.h"
#include "Texture.h"

namespace eld::asset {

class ResourceProvider {
public:
    explicit ResourceProvider(
        const eld::cache::Cache& cache
    );

    const eld::model::ModelResource& getModel(
        std::uint16_t id
    ) const;

    bool containsModel(
        std::uint16_t id
    ) const;

    const eld::texture::Texture& getTexture(
        std::uint16_t id
    ) const;

    bool containsTexture(
        std::uint16_t id
    ) const;

    const eld::animation::Animation& getAnimation(
        std::uint16_t id
    ) const;

    bool containsAnimation(
        std::uint16_t id
    ) const;

    std::optional<eld::animation::AnimationFrameView>
    findAnimationFrame(
        std::uint16_t frameId
    ) const;

private:
    struct FrameLocation {
        std::uint16_t animationId = 0;
        std::size_t frameIndex = 0;
    };

    eld::model::ModelResource loadModel(
        std::uint16_t id
    ) const;

    eld::texture::Texture loadTexture(
        std::uint16_t id
    ) const;

    eld::animation::Animation loadAnimation(
        std::uint16_t id
    ) const;

    void ensureAnimationFrameIndex() const;

    eld::cache::Store configStore_;
    eld::cache::Store modelStore_;
    eld::cache::Store animationStore_;

    eld::archive::Archive textureArchive_;

    eld::model::ModelDecoder modelDecoder_;
    eld::image::ImageDecoder imageDecoder_;
    eld::animation::AnimationDecoder animationDecoder_;

    mutable std::map<
        std::uint16_t,
        eld::model::ModelResource
    > models_;

    mutable std::map<
        std::uint16_t,
        eld::texture::Texture
    > textures_;

    mutable std::map<
        std::uint16_t,
        eld::animation::Animation
    > animations_;

    mutable std::map<
        std::uint16_t,
        FrameLocation
    > animationFrames_;

    mutable bool animationFrameIndexBuilt_ = false;
};

}
