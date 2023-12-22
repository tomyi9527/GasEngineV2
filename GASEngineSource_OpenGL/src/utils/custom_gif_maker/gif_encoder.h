#pragma once
#include <array>
#include <vector>
#include <sstream>
#include "utils/image_data.h"
#include "neuralnet_quantization.h"

enum DitherKernel {
    kFalseFloydSteinberg,
    kFloydSteinberg,
    kStucki,
    kAtkinson,
    DitherKernelCount
};

class GifEncoder {
public:
    GifEncoder(int palette_size = 256): quantizer(palette_size) {
        for (auto& m : used_entry) { m = false; }
        first_frame = true;
        trans_index = 0;
        target_stream = &mem_stream;
    }
    GifEncoder(const GifEncoder&) = delete;
    GifEncoder(GifEncoder&&) = delete;
    GifEncoder& operator=(const GifEncoder&) = delete;
    GifEncoder& operator=(GifEncoder&&) = delete;
    ~GifEncoder() { Clear(); }

    void Begin();
    void AddFrame(RGBImageData&& data);
    void AddFrame(const RGBImageData& data);
    void Finish();
    std::string GetGeneratedFileContent() const;

    // return last analysed pixels result, for debug
    RGBImageData GetImageData() const;

    void Clear();

    // config
    int samples = 10;
    bool use_dither = false;
    DitherKernel method = kFloydSteinberg;
    bool serpentine = true;
    int loop = 0; // 0=infinate
    int delay = 3; // 1/100 s, 不是毫秒
    bool use_global_palette = false;
    bool flip_upside_down = true;

protected:
    void OverrideOutputStream(std::ostream& o);
    void _AnalyzePixels(RGBImageData&& image);
    void _AnalyzePixels(const RGBImageData& image);
    // dither will change data in img, but the changed result is not final output.
    void _DitherPixels(RGBImageData& img, DitherKernel kernel, bool serpentine);
    int _LookupPixel(uint8_t r, uint8_t g, uint8_t b);
    void _IndexImage(const RGBImageData& img);
    void _FlipUpsideDown();

    bool first_frame;
    int trans_index;
    RGBImageData img_data;
    NNQuantizer quantizer;
    std::vector<u8triple> color_tab;
    std::vector<uint8_t> indexed_pixels;
    std::array<bool, 256> used_entry;

protected:
    constexpr static int palsize = 7;
    constexpr static int color_depth = 8;
    void WriteByte(uint8_t);
    void WriteShort(short);
    void WriteBytes(const std::vector<u8triple>&);
    void WriteBytes(const std::string&);
    void WriteBytes(const std::vector<uint8_t> & data);
    void _WriteHeader();
    void _WriteLSD();
    void _WritePalette();
    void _WriteNetscapeExt();
    void _WriteGraphicCtrlExt();
    void _WriteImageDesc();
    void _WritePixels();
    void _WriteTrailer();

    bool finished = true;
    std::ostream* target_stream;
    std::stringstream mem_stream;
};

