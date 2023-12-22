#pragma once
#include <string>
#include "utils/logger.h"

struct BasicConfig {
    std::string log_file = "screen_shot_log";
    LogLevel log_level = LogLevel::kLogLevel_Debug;
    LoggerMode logger_mode = LoggerMode::ToBoth;
    std::string locale_id = "zh_CN.UTF-8";
    int window_width = 800;
    int window_height = 600;
    int sampler = 2;
    int opengl_major = 4;
    int opengl_minor = 6;
    int background_type = 0;
    bool save_gif = false;
    float play_speed = 1.0f;
    float play_start = 0.0f;
    float play_end = -1.0f;
    float min_duration = 1.0f;
    int fps = 30;
    bool rotate_snap = true;
    bool draw_bbox = false;
    bool draw_skeleton = false;
    int worker = 8;
    std::string input;
    std::string output = "output/";
};