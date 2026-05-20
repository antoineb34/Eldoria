#pragma once

#include <filesystem>
#include <vector>
#include "Buffer.h"

namespace core::io {

    class FileStore1 {
        public:
            explicit FileStore1(const std::filesystem::path& path);

            void read();

        private:
            const std::filesystem::path path;
            const Buffer data;
            const std::vector<Buffer> indexes;
        };

}
