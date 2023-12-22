#pragma once

struct GifMakerConfig {
    GifMakerConfig() {}
    GifMakerConfig(int width_in, int height_in, int delay_ms_in)
        : width(width_in), height(height_in), delay_ms(delay_ms_in) {}
    void SetDelay(int delay_ms_in) {
        delay_ms = delay_ms_in;
        fps = std::round(1000.0 / delay_ms);
    }
    void SetFrameRate(int fps_in) {
        fps = fps_in;
        delay_ms = std::round(1000.0 / fps);
    }
    // -1: no repeat  0: repeat infinately
    void SetRepeat(int repeat_in = 0) { repeat = repeat_in; }
    void SetQuality(int quality_in) { quality = std::clamp(quality_in, 1, 20); }

    int width = 0;
    int height = 0;
    float delay_ms = 30;
    int fps = 33;
    int repeat = 0;
    int quality = 10;
    int disposal = 0;
    bool async = false;
    int pool_size = 8;
};

#ifndef GIF_MAKER_USING_MAGICK
// #warning "gifmaker using freeimage"
#include "gif_maker.h.multithread.freeimage"
#else
// #warning "gifmaker using magick"
#include "gif_maker.h.magick"
#endif