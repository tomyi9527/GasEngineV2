#include "main_app.h"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include "FreeImage.h"
#include "build_scene.h"
#include "online_resource_rule.h"
#include "utils/json_maker.h"
#include "utils/resource_manager.h"
#include "utils/set_locale.h"
#include "utils/string_util.h"

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
DEFINE_string(input, "",
              "input, examples:  1.:  '/data/gas2/girlwalk/girlwalk.fbx'   2.  "
              "'http://testdomain.com/gas2/girlwalk/girlwalk.fbx'  3. '@@AssetHub_Atc/123456'");
DEFINE_string(token, "", "arthub public token");
DEFINE_string(pcg_token, "", "arthub inner pcg token");
DEFINE_int32(network, 0, "0-oa, 1-idc, 2-qq");
DEFINE_string(output, "output/", "output screen shot file path");
DEFINE_int32(output_method, 0,
             "output mode for screen shot image. 0-local, 1-arthub_preview_update, 2-both");
DEFINE_bool(save_gif, false, "output screen animation gif if has animator");
DEFINE_bool(draw_bbox, false, "show debug bbox");
DEFINE_bool(draw_skeleton, false, "show debug skeleton");
DEFINE_double(play_speed, 1.0, "animation speed");
DEFINE_int32(fps, 30, "frames per second. won't affect play_speed");
DEFINE_double(play_start, 0.0f, "start timepoint");
DEFINE_double(min_duration, 1.0f, "end timepoint");
DEFINE_double(play_end, -1.0f, "end timepoint");
DEFINE_bool(rotate_snap, false, "animation snap with rotation if time is longer than 3 seconds");
DEFINE_int32(background, 2, "0-cubemap, 1-sph, 2-image, 3-purecolor");
DEFINE_int32(worker, 16, "gif quantizer worker count");

std::string TryAtAtLoad(const std::string& input);
std::string TryHttpLoad(std::string input);
std::string TryLocalLoad(std::string input);

int LoadFromArg(int argc, char* argv[], AppConfig& config) {
    AddOnlineResourceLoader();
    // analyse
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    // assign
    config.token = FLAGS_token;
    config.pcg_token = FLAGS_pcg_token;
    config.save_gif = FLAGS_save_gif;
    config.network_env = NetworkConditionType(FLAGS_network);
    config.locale_id = FLAGS_locale_id;
    config.logger_mode = LoggerMode(FLAGS_logger_mode);
    config.log_file = FLAGS_log_file;
    config.log_level = LogLevel(FLAGS_log_level);
    config.window_height = FLAGS_window_height;
    config.window_width = FLAGS_window_width;
    config.opengl_major = FLAGS_opengl_major;
    config.opengl_minor = FLAGS_opengl_minor;
    config.output = FLAGS_output;
    config.output_method = OutputMethod(FLAGS_output_method);
    config.draw_bbox = FLAGS_draw_bbox;
    config.draw_skeleton = FLAGS_draw_skeleton;
    config.min_duration = FLAGS_min_duration;
    config.fps = FLAGS_fps;
    config.play_speed = FLAGS_play_speed;
    config.play_start = FLAGS_play_start;
    config.play_end = FLAGS_play_end;
    config.rotate_snap = FLAGS_rotate_snap;
    config.background_type = FLAGS_background;
    config.worker = FLAGS_worker;
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
    // update config
    auto global_config = ArthubConfig::GetGlobalConfig();
    if (!config.token.empty()) {
        global_config->cred.credential_type = kArthubCredential_Token;
        global_config->cred.account_name = config.token;
        global_config->network_environment = config.network_env;
    } else if (!config.pcg_token.empty()) {
        global_config->network_environment = kNetworkCondition_IDC;
        global_config->cred.credential_type = kArthubCredential_PCG_Token;
        global_config->cred.account_name = config.pcg_token;
    }
    DetermineEndpoint(*global_config);
    LOG_DEBUG("endpoint is %s", global_config->endpoint.c_str());

    // try if input is @@ started
    std::string universal_resource_location = TryAtAtLoad(FLAGS_input);
    if (universal_resource_location.empty()) {
        // try if input is http(s)://... started
        universal_resource_location = TryHttpLoad(FLAGS_input);
        if (universal_resource_location.empty()) {
            universal_resource_location = TryLocalLoad(FLAGS_input);
            if (universal_resource_location.empty()) {
                ret++;
                LOG_ERROR("invalid input %s", FLAGS_input.c_str());
                return ret;
            }
        }
    }
    config.input = universal_resource_location;

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

std::string MIMEFromFileExt(std::string file_ext) {
    for (auto& c : file_ext) {
        c = std::tolower(c);
    }
    if (file_ext == ".png") {
        return "image/png";
    } else if (file_ext == ".gif") {
        return "image/gif";
    } else if (file_ext == ".jpeg" || file_ext == ".jpe" || file_ext == ".jpg") {
        return "image/jpeg";
    } else if (file_ext == ".bmp") {
        return "image/bmp";
    } else if (file_ext == ".tif" || file_ext == ".tiff") {
        return "image/tiff";
    } else if (file_ext == ".svg") {
        return "image/svg+xml";
    } else {
        return "";
    }
}

void SaveMemoryToArthub(const BasicConfig& config, const std::string& memory,
                        const std::string& force_ext) {
    // check input is arthub url
    ArthubAtAtResourceIdentifier id;
    id.FromString(config.input);
    if (!id.IsValid()) {
        SaveMemoryToLocal(config, memory, force_ext);
        return;
    }

    std::filesystem::path output_path = config.output;
    if (output_path.empty()) {
        output_path = id.file_name;
    }
    if (output_path.empty()) {
        output_path = "generated_preview_image" + force_ext;
    }
    if (!force_ext.empty()) {
        if (output_path.has_extension()) {
            std::string ext = output_path.extension().string();
            for (auto& c : ext) {
                c = std::tolower(c);
            }
            if (ext != force_ext) output_path.replace_extension(force_ext);
        } else {
            output_path += force_ext;
        }
    }

    std::string mime_type = MIMEFromFileExt(force_ext);

    auto global_config = ArthubConfig::GetGlobalConfig();
    ArthubClient client(global_config);
    GetSignatureReqItem upload_config;
    upload_config.object_id = id.asset_id;
    upload_config.object_meta = "preview_url";
    upload_config.file_name = output_path.filename().string();
    std::string raw_url = client.UploadContent(id.depot, upload_config, memory, mime_type);
    UpdateAssetByIDReqItem update_config;
    update_config.id = id.asset_id;
    update_config.preview_url = raw_url;
    client.UpdateAssetByID(id.depot, {update_config});
}
void SaveMemoryToBoth(const BasicConfig& config, const std::string& memory,
    const std::string& force_ext) {
    SaveMemoryToLocal(config, memory, force_ext);
    SaveMemoryToArthub(config, memory, force_ext);
}


const static std::string ext_1 = ".convertedFiles";
const static std::string ext_2 = ".structure.json";
// TODO(beanpliu): 支持加载scene.structure.json
const static std::string ext_3 = "scene.structure.json";

std::string TryAtAtLoad(const std::string& input) {
    ArthubAtAtResourceIdentifier resource_id;
    resource_id.FromString(FLAGS_input);
    resource_id.asset_meta = "display_url";

    if (!resource_id.IsValid()) {
        LOG_ERROR("input(depot, id, meta) is not valid.");
        return "";
    }

    auto global_config = ArthubConfig::GetGlobalConfig();
    ArthubClient cli(global_config);
    auto files = cli.GetFileList(resource_id.depot, resource_id.asset_id);
    if (files.size() <= 1) {
        LOG_ERROR("input(depot, id, meta) find no more than 1 file.");
        return "";
    }

    std::string converted_files;
    std::string structure_file;
    for (const auto& m : files) {
        if (EndsWith(m, ext_1)) {
            converted_files = m;
        }
        if (EndsWith(m, ext_2) && !EndsWith(m, ext_3)) {
            structure_file = m;
        }
    }

    if (!converted_files.empty()) {
        resource_id.file_name = converted_files;
        if (!CheckVersion(resource_id.ToString())) {
            return "";
        }
    }

    if (structure_file.empty()) {
        return "";
    }
    resource_id.file_name = structure_file;

    return resource_id.ToString();
}

std::string TryHttpLoad(std::string input) {
    Url input_parsed(input);

    if (input_parsed.protocol_.empty() || input_parsed.host_.empty() ||
        input_parsed.path_.empty()) {
        LOG_ERROR("input(protocol, host, path) is not valid.");
        return "";
    }
    // TODO(beanpliu): 对http来源也检查版本

    return input;
}

std::string TryLocalLoad(std::string input) {
    try {
        bool check_for_version_string = true;
        if (EndsWith(input, ext_1)) {
            input.erase(input.size() - ext_1.size());
        }
        if (EndsWith(input, ext_2)) {
            // 如果直接指定了 structure.json， 则不检查convertedFiles中的版本。
            check_for_version_string = false;
            input.erase(input.size() - ext_1.size());
        }

        if (check_for_version_string) {
            if (!CheckVersion(input + ext_1)) {
                return "";
            }
        }

        input += ext_2;
        if (!std::filesystem::is_regular_file(input)) {
            LOG_ERROR("input .structure.json file doesn't exsits: %s", input.c_str());
            return "";
        }
    } catch (std::runtime_error& e) {
        LOG_ERROR("input path check failed: %s", input.c_str());
        LOG_ERROR("%s", e.what());
    }
    return input;
}