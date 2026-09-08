#include "Cache.h"

#include <algorithm>
#include <stdexcept>

namespace eld::cache {

Cache::Cache(
    const std::filesystem::path& rootPath
)
    : dataPath_(
          rootPath / "main_file_cache.dat"
      ),
      indexes_{
          Index{
              IndexId::Config,
              rootPath / "main_file_cache.idx0"
          },
          Index{
              IndexId::Models,
              rootPath / "main_file_cache.idx1"
          },
          Index{
              IndexId::Animations,
              rootPath / "main_file_cache.idx2"
          },
          Index{
              IndexId::Midi,
              rootPath / "main_file_cache.idx3"
          },
          Index{
              IndexId::Maps,
              rootPath / "main_file_cache.idx4"
          }
      } {
    if (
        !std::filesystem::is_regular_file(
            dataPath_
        )
    ) {
        throw std::runtime_error(
            "Cache data file does not exist: " +
            dataPath_.string()
        );
    }

    for (const Index& index : indexes_) {
        if (
            !std::filesystem::is_regular_file(
                index.path
            )
        ) {
            throw std::runtime_error(
                "Cache index file does not exist: " +
                index.path.string()
            );
        }
    }
}

Store Cache::open(
    IndexId indexId
) const {
    const auto iterator = std::find_if(
        indexes_.cbegin(),
        indexes_.cend(),
        [indexId](const Index& index) {
            return index.id == indexId;
        }
    );

    if (iterator == indexes_.cend()) {
        throw std::out_of_range(
            "Cache index does not exist"
        );
    }

    return Store{
        dataPath_,
        *iterator
    };
}

const std::vector<Index>& Cache::list() const {
    return indexes_;
}

}
