#pragma once
#include <chrono>  // NOLINT

inline int64_t GetTimeStampInSec() {
    typedef std::chrono::duration<int64_t, std::ratio<1, 1>> microsec_type;
    std::chrono::time_point<std::chrono::system_clock, microsec_type> now =
        std::chrono::time_point_cast<microsec_type>(std::chrono::system_clock::now());
    int64_t secs = now.time_since_epoch().count();
    return secs;
}

// mili second
inline int64_t GetTimeStampInMs() {
    typedef std::chrono::duration<int64_t, std::ratio<1, 1000>> microsec_type;
    std::chrono::time_point<std::chrono::system_clock, microsec_type> now =
        std::chrono::time_point_cast<microsec_type>(std::chrono::system_clock::now());
    int64_t mili_secs = now.time_since_epoch().count();
    return mili_secs;
}

// micro second
inline int64_t GetTimeStampInUs() {
    typedef std::chrono::duration<int64_t, std::ratio<1, 1000000>> microsec_type;
    std::chrono::time_point<std::chrono::system_clock, microsec_type> now =
        std::chrono::time_point_cast<microsec_type>(std::chrono::system_clock::now());
    int64_t micro_secs = now.time_since_epoch().count();
    return micro_secs;
}