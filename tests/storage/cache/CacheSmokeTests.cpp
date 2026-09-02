#include <catch2/catch_test_macros.hpp>

#include "cache/Sector.h"

TEST_CASE("Cache sector format has the expected size") {
    REQUIRE(eld::cache::Sector::HeaderSize == 8);
    REQUIRE(eld::cache::Sector::DataSize == 512);
    REQUIRE(eld::cache::Sector::TotalSize == 520);
}
