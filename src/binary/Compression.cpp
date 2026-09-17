#include "Compression.h"

#include <array>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

#include <bzlib.h>
#include <zlib.h>

namespace eld::binary {

namespace {

bool hasGzipHeader(
    const std::vector<std::uint8_t>& bytes
) {
    return
        bytes.size() >= 2 &&
        bytes[0] == 0x1F &&
        bytes[1] == 0x8B;
}

bool hasBzip2Header(
    const std::vector<std::uint8_t>& bytes
) {
    return
        bytes.size() >= 4 &&
        bytes[0] == 'B' &&
        bytes[1] == 'Z' &&
        bytes[2] == 'h' &&
        bytes[3] >= '1' &&
        bytes[3] <= '9';
}

std::vector<std::uint8_t> compressGzip(
    const std::vector<std::uint8_t>& bytes
) {
    if (
        bytes.size() >
        std::numeric_limits<uInt>::max()
    ) {
        throw std::runtime_error(
            "Gzip input is too large"
        );
    }

    z_stream stream{};

    const int initializationResult =
        deflateInit2(
            &stream,
            Z_DEFAULT_COMPRESSION,
            Z_DEFLATED,
            16 + MAX_WBITS,
            8,
            Z_DEFAULT_STRATEGY
        );

    if (initializationResult != Z_OK) {
        throw std::runtime_error(
            "Failed to initialize Gzip compression"
        );
    }

    stream.next_in =
        const_cast<Bytef*>(
            reinterpret_cast<const Bytef*>(
                bytes.data()
            )
        );

    stream.avail_in =
        static_cast<uInt>(
            bytes.size()
        );

    std::vector<std::uint8_t> output;
    std::array<std::uint8_t, 4096> buffer{};

    int result = Z_OK;

    while (result != Z_STREAM_END) {
        stream.next_out =
            buffer.data();

        stream.avail_out =
            static_cast<uInt>(
                buffer.size()
            );

        result =
            deflate(
                &stream,
                Z_FINISH
            );

        if (
            result != Z_OK &&
            result != Z_STREAM_END
        ) {
            deflateEnd(&stream);

            throw std::runtime_error(
                "Gzip compression failed"
            );
        }

        const std::size_t written =
            buffer.size() -
            stream.avail_out;

        output.insert(
            output.end(),
            buffer.begin(),
            buffer.begin() +
                static_cast<std::ptrdiff_t>(
                    written
                )
        );
    }

    deflateEnd(&stream);

    return output;
}

std::vector<std::uint8_t> decompressGzip(
    const std::vector<std::uint8_t>& bytes
) {
    if (
        bytes.empty() ||
        bytes.size() >
            std::numeric_limits<uInt>::max()
    ) {
        throw std::runtime_error(
            "Invalid Gzip input"
        );
    }

    z_stream stream{};

    stream.next_in =
        const_cast<Bytef*>(
            reinterpret_cast<const Bytef*>(
                bytes.data()
            )
        );

    stream.avail_in =
        static_cast<uInt>(
            bytes.size()
        );

    const int initializationResult =
        inflateInit2(
            &stream,
            16 + MAX_WBITS
        );

    if (initializationResult != Z_OK) {
        throw std::runtime_error(
            "Failed to initialize Gzip decompression"
        );
    }

    std::vector<std::uint8_t> output;
    std::array<std::uint8_t, 4096> buffer{};

    int result = Z_OK;

    while (result == Z_OK) {
        stream.next_out =
            buffer.data();

        stream.avail_out =
            static_cast<uInt>(
                buffer.size()
            );

        result =
            inflate(
                &stream,
                Z_NO_FLUSH
            );

        const std::size_t written =
            buffer.size() -
            stream.avail_out;

        output.insert(
            output.end(),
            buffer.begin(),
            buffer.begin() +
                static_cast<std::ptrdiff_t>(
                    written
                )
        );
    }

    inflateEnd(&stream);

    if (result != Z_STREAM_END) {
        throw std::runtime_error(
            "Gzip decompression failed"
        );
    }

    return output;
}

std::vector<std::uint8_t> compressBzip2(
    const std::vector<std::uint8_t>& bytes
) {
    if (
        bytes.size() >
        std::numeric_limits<unsigned int>::max()
    ) {
        throw std::runtime_error(
            "Bzip2 input is too large"
        );
    }

    const std::size_t estimatedSize =
        bytes.size() +
        bytes.size() / 100 +
        601;

    if (
        estimatedSize >
        std::numeric_limits<unsigned int>::max()
    ) {
        throw std::runtime_error(
            "Bzip2 output would be too large"
        );
    }

    unsigned int outputSize =
        static_cast<unsigned int>(
            estimatedSize
        );

    std::vector<std::uint8_t> output(
        outputSize
    );

    const int result =
        BZ2_bzBuffToBuffCompress(
            reinterpret_cast<char*>(
                output.data()
            ),
            &outputSize,
            const_cast<char*>(
                reinterpret_cast<const char*>(
                    bytes.data()
                )
            ),
            static_cast<unsigned int>(
                bytes.size()
            ),
            1,
            0,
            30
        );

    if (result != BZ_OK) {
        throw std::runtime_error(
            "Bzip2 compression failed"
        );
    }

    output.resize(
        outputSize
    );

    return output;
}

std::vector<std::uint8_t> decompressBzip2(
    const std::vector<std::uint8_t>& bytes
) {
    if (bytes.empty()) {
        throw std::runtime_error(
            "Invalid Bzip2 input"
        );
    }

    std::vector<std::uint8_t> input;

    if (hasBzip2Header(bytes)) {
        input = bytes;
    }
    else {
        input.reserve(
            bytes.size() + 4
        );

        input.push_back('B');
        input.push_back('Z');
        input.push_back('h');
        input.push_back('1');

        input.insert(
            input.end(),
            bytes.begin(),
            bytes.end()
        );
    }

    if (
        input.size() >
        std::numeric_limits<unsigned int>::max()
    ) {
        throw std::runtime_error(
            "Bzip2 input is too large"
        );
    }

    bz_stream stream{};

    stream.next_in =
        const_cast<char*>(
            reinterpret_cast<const char*>(
                input.data()
            )
        );

    stream.avail_in =
        static_cast<unsigned int>(
            input.size()
        );

    const int initializationResult =
        BZ2_bzDecompressInit(
            &stream,
            0,
            0
        );

    if (initializationResult != BZ_OK) {
        throw std::runtime_error(
            "Failed to initialize Bzip2 decompression"
        );
    }

    std::vector<std::uint8_t> output;
    std::array<char, 4096> buffer{};

    int result = BZ_OK;

    while (result == BZ_OK) {
        stream.next_out =
            buffer.data();

        stream.avail_out =
            static_cast<unsigned int>(
                buffer.size()
            );

        result =
            BZ2_bzDecompress(
                &stream
            );

        const std::size_t written =
            buffer.size() -
            stream.avail_out;

        output.insert(
            output.end(),
            reinterpret_cast<const std::uint8_t*>(
                buffer.data()
            ),
            reinterpret_cast<const std::uint8_t*>(
                buffer.data() + written
            )
        );
    }

    BZ2_bzDecompressEnd(&stream);

    if (result != BZ_STREAM_END) {
        throw std::runtime_error(
            "Bzip2 decompression failed"
        );
    }

    return output;
}

}

CompressionType getCompressionType(
    const std::vector<std::uint8_t>& bytes
) {
    if (hasGzipHeader(bytes)) {
        return CompressionType::Gzip;
    }

    if (hasBzip2Header(bytes)) {
        return CompressionType::Bzip2;
    }

    return CompressionType::None;
}

std::vector<std::uint8_t> compress(
    const std::vector<std::uint8_t>& bytes,
    CompressionType type
) {
    switch (type) {
        case CompressionType::None:
            return bytes;

        case CompressionType::Gzip:
            return compressGzip(bytes);

        case CompressionType::Bzip2:
            return compressBzip2(bytes);
    }

    throw std::invalid_argument(
        "Unsupported compression type"
    );
}

std::vector<std::uint8_t> decompress(
    const std::vector<std::uint8_t>& bytes,
    CompressionType type
) {
    switch (type) {
        case CompressionType::None:
            return bytes;

        case CompressionType::Gzip:
            return decompressGzip(bytes);

        case CompressionType::Bzip2:
            return decompressBzip2(bytes);
    }

    throw std::invalid_argument(
        "Unsupported compression type"
    );
}

}
