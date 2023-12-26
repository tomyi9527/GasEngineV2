#pragma once

#include <iostream>
#include <string>

inline constexpr bool s_force_corrupt_on_fatal = false;
inline constexpr bool s_print_level = true;
inline constexpr bool s_print_reason = true;

// 用于标识出错范围
enum ErrReason {
    OleParserError,
    MaxBinaryParserError,
    GAS2ExporterError,
    MaxClassImplError,
    ApplicationError,
    None
};

enum ErrLevel {
    LEVEL_DEBUG = 0,   // unimportant information
    LEVEL_INFO = 1,    // some important log
    LEVEL_WARNING = 2, // will influence output, maybe result in error
    LEVEL_ERROR = 3,   // will cause error
    LEVEL_FATAL = 4,   // can not generate useful result
    LevelCount
};

inline constexpr ErrLevel s_min_level = LEVEL_INFO;

inline const char* ReasonToString(ErrReason reason) {
    switch (reason) {
    case OleParserError:
        return "Ole(cfb) Parser Error";
    case MaxBinaryParserError:
        return "Binary Parser Error";
    case GAS2ExporterError:
        return "GAS2 Exporter Error";
    case ApplicationError:
        return "Application Error";
    default:
        return "Unknown Error";
    }
}

inline void PrintError(ErrReason reason, ErrLevel level, const std::string& content, bool throw_runtime_error = false) {
    static const char* level_str[LevelCount] = { "Debug", "Info", "Warning", "Error", "Fatal"};
    if (throw_runtime_error || 
        (level == LEVEL_FATAL && s_force_corrupt_on_fatal)
        ) {
        throw std::runtime_error(std::string(ReasonToString(reason)) + ": " + content);
    }
    if (level < s_min_level) {
        return;
    }
    if (s_print_level)
        std::cerr << "[" << level_str[level] << "] ";
    if (s_print_reason && reason != None)
        std::cerr << ReasonToString(reason) << ": ";
    std::cerr << content << std::endl;
}

inline void PrintInfo(const std::string& info) {
    PrintError(None, LEVEL_INFO, info);
}