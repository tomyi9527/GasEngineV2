#include "main_app.h"
#include <cctype>
#include <filesystem>
#include <memory>
#include <string>
#include "build_scene.h"
#include "utils/string_util.h"
#include "utils/set_locale.h"

DEFINE_int32(window_width, 800,
             "if program runs under normal mode, specify the initial window width");
DEFINE_int32(window_height, 600,
             "if program runs under normal mode, specify the initial window height");
DEFINE_int32(log_level, 1, "controls the log level. 0-debug, 1-info, 2-warning, 3-error, 4-fatal");
DEFINE_int32(logger_mode, 1, "controls the logger output mode. 0-to_file, 1-to_console, 2-to_both");
DEFINE_int32(opengl_major, 4, "spcify the opengl context major version to create.");
DEFINE_int32(opengl_minor, 6, "spcify the opengl context minor version to create.");
DEFINE_string(locale_id, "zh_CN.UTF-8", "it's usually set to some utf-8 locale");
DEFINE_string(log_file, "screen_shot_log",
              "name of the output log file (when logger_mode is not 1)");
DEFINE_string(input, "", "gas2 model *.structure.json file path");
DEFINE_string(output, "output/", "output screen shot file path");
DEFINE_bool(draw_bbox, false, "show debug bbox");
DEFINE_double(play_speed, 1.0, "animation speed");
DEFINE_int32(fps, 30, "frames per second. won't affect play_speed");

int LoadFromArg(int argc, char* argv[], AppConfig& config) {
    // analyse
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    // assign
    config.input = FLAGS_input;
    config.output = FLAGS_output;
    config.locale_id = FLAGS_locale_id;
    config.logger_mode = LoggerMode(FLAGS_logger_mode);
    config.log_file = FLAGS_log_file;
    config.log_level = LogLevel(FLAGS_log_level);
    config.window_height = FLAGS_window_height;
    config.window_width = FLAGS_window_width;
    config.opengl_major = FLAGS_opengl_major;
    config.opengl_minor = FLAGS_opengl_minor;
    config.draw_bbox = FLAGS_draw_bbox;
    config.play_speed = FLAGS_play_speed;
    config.fps = FLAGS_fps;
    SetLocale(config.locale_id);
    // check
    int ret = 0;
    if (config.window_height <= 0 || config.window_height >= 100000) {
        LOG_ERROR("input window_height malformed: %d", config.window_height);
        ret++;
    }
    if (config.window_width <= 0 || config.window_width >= 100000) {
        LOG_ERROR("input window_width malformed: %d", config.window_width);
        ret++;
    }
    if (FLAGS_log_level < 0 || FLAGS_log_level >= kLogLevelCount) {
        LOG_ERROR("input log_level malformed: %d", FLAGS_log_level);
        ret++;
    }
    if (FLAGS_logger_mode < 0 || FLAGS_logger_mode >= LoggerMode::ModeCount) {
        LOG_ERROR("input logger_mode malformed: %d", FLAGS_logger_mode);
        ret++;
    }
    try {
        std::locale tmp(config.locale_id);
    } catch (std::runtime_error& e) {
        LOG_ERROR("input locale_id malformed: %s", config.locale_id.c_str());
        LOG_ERROR("%s", e.what());
        ret++;
    }
    if (config.opengl_major > 5 || config.opengl_major < 0) {
        LOG_ERROR("input opengl_major malformed: %d", config.opengl_major);
        ret++;
    }
    if (config.opengl_minor > 8 || config.opengl_minor < 0) {
        LOG_ERROR("input opengl_minor malformed: %d", config.opengl_minor);
        ret++;
    }
    try {
        const static std::string ext_1 = ".convertedFiles";
        const static std::string ext_2 = ".structure.json";
        bool check_for_version_string = true;
        if (EndsWith(config.input, ext_1)) {
            config.input.erase(config.input.size() - ext_1.size());
        }
        if (EndsWith(config.input, ext_2)) {
            // 如果直接指定了 structure.json， 则不检查convertedFiles中的版本。
            check_for_version_string = false;
            config.input.erase(config.input.size() - ext_1.size());
        }

        if (check_for_version_string) {
            if (!CheckVersion(config.input + ext_1)) {
                ret++;
                return ret;
            }
        }

        config.input += ext_2;
        if (!std::filesystem::is_regular_file(config.input)) {
            LOG_ERROR("input .strcuture.json file doesn't exsits: %s", config.input.c_str());
            ret++;
            return ret;
        }
    } catch (std::runtime_error& e) {
        LOG_ERROR("input path check failed: %s", config.input.c_str());
        LOG_ERROR("%s", e.what());
        ret++;
    }
    try {
        auto output_path = std::filesystem::path(config.output);
        if (output_path.has_parent_path() &&
            !std::filesystem::is_directory(output_path.parent_path())) {
            std::filesystem::create_directories(output_path.parent_path());
        }
        if (std::filesystem::is_directory(output_path)) {
            output_path /= "output.png";
        }
        config.output = output_path.string();
    } catch (std::runtime_error& e) {
        LOG_ERROR("output path check failed: %s", config.output.c_str());
        LOG_ERROR("%s", e.what());
        ret++;
    }
    return ret;
}