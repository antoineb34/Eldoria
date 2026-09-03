#include "ResourceProvider.h"

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

#include "archive/ArchiveParser.h"

namespace eld::asset {

namespace {

constexpr std::uint16_t TextureArchiveId = 6;

eld::archive::Archive loadTextureArchive(
    const eld::cache::Store& configStore
) {
    const eld::cache::File file =
        configStore.get(
            TextureArchiveId
        );

    eld::archive::ArchiveParser parser;

    auto archive =
        parser.parse(
            file.getBytes()
        );

    if (!archive.has_value()) {
        throw std::runtime_error(
            "Failed to parse texture archive"
        );
    }

    return std::move(*archive);
}

}

ResourceProvider::ResourceProvider(
    const eld::cache::Cache& cache
)
    : configStore_(
          cache.open(
              eld::cache::IndexId::Config
          )
      ),
      modelStore_(
          cache.open(
              eld::cache::IndexId::Models
          )
      ),
      animationStore_(
          cache.open(
              eld::cache::IndexId::Animations
          )
      ),
      textureArchive_(
          loadTextureArchive(
              configStore_
          )
      ) {
}

const eld::model::ModelResource&
ResourceProvider::getModel(
    std::uint16_t id
) const {
    const auto existing =
        models_.find(id);

    if (existing != models_.end()) {
        return existing->second;
    }

    const auto [inserted, _] =
        models_.emplace(
            id,
            loadModel(id)
        );

    return inserted->second;
}

eld::model::ModelResource
ResourceProvider::loadModel(
    std::uint16_t id
) const {
    const eld::cache::File file =
        modelStore_.get(id);

    eld::model::Model model;

    try {
        model =
            modelDecoder_.decode(
                file.getBytes()
            );
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode model " +
            std::to_string(id) +
            ": " +
            error.what()
        );
    }

    model.id = id;

    eld::model::ModelResource resource{
        .model = std::move(model)
    };

    for (
        const eld::model::Face& face :
        resource.model.faces
    ) {
        if (!face.textureId.has_value()) {
            continue;
        }

        const std::uint16_t textureId =
            *face.textureId;

        if (
            resource.textures.contains(
                textureId
            )
        ) {
            continue;
        }

        resource.textures.emplace(
            textureId,
            getTexture(textureId)
        );
    }

    return resource;
}

bool ResourceProvider::containsModel(
    std::uint16_t id
) const {
    return modelStore_.contains(id);
}

const eld::texture::Texture&
ResourceProvider::getTexture(
    std::uint16_t id
) const {
    const auto existing =
        textures_.find(id);

    if (existing != textures_.end()) {
        return existing->second;
    }

    const auto [inserted, _] =
        textures_.emplace(
            id,
            loadTexture(id)
        );

    return inserted->second;
}

eld::texture::Texture
ResourceProvider::loadTexture(
    std::uint16_t id
) const {
    const auto& index =
        textureArchive_.get(
            "index.dat"
        );

    const auto& data =
        textureArchive_.get(
            std::to_string(id) +
            ".dat"
        );

    try {
        return eld::texture::Texture{
            .id = id,
            .image = imageDecoder_.decode(
                data.payload,
                index.payload
            )
        };
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode texture " +
            std::to_string(id) +
            ": " +
            error.what()
        );
    }
}

bool ResourceProvider::containsTexture(
    std::uint16_t id
) const {
    return textureArchive_.contains(
        std::to_string(id) +
        ".dat"
    );
}

const eld::animation::Animation&
ResourceProvider::getAnimation(
    std::uint16_t id
) const {
    const auto existing =
        animations_.find(id);

    if (existing != animations_.end()) {
        return existing->second;
    }

    const auto [inserted, _] =
        animations_.emplace(
            id,
            loadAnimation(id)
        );

    return inserted->second;
}

eld::animation::Animation
ResourceProvider::loadAnimation(
    std::uint16_t id
) const {
    const eld::cache::File file =
        animationStore_.get(id);

    try {
        eld::animation::Animation animation =
            animationDecoder_.decode(
                file.getBytes()
            );

        animation.id = id;

        return animation;
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode animation " +
            std::to_string(id) +
            ": " +
            error.what()
        );
    }
}

bool ResourceProvider::containsAnimation(
    std::uint16_t id
) const {
    return animationStore_.contains(id);
}

void ResourceProvider::ensureAnimationFrameIndex() const {
    if (animationFrameIndexBuilt_) {
        return;
    }

    std::map<
        std::uint16_t,
        FrameLocation
    > frames;

    for (
        const eld::cache::FileEntry& entry :
        animationStore_.list()
    ) {
        const std::uint16_t animationId =
            entry.fileId;

        const auto& animation =
            getAnimation(animationId);

        for (
            std::size_t frameIndex = 0;
            frameIndex < animation.frames.size();
            ++frameIndex
        ) {
            const std::uint16_t frameId =
                animation.frames[
                    frameIndex
                ].id;

            const auto [_, inserted] =
                frames.emplace(
                    frameId,
                    FrameLocation{
                        animationId,
                        frameIndex
                    }
                );

            if (!inserted) {
                throw std::runtime_error(
                    "Duplicate global animation frame ID " +
                    std::to_string(frameId)
                );
            }
        }
    }

    animationFrames_ =
        std::move(frames);

    animationFrameIndexBuilt_ = true;
}

std::optional<eld::animation::AnimationFrameView>
ResourceProvider::findAnimationFrame(
    std::uint16_t frameId
) const {
    ensureAnimationFrameIndex();

    const auto location =
        animationFrames_.find(
            frameId
        );

    if (
        location ==
        animationFrames_.end()
    ) {
        return std::nullopt;
    }

    const auto& animation =
        getAnimation(
            location->second.animationId
        );

    return eld::animation::AnimationFrameView{
        animation.frames[
            location->second.frameIndex
        ],
        animation.skeleton
    };
}

}
