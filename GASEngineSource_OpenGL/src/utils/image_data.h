#pragma once
#include <vector>

struct u8triple {
    uint8_t v[3] = { 0, 0, 0 };
};

class RGBImageData;
// 一般来说 width * height * 4 == data.size()
class RGBAImageData {
public:
    std::vector<uint8_t> data;
    uint32_t width = 0;
    uint32_t height = 0;
};

// 一般来说 width * height * 3 == data.size()
class RGBImageData {
public:
    std::vector<uint8_t> data;
    uint32_t width = 0;
    uint32_t height = 0;

    static RGBImageData ConvertFrom(const RGBAImageData& in) {
        RGBImageData out;
        out.width = in.width;
        out.height = in.height;
        int total = out.width * out.height;
        out.data.resize(total * 3);
        for (int i = 0; i < total; ++i) {
            out.data[3 * i] = in.data[4 * i];
            out.data[3 * i + 1] = in.data[4 * i + 1];
            out.data[3 * i + 2] = in.data[4 * i + 2];
        }
        return out;
    }
};