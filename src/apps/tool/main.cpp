#include <iostream>

#include "../../core/cache/CacheStore.h"

int main() {

    rf::cache::CacheStore cache(
        "cache/main_file_cache.dat",
        "cache/main_file_cache.idx1"
    );

    rf::cache::CacheArchive archive =
        cache.readArchive(2635);

    std::cout
        << "\narchive size: "
        << archive.entry.size

        << "\npayload size: "
        << archive.payload.size()

        << "\n";

    return 0;
}
