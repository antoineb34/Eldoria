#include "Compression.h"

#include <iostream>
#include <zlib.h>

namespace rf::io {

CompressionType detectCompression(
    const std::vector<char>& payload
) {
    if (payload.size() < 3) {
        return CompressionType::Unknown;
    }

    unsigned char b0 = payload[0];
    unsigned char b1 = payload[1];
    unsigned char b2 = payload[2];

    if (
        b0 == 0x1F &&
        b1 == 0x8B &&
        b2 == 0x08
    ) {
        return CompressionType::Gzip;
    }

    return CompressionType::Unknown;
}

std::vector<char> decompressBzip2(
    const std::vector<char>& payload,
    size_t expectedSize
) {
    std::vector<char> output(
        expectedSize
    );

    unsigned int outputSize =
        static_cast<unsigned int>(
            output.size()
        );

    std::vector<char> bzipPayload;

    bzipPayload.push_back('B');
    bzipPayload.push_back('Z');
    bzipPayload.push_back('h');
    bzipPayload.push_back('1');

    bzipPayload.insert(
        bzipPayload.end(),
        payload.begin(),
        payload.end()
    );

    int result =
        BZ2_bzBuffToBuffDecompress(
            output.data(),
            &outputSize,
            bzipPayload.data(),
            static_cast<unsigned int>(
                bzipPayload.size()
            ),
            0,
            0
        );

    if (result != BZ_OK) {
        std::cerr
            << "BZip2 decompress failed: "
            << result
            << "\n";

        return {};
    }

    output.resize(
        outputSize
    );

    return output;
}

std::vector<char> decompressGzip(
    const std::vector<char>& payload
) {
    z_stream stream{};

    stream.next_in =
        reinterpret_cast<Bytef*>(
            const_cast<char*>(
                payload.data()
            )
        );

    stream.avail_in =
        payload.size();

    int inflateResult =
        inflateInit2(
            &stream,
            16 + MAX_WBITS
        );

    if (inflateResult != Z_OK) {
        std::cerr
            << "inflateInit2 failed: "
            << inflateResult
            << "\n";

        return {};
    }

    std::vector<char> decompressedPayload;

    char buffer[4096];

    do {
        stream.next_out =
            reinterpret_cast<Bytef*>(
                buffer
            );

        stream.avail_out =
            sizeof(buffer);

        inflateResult =
            inflate(
                &stream,
                Z_NO_FLUSH
            );

        if (
            inflateResult != Z_OK &&
            inflateResult != Z_STREAM_END
        ) {
            std::cerr
                << "inflate failed: "
                << inflateResult
                << "\n";

            inflateEnd(&stream);
            return {};
        }

        int bytesProduced =
            sizeof(buffer) -
            stream.avail_out;

        decompressedPayload.insert(
            decompressedPayload.end(),
            buffer,
            buffer + bytesProduced
        );

    } while (
        inflateResult != Z_STREAM_END
    );

    inflateEnd(&stream);

    return decompressedPayload;
}

}
