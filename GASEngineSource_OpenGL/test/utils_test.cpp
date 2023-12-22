#include "utils/logger.h"
#include "utils/set_locale.h"
#include "utils/time_stamp.h"

int main() {
    SetLocale("zh_CN.UTF-8");
    LOG_ERROR("test for log and timestamp, current timestamp is: %ld ms", GetTimeStampInMs());
    return 0;
}