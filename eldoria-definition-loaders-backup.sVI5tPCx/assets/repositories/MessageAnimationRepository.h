#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "MessageAnimation.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/MessageAnimationDecoder.h"

namespace eld::message_animation {

class MessageAnimationRepository {
public:
    explicit MessageAnimationRepository(
        const eld::cache::Cache& cache
    );

    MessageAnimation get(
        std::uint16_t id
    ) const;

    std::optional<MessageAnimation> find(
        std::uint16_t id
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    static constexpr auto Index =
        eld::cache::IndexId::Config;

    static constexpr std::uint16_t ArchiveId =
        2;

    static constexpr std::string_view DataFile =
        "mesanim.dat";

    static constexpr std::string_view IndexFile =
        "mesanim.idx";

    eld::archive::Archive archive_;
    MessageAnimationDecoder decoder_;
};

}
