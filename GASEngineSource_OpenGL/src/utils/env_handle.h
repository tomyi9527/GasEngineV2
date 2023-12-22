#pragma once
#include <stdlib.h>
#include <string>

#ifdef WIN32
#define _put_env(p, s) _putenv_s(p, s)
#define _get_env(p) getenv(p)
#else
#define _put_env(p, s)        \
    do {                      \
        if (p) {              \
            std::string m(p); \
            putenv(m.data()); \
        }                     \
    } while (0)
#define _get_env(p) getenv(p)
#endif

inline void set_env(const std::string& key, const std::string& value) {
    _put_env(key.c_str(), value.c_str());
}

inline std::string get_env(const std::string& key) {
    char* ptr = _get_env(key.c_str());
    if (ptr == nullptr)
        return std::string();
    else
        return std::string(ptr);
}