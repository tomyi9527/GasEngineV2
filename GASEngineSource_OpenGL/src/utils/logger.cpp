#include "utils/logger.h"
#include "log4cplus/configurator.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/fileappender.h"
#include "log4cplus/initializer.h"
#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"
#include "log4cplus/loglevel.h"

LogCallback g_logger = nullptr;
int g_min_logger_level = kLogLevel_Debug;

constexpr char l4cp_pattern[] = "%d{%y-%m-%d %H:%M:%S.%q} %m%n";
log4cplus::Logger logger;

inline void L4CPLoggerCallback(int log_level, const char* msg) {
    LOG4CPLUS_INFO_FMT(logger, "%s", msg);
}

std::unique_ptr<log4cplus::Initializer>
    initializer;  // RAII的线程安全的init和deinit，但生存期需要限制在main函数内
log4cplus::BasicConfigurator config;

class Log4CplusLogger {
 public:
    Log4CplusLogger(LoggerMode mode, LogLevel level, const std::string& log_file_name,
                    const std::string& locale_str) {}

    ~Log4CplusLogger() {}

 protected:
};

void InitL4CPLogger(LoggerMode mode, LogLevel level, const std::string& log_file_name,
                    const std::string& locale_str) {
    if (initializer == nullptr) {
        initializer.reset(new log4cplus::Initializer);
        config.configure();
        logger = log4cplus::Logger::getRoot();
        // logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("SERVER"));
    }

    // 处理console的输出
    if (mode == ToFile) {
        logger.removeAllAppenders();
    } else {
        auto tmp = logger.getAllAppenders();
        if (tmp.size()) {
            std::unique_ptr<log4cplus::Layout> layout(new log4cplus::PatternLayout(l4cp_pattern));
            tmp.front()->setLayout(std::move(layout));
        }
    }

    if (mode == ToFile || mode == ToBoth) {
        // 处理文件的输出
        log4cplus::SharedAppenderPtr ptr(
            new log4cplus::DailyRollingFileAppender(log_file_name)  //, log4cplus::DAILY, true)
        );

        auto casted_ptr = dynamic_cast<log4cplus::DailyRollingFileAppender*>(ptr.get());
        if (casted_ptr != nullptr) {
            casted_ptr->imbue(std::locale(locale_str));
        }

        // ptr->setName("file");
        std::unique_ptr<log4cplus::Layout> layout(new log4cplus::PatternLayout(l4cp_pattern));
        ptr->setLayout(std::move(layout));
        logger.addAppender(ptr);
    }
    // 由于等级筛选被上层Logger宏做了，所以此处使用固定等级即可。
    logger.setLogLevel(log4cplus::INFO_LOG_LEVEL);

    g_logger = L4CPLoggerCallback;
    g_min_logger_level = level;
}

void ShutDownL4CPLogger() {
    g_logger = nullptr;
    logger.shutdown();
    initializer.reset();
}
