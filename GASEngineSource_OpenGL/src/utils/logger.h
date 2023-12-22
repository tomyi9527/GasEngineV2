#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>

typedef void (*LogCallback)(int log_level, const char* msg);

enum LogLevel {
    kLogLevel_Debug = 0,
    kLogLevel_Info = 1,
    kLogLevel_Warning = 2,
    kLogLevel_Error = 3,
    kLogLevel_FATAL = 4,
    kLogLevelCount
};

enum LoggerMode { ToFile, ToConsole, ToBoth, ModeCount };

// define logger
extern LogCallback g_logger;
extern int g_min_logger_level;

#define MAX_LOG_SIZE 1024

#ifdef _WIN32
#define DO_LOG(log_level, format, ...)                                                          \
    do {                                                                                        \
        if (log_level >= g_min_logger_level && kLogLevel_Debug <= log_level &&                  \
            log_level <= kLogLevel_FATAL) {                                                     \
            if (g_logger != NULL) {                                                             \
                char buf[MAX_LOG_SIZE] = {0};                                                   \
                _snprintf_s(buf, MAX_LOG_SIZE - 1, "[%c]%s:%d %s "##format, "DIWEF"[log_level], \
                            __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);                   \
                g_logger(log_level, buf);                                                       \
            } else {                                                                            \
                printf("[%c]%s:%d %s "##format##"\n", "DIWEF"[log_level], __FILE__, __LINE__,   \
                       __FUNCTION__, ##__VA_ARGS__);                                            \
            }                                                                                   \
        }                                                                                       \
    } while (0)
#else
#define DO_LOG(log_level, format, ...)                                                            \
    do {                                                                                          \
        if (log_level >= g_min_logger_level && kLogLevel_Debug <= log_level &&                    \
            log_level <= kLogLevel_FATAL) {                                                       \
            if (g_logger != NULL) {                                                               \
                char buf[MAX_LOG_SIZE] = {0};                                                     \
                snprintf(buf, MAX_LOG_SIZE, "[%c]%s:%d %s " format, "DIWEF"[log_level], __FILE__, \
                         __LINE__, __FUNCTION__, ##__VA_ARGS__);                                  \
                g_logger(log_level, buf);                                                         \
            } else {                                                                              \
                printf("[%c]%s:%d %s " format "\n", "DIWEF"[log_level], __FILE__, __LINE__,       \
                       __FUNCTION__, ##__VA_ARGS__);                                              \
            }                                                                                     \
        }                                                                                         \
    } while (0)

#endif  // _WIN32

#define LOG_DEBUG(format, ...) DO_LOG(kLogLevel_Debug, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) DO_LOG(kLogLevel_Info, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) DO_LOG(kLogLevel_Warning, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) DO_LOG(kLogLevel_Error, format, ##__VA_ARGS__)
#define LOG_DOT() LOG_DEBUG("")

#define LOG_FATAL(format, ...)                          \
    do {                                                \
        DO_LOG(kLogLevel_FATAL, format, ##__VA_ARGS__); \
        /*abort();*/                                    \
    } while (0)

#define LOG_CHECK(cond, format, ...)                        \
    do {                                                    \
        if (false == (cond)) {                              \
            DO_LOG(kLogLevel_FATAL, format, ##__VA_ARGS__); \
            /*abort(); */                                   \
        }                                                   \
    } while (0)

// 初始化并安装L4CP的logger callback
void InitL4CPLogger(LoggerMode mode, LogLevel level, const std::string& log_file_name,
                    const std::string& locale_str = "zh_CN.UTF-8");
// 停止L4CP的logger
void ShutDownL4CPLogger();
