#include "dump/AssetDumpWriter.h"

#include <cctype>
#include <exception>
#include <fstream>
#include <iomanip>

namespace eld::elforge {

bool writeAssetDump(
    const std::filesystem::path& path,
    const std::function<void(std::ostream&)>& writer,
    std::string& error
) {
    try {
        const std::filesystem::path parent =
            path.parent_path();

        if (!parent.empty()) {
            std::filesystem::create_directories(parent);
        }

        std::ofstream output(
            path,
            std::ios::out | std::ios::trunc
        );

        if (!output.is_open()) {
            error = "could not open dump file";
            return false;
        }

        writer(output);

        if (!output.good()) {
            error = "failed while writing dump file";
            return false;
        }

        return true;
    }
    catch (const std::exception& exception) {
        error = exception.what();
        return false;
    }
}


void writeHexDump(
    std::ostream& output,
    const void* data,
    std::size_t size
) {
    constexpr std::size_t BytesPerLine = 16;

    const auto* bytes =
        static_cast<const unsigned char*>(data);

    const std::ios::fmtflags originalFlags =
        output.flags();

    const char originalFill =
        output.fill();

    for (
        std::size_t offset = 0;
        offset < size;
        offset += BytesPerLine
    ) {
        output
            << std::hex
            << std::uppercase
            << std::setfill('0')
            << std::setw(8)
            << offset
            << "  ";

        for (
            std::size_t column = 0;
            column < BytesPerLine;
            ++column
        ) {
            const std::size_t index =
                offset + column;

            if (index < size) {
                output
                    << std::setw(2)
                    << static_cast<unsigned int>(
                        bytes[index]
                    )
                    << ' ';
            }
            else {
                output << "   ";
            }

            if (column == 7) {
                output << ' ';
            }
        }

        output << " |";

        for (
            std::size_t column = 0;
            column < BytesPerLine &&
                offset + column < size;
            ++column
        ) {
            const unsigned char value =
                bytes[offset + column];

            output << (
                std::isprint(value)
                    ? static_cast<char>(value)
                    : '.'
            );
        }

        output << "|\n";
    }

    output.flags(originalFlags);
    output.fill(originalFill);
}

}
