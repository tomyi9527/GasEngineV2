#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

template <typename Str>
std::string& RemoveSpecialChar(Str&& in, const std::string& chars_to_remove) {
    std::string ret;
    for (auto& m : in) {
        if (chars_to_remove.find_first_of(m) == std::string::npos) {
            ret.push_back(m);
        }
    }
    in.swap(ret);
    return in;
}

inline std::string& ToUpper(std::string& in) {
    for (auto& c : in) {
        c = std::toupper(c);
    }
    return in;
}

inline std::string& ToLower(std::string& in) {
    for (auto& c : in) {
        c = std::tolower(c);
    }
    return in;
}

inline bool StartsWith(const std::string& in, const std::string& pre_fix) {
    if (in.size() < pre_fix.size()) {
        return false;
    }
    for (size_t i = 0; i < pre_fix.size(); ++i) {
        if (in[i] != pre_fix[i]) {
            return false;
        }
    }
    return true;
}

inline bool EndsWith(const std::string& in, const std::string& post_fix) {
    auto _pos = in.find(post_fix);
    if (_pos != std::string::npos && _pos == in.size() - post_fix.size()) {
        return true;
    } else {
        return false;
    }
}

// split by charactor
inline std::vector<std::string> StringSplit(const std::string& in, const char separator = '|') {
    std::vector<std::string> ret;
    if (in.empty()) return ret;
    size_t sep_pos = 0, recent_after_sep_pos = 0;
    if (in[0] == separator) {
        ret.emplace_back();
        recent_after_sep_pos = 1;
    }
    while (recent_after_sep_pos < in.size()) {
        sep_pos = in.find_first_of(separator, sep_pos + 1);
        if (sep_pos == std::string::npos) sep_pos = in.size();
        ret.emplace_back(in.substr(recent_after_sep_pos, sep_pos - recent_after_sep_pos));
        recent_after_sep_pos = sep_pos + 1;
    }
    return ret;
}

// reverse function for StringSplit
inline std::string StringVectorJoin(const std::vector<std::string>& in,
                                    const char delimiter = '|') {
    std::string ret;
    for (std::vector<std::string>::const_iterator itr = in.begin(); itr != in.end(); itr++) {
        ret.append(*itr);
        if (itr != (in.end() - 1)) {
            ret.append(1, delimiter);
        }
    }
    return ret;
}