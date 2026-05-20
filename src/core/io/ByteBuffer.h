#pragma once

#include <cstdint>
#include <vector>

namespace rf::io {

class ByteBuffer {
public:

    explicit ByteBuffer(
        const std::vector<char>& data
    );

    uint8_t readU8();

    uint16_t readU16();

    uint32_t readU24();

    int readSmart();

    void skip(int bytes);

    int position() const;

    void setPosition(int position);

private:

    const std::vector<char>& data_;

    int position_ = 0;
};

}
