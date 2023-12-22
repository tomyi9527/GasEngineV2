#pragma once
#include <algorithm>
#include <fstream>
#include <vector>
#include <set>
#include <cmath>
#include "utils/image_data.h"
#include "gif_encoder.h"

class GifMaker {
 public:
     GifMaker(){}
    GifMaker(const GifMakerConfig& in) { Init(in); }
     ~GifMaker() { Destroy(); }

     void Init(const GifMakerConfig& in) {
         Destroy();
         config = in;

         encoder.loop = 0;
         encoder.samples = in.quality;

         current_frame_time = 0.0f;
         step_frame_time = std::min(3000.0f, config.delay_ms);
         encoder.Begin();
     }
     // will discard alpha
     bool AddFrame(const RGBAImageData& src) {
         RGBImageData converted = RGBImageData::ConvertFrom(src);
         return AddFrame(converted);
     }
     bool AddFrame(const RGBImageData& src) {
         current_frame_time += step_frame_time;
         uint64_t dw_frame_time = std::round(current_frame_time - current_frame_ms);
         current_frame_ms += dw_frame_time;

         encoder.delay = dw_frame_time / 10;
         encoder.AddFrame(src);

         return true;
     }
     std::string SaveToMem() {
         std::string ret;
         encoder.Finish();
         ret = encoder.GetGeneratedFileContent();
         return ret;
     }

     void SaveToFile(const std::string& file) {
         std::ofstream fs(file, std::ios::out | std::ios::binary);
         if (!fs.is_open()) {
             return;
         }
         std::string to_save = SaveToMem();
         fs.write(to_save.data(), to_save.size());
     }

     void Destroy() {
         encoder.Clear();
         current_frame_time = 0.0f;
         current_frame_ms = 0;
     }


 protected:
    double current_frame_time = 0.0f;  // ms
    double step_frame_time = 0.0f;
    uint64_t current_frame_ms = 0;  // ms
    GifMakerConfig config;
    GifEncoder encoder;
};

