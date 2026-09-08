#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "Message.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/MessageDecoder.h"

namespace eld::message {

class MessageRepository {
public:
    explicit MessageRepository(
        const eld::cache::Cache& cache
    );

    Message get(
        std::uint16_t id
    ) const;

    std::optional<Message> find(
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
        "mes.dat";

    static constexpr std::string_view IndexFile =
        "mes.idx";

    eld::archive::Archive archive_;
    MessageDecoder decoder_;
};

}
