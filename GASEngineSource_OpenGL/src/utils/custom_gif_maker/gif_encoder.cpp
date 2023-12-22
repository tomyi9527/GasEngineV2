#include "gif_encoder.h"

#include <array>
#include <utility>
#include <tuple>
#include <vector>
#include "lzw_encoder.h"

// means  weight, weight_sum, dx, dy
using KernelDataType = std::vector<std::tuple<int, int, int, int>>;

namespace kernels {
static KernelDataType FalseFloydSteinberg = {
        { 3, 8, 1, 0 },
        { 3, 8, 0, 1 },
        { 2, 8, 1, 1 }
};
static KernelDataType FloydSteinberg = {
        {7, 16, 1, 0},
        {3, 16, -1, 1},
        {5, 16, 0, 1},
        {1, 16, 1, 1}
};
static KernelDataType Stucki = {
        {8, 42, 1, 0},
        {4, 42, 2, 0},
        {2, 42, -2, 1},
        {4, 42, -1, 1},
        {8, 42, 0, 1},
        {4, 42, 1, 1},
        {2, 42, 2, 1},
        {1, 42, -2, 2},
        {2, 42, -1, 2},
        {4, 42, 0, 2},
        {2, 42, 1, 2},
        {1, 42, 2, 2}
};
static KernelDataType Atkinson = {
        {1, 8, 1, 0},
        {1, 8, 2, 0},
        {1, 8, -1, 1},
        {1, 8, 0, 1},
        {1, 8, 1, 1},
        {1, 8, 0, 2}
};
}

static std::array<KernelDataType, DitherKernelCount> all_kernels = {
    kernels::FalseFloydSteinberg,
    kernels::FloydSteinberg,
    kernels::Stucki,
    kernels::Atkinson
};

void GifEncoder::_DitherPixels(RGBImageData& img, DitherKernel kernel, bool serpentine) {
    if (kernel == DitherKernelCount) {
        kernel = kFloydSteinberg;
    }

    const KernelDataType& ds = all_kernels[kernel];
    int index = 0,
        height = img.height,
        width = img.width;
    uint8_t* data = img.data.data();
    int direction = serpentine ? -1 : 1;

    indexed_pixels.resize(img.data.size()/ 3);

    for (int y = 0; y < height; y++) {

        if (serpentine) direction = direction * -1;

        for (int x = (direction == 1 ? 0 : width - 1), xend = (direction == 1 ? width : 0); x != xend; x += direction) {

            index = (y * width) + x;
            // Get original colour
            int idx3 = index * 3;
            uint8_t r1 = data[idx3];
            uint8_t g1 = data[idx3 + 1];
            uint8_t b1 = data[idx3 + 2];

            // Get converted colour
            int idx = _LookupPixel(r1, g1, b1);
            used_entry[idx] = true;
            indexed_pixels[index] = idx;
            uint8_t r2 = color_tab[idx].v[0];
            uint8_t g2 = color_tab[idx].v[1];
            uint8_t b2 = color_tab[idx].v[2];

            int er = (int)r1 - (int)r2;
            int eg = (int)g1 - (int)g2;
            int eb = (int)b1 - (int)b2;

            for (int i = (direction == 1 ? 0 : ds.size() - 1), end = (direction == 1 ? ds.size() : 0); i != end; i += direction) {
                int x1 = std::get<2>(ds[i]); // *direction;  //  Should this by timesd by direction?..to make the kernel go in the opposite direction....got no idea....
                int y1 = std::get<3>(ds[i]);
                if (x1 + x >= 0 && x1 + x < width && y1 + y >= 0 && y1 + y < height) {
                    int weight = std::get<0>(ds[i]);
                    int weight_sum = std::get<1>(ds[i]);
                    idx3 = index + x1 + (y1 * width);
                    idx3 *= 3;

                    data[idx3] = std::clamp(data[idx3] + er * weight / weight_sum, 0, 255);
                    data[idx3 + 1] = std::clamp(data[idx3 + 1] + eg * weight / weight_sum, 0, 255);
                    data[idx3 + 2] = std::clamp(data[idx3 + 2] + eb * weight / weight_sum, 0, 255);
                }
            }
        }
    }
}

void GifEncoder::OverrideOutputStream(std::ostream & o) {
    target_stream = &o;
}

std::string GifEncoder::GetGeneratedFileContent() const {
    return mem_stream.str();
}

void GifEncoder::Clear() {
    color_tab.clear();
    indexed_pixels.clear();
}

void GifEncoder::Begin() {
    if (finished) {
        mem_stream.str("");
        _WriteHeader();
    }
    finished = false;
    first_frame = true;
    for (auto& m : used_entry) { m = false; }
}

void GifEncoder::AddFrame(RGBImageData && data) {
    _AnalyzePixels(std::move(data));
    if (first_frame) {
        _WriteLSD();
        _WritePalette();
        if (loop >= 0) {
            _WriteNetscapeExt();
        }
    }
    _WriteGraphicCtrlExt();
    _WriteImageDesc();
    if (!first_frame && !use_global_palette)
        _WritePalette();
    _WritePixels();
    first_frame = false;
}

void GifEncoder::AddFrame(const RGBImageData & data) {
    RGBImageData copy = data;
    AddFrame(std::move(copy));
}

void GifEncoder::Finish() {
    if (!finished)
        _WriteTrailer();
    finished = true;
}

void GifEncoder::_AnalyzePixels(RGBImageData && image) {
    img_data = std::move(image);
    if (color_tab.empty() || quantizer.FittingRatio(img_data) > 0.05) {
        quantizer.Quantize(img_data, {}, samples);
        color_tab = quantizer.GetPalette();
    }
    if (use_dither) {
        _DitherPixels(img_data, method, serpentine);
    } else {
        _IndexImage(img_data);
    }
    // image content is now in indexed_pixels
    if (flip_upside_down) {
        _FlipUpsideDown();
    }
}

void GifEncoder::_AnalyzePixels(const RGBImageData& image) {
    RGBImageData copy = image;
    _AnalyzePixels(std::move(copy));
}

RGBImageData GifEncoder::GetImageData() const {
    RGBImageData out;
    out.width = img_data.width;
    out.height = img_data.height;
    out.data.resize(img_data.data.size());
    for (int i = 0; i < indexed_pixels.size(); ++i) {
        out.data[3 * i + 0] = color_tab[indexed_pixels[i]].v[0];
        out.data[3 * i + 1] = color_tab[indexed_pixels[i]].v[1];
        out.data[3 * i + 2] = color_tab[indexed_pixels[i]].v[2];
    }
    return out;
}

void GifEncoder::WriteByte(uint8_t data) {
    target_stream->write(reinterpret_cast<const char*>(&data), sizeof(uint8_t));
}

void GifEncoder::WriteShort(short data) {
    target_stream->write(reinterpret_cast<const char*>(&data), sizeof(short));
}

void GifEncoder::WriteBytes(const std::vector<u8triple>& data) {
    target_stream->write(reinterpret_cast<const char*>(data.data()), sizeof(u8triple) * data.size());
}

void GifEncoder::WriteBytes(const std::string & data) {
    target_stream->write(reinterpret_cast<const char*>(data.data()), sizeof(char) * data.size());
}

void GifEncoder::WriteBytes(const std::vector<uint8_t> & data) {
    target_stream->write(reinterpret_cast<const char*>(data.data()), sizeof(uint8_t) * data.size());
}

void GifEncoder::_WriteHeader() {
    WriteBytes("GIF89a");
}

void GifEncoder::_WriteLSD() {
    WriteShort(img_data.width);
    WriteShort(img_data.height);

    uint8_t val =
        // 1 : global color table flag = 1 (gct used)
        0x80 |
        0x70 | // 2-4 : color resolution = 7
        0x00 | // 5 : gct sort flag = 0
        palsize // 6-8 : gct size
        ;

    WriteByte(val);

    WriteByte(0); // background color index
    WriteByte(0); // pixel aspect ratio - assume 1:1
}

void GifEncoder::_WritePalette() {
    WriteBytes(color_tab);
    int n = 256 - color_tab.size();
    for (int i = 0; i < 3 * n; i++)
        WriteByte(0);
}

void GifEncoder::_WriteNetscapeExt() {
    WriteByte(0x21); // extension introducer
    WriteByte(0xff); // app extension label
    WriteByte(0x0b); // block size
    WriteBytes("NETSCAPE2.0"); // app id + auth code
    WriteByte(0x03); // sub-block size
    WriteByte(0x01); // loop sub-block id
    WriteShort(loop); // loop count (extra iterations, 0=repeat forever)
    WriteByte(0x00); // block terminator
}

void GifEncoder::_WriteGraphicCtrlExt() {
    WriteByte(0x21); // extension introducer
    WriteByte(0xf9); // GCE label
    WriteByte(0x04); // data block size

    // 暂时先不支持transparent
    int transp, disp;
    if (true) {
        transp = 0;
        disp = 0; // dispose = no action
    } else {
        transp = 1;
        disp = 2; // force clear if using transparent color
    }

    //if (dispose >= 0) {
    //    disp = dispose & 7; // user override
    //}
    disp <<= 2;

    // packed fields
    WriteByte(
        0 | // 1:3 reserved
        disp | // 4:6 disposal
        0 | // 7 user input - 0 = none
        transp // 8 transparency flag
    );

    WriteShort(delay); // delay x 1/100 sec
    WriteByte(trans_index); // transparent color index
    WriteByte(0x00); // block terminator
}

void GifEncoder::_WriteImageDesc() {
    WriteByte(0x2c); // image separator
    WriteShort(0); // image position x,y = 0,0
    WriteShort(0);
    WriteShort(img_data.width); // image size
    WriteShort(img_data.height);

    // packed fields
    if (first_frame || use_global_palette) {
        // no LCT - GCT is used for first (or only) frame
        WriteByte(0x00);
    } else {
        // specify normal LCT
        WriteByte(
            0x80 | // 1 local color table 1=yes
            0 | // 2 interlace - 0=no
            0 | // 3 sorted - 0=no
            0 | // 4-5 reserved
            palsize // 6-8 size of color table
        );
    }
}

void GifEncoder::_WritePixels() {
    // get original LZW bytes
    LZWEncoder encoded(img_data.width, img_data.height, indexed_pixels, 8);
    encoded.encode(*target_stream);
}

void GifEncoder::_WriteTrailer() {
    WriteByte(0x3b); // gif trailer
}

int GifEncoder::_LookupPixel(uint8_t r, uint8_t g, uint8_t b) {
    if (color_tab.empty()) {
        return -1;
    }

    return quantizer.LookupPixel(r, g, b);
}

void GifEncoder::_IndexImage(const RGBImageData & img) {
    int n_pixels = img.data.size() / 3;
    indexed_pixels.resize(n_pixels);
    int k = 0;
    for (int j = 0; j < n_pixels; j++) {
        int index = _LookupPixel(
            img.data[k],
            img.data[k+1],
            img.data[k+2]
        );
        k += 3;
        used_entry[index] = true;
        indexed_pixels[j] = index;
    }
}

void GifEncoder::_FlipUpsideDown() {
    std::vector<uint8_t> flipped(indexed_pixels.size(), 0);
    for (int i = 0; i < img_data.height; ++i) {
        for (int j = 0; j < img_data.width; ++j) {
            int index_src = (i * img_data.width) + j;
            int index_dest = ((img_data.height - i - 1) * img_data.width) + j;
            flipped[index_dest] = indexed_pixels[index_src];
        }
    }
    std::swap(flipped, indexed_pixels);
}
