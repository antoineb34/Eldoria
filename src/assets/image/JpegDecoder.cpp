#include "JpegDecoder.h"

#include <cstddef>
#include <cstdio>
#include <limits>
#include <setjmp.h>
#include <stdexcept>
#include <string>

#include <jpeglib.h>

namespace eld::image {

namespace {

struct JpegErrorManager {
    jpeg_error_mgr manager;
    jmp_buf jumpBuffer;
    char message[JMSG_LENGTH_MAX]{};
};

void handleJpegError(
    j_common_ptr decoder
) {
    auto* error =
        reinterpret_cast<JpegErrorManager*>(
            decoder->err
        );

    error->manager.format_message(
        decoder,
        error->message
    );

    longjmp(
        error->jumpBuffer,
        1
    );
}

}

Image JpegDecoder::decode(
    const std::vector<std::uint8_t>& bytes
) const {
    if (bytes.empty()) {
        throw std::invalid_argument(
            "JPEG payload is empty"
        );
    }

    if (
        bytes.size() >
        std::numeric_limits<unsigned long>::max()
    ) {
        throw std::invalid_argument(
            "JPEG payload is too large"
        );
    }

    Image image;

    jpeg_decompress_struct decoder{};
    JpegErrorManager error{};

    decoder.err =
        jpeg_std_error(
            &error.manager
        );

    error.manager.error_exit =
        handleJpegError;

    if (
        setjmp(
            error.jumpBuffer
        ) != 0
    ) {
        if (decoder.mem != nullptr) {
            jpeg_destroy_decompress(
                &decoder
            );
        }

        throw std::runtime_error(
            "JPEG decoding failed: " +
            std::string(error.message)
        );
    }

    jpeg_create_decompress(
        &decoder
    );

    jpeg_mem_src(
        &decoder,
        bytes.data(),
        static_cast<unsigned long>(
            bytes.size()
        )
    );

    jpeg_read_header(
        &decoder,
        TRUE
    );

    decoder.out_color_space =
        JCS_RGB;

    jpeg_start_decompress(
        &decoder
    );

    if (
        decoder.output_width >
            std::numeric_limits<std::uint16_t>::max() ||
        decoder.output_height >
            std::numeric_limits<std::uint16_t>::max()
    ) {
        jpeg_destroy_decompress(
            &decoder
        );

        throw std::runtime_error(
            "JPEG dimensions are unsupported"
        );
    }

    image.width =
        static_cast<std::uint16_t>(
            decoder.output_width
        );

    image.height =
        static_cast<std::uint16_t>(
            decoder.output_height
        );

    image.pixels.resize(
        static_cast<std::size_t>(
            image.width
        ) *
        static_cast<std::size_t>(
            image.height
        )
    );

    JSAMPARRAY scanline =
        decoder.mem->alloc_sarray(
            reinterpret_cast<j_common_ptr>(
                &decoder
            ),
            JPOOL_IMAGE,
            decoder.output_width *
                decoder.output_components,
            1
        );

    while (
        decoder.output_scanline <
        decoder.output_height
    ) {
        const std::size_t row =
            decoder.output_scanline;

        jpeg_read_scanlines(
            &decoder,
            scanline,
            1
        );

        for (
            std::size_t column = 0;
            column < decoder.output_width;
            column++
        ) {
            const std::size_t source =
                column * 3;

            const std::size_t destination =
                row *
                    decoder.output_width +
                column;

            image.pixels[destination] =
                RgbaPixel{
                    scanline[0][source],
                    scanline[0][source + 1],
                    scanline[0][source + 2],
                    255
                };
        }
    }

    jpeg_finish_decompress(
        &decoder
    );

    jpeg_destroy_decompress(
        &decoder
    );

    return image;
}

}
