#include "Compression.h"

#include <bzlib.h>
#include <zlib.h>

namespace rf::compression {

CompressionType detectCompression(
    const std::vector<uint8_t>& payload
) {
    if (
        payload.size() >= 2 &&
        payload[0] == 0x1F &&
        payload[1] == 0x8B
    ) {
        return CompressionType::Gzip;
    }

    if (
        payload.size() >= 3 &&
        payload[0] == 'B' &&
        payload[1] == 'Z' &&
        payload[2] == 'h'
    ) {
        return CompressionType::Bzip2;
    }

    return CompressionType::Unknown;
}

std::vector<uint8_t> decompressGzip(
    const std::vector<uint8_t>& payload
) {
    z_stream stream {};

    stream.next_in =
        const_cast<Bytef*>(
            reinterpret_cast<const Bytef*>(payload.data())
        );

    stream.avail_in =
        static_cast<uInt>(payload.size());

    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
        return {};
    }

    std::vector<uint8_t> output;
    uint8_t buffer[4096];

    int result = Z_OK;

    while (result == Z_OK) {
        stream.next_out = buffer;
        stream.avail_out = sizeof(buffer);

        result = inflate(&stream, Z_NO_FLUSH);

        std::size_t written =
            sizeof(buffer) - stream.avail_out;

        output.insert(
            output.end(),
            buffer,
            buffer + written
        );
    }

    inflateEnd(&stream);

    if (result != Z_STREAM_END) {
        return {};
    }

    return output;
}

std::vector<uint8_t> decompressBzip2(
    const std::vector<uint8_t>& payload,
    std::size_t expectedSize
) {
    std::vector<uint8_t> input;

    if (
        payload.size() >= 3 &&
        payload[0] == 'B' &&
        payload[1] == 'Z' &&
        payload[2] == 'h'
    ) {
        input = payload;
    }
    else {
        input = { 'B', 'Z', 'h', '1' };
        input.insert(
            input.end(),
            payload.begin(),
            payload.end()
        );
    }

    std::vector<uint8_t> output(expectedSize);

    unsigned int outputSize =
        static_cast<unsigned int>(output.size());

    int result = BZ2_bzBuffToBuffDecompress(
        reinterpret_cast<char*>(output.data()),
        &outputSize,
        const_cast<char*>(
            reinterpret_cast<const char*>(input.data())
        ),
        static_cast<unsigned int>(input.size()),
        0,
        1
    );

    if (result != BZ_OK) {
        return {};
    }

    output.resize(outputSize);

    return output;
}

}