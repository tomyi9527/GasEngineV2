#pragma once
#include <filesystem>
#include <string_view>
#include "utils/encoding_conv.h"
#include "utils/logger.h"

// filesystem此处真正访问了硬盘
inline std::vector<std::string> FindFileByPathHint(const std::string& filename,
                                                   const std::vector<std::string>& pathes) {
    std::vector<std::string> ret;
    std::error_code ec;
    try {
        if (std::filesystem::is_regular_file(filename, ec)) {
            ret.push_back(filename);
        }
    } catch (std::exception& e) {
        std::string exception_name = e.what();
#if defined WIN32 && !defined UNICODE 
        exception_name = GBKToUtf8(exception_name);
#endif
        LOG_ERROR("%s", exception_name.c_str());
    }
    for (const auto& m : pathes) {
        std::filesystem::path p(m);
        if (!m.empty()) {
            p /= filename;
        } else {
            p = filename;
        }
        if (std::filesystem::is_regular_file(p)) {
            ret.push_back(p.string());
        }
    }
    return ret;
}

inline std::string FrontOrDefault(const std::vector<std::string>& v) {
    if (v.empty())
        return std::string();
    else
        return v.front();
}