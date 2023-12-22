#pragma once

#include <map>
#include <optional>
#include <string>

class Url {
 public:
    Url() { parsed_ = false; }
    explicit Url(const std::string& input) { ParseUrl(input); }

    bool operator==(const Url& rhs) const {
        return url_ == rhs.url_ && parameters_ == rhs.parameters_ && fragment_ == rhs.fragment_;
    }

    std::string GetParameter(const std::string& parameter_field) const {
        auto it = parameters_.find(parameter_field);
        if (it != parameters_.end()) {
            return it->second;
        } else {
            return std::string();
        }
    }

    void ParseUrl(const std::string& input) {
        url_ = input;

        size_t url_protocol_end_pos = url_.find("://");
        if (url_protocol_end_pos != std::string::npos) {
            protocol_ = url_.substr(0, url_protocol_end_pos);
            url_protocol_end_pos += 3;
        } else {
            url_protocol_end_pos = 0;
        }

        size_t host_end_pos = url_.find_first_of('/', url_protocol_end_pos);
        if (host_end_pos == std::string::npos) {
            host_ = url_.substr(url_protocol_end_pos);
        } else {
            host_ = url_.substr(url_protocol_end_pos, host_end_pos - url_protocol_end_pos);
        }

        size_t url_end_pos = url_.find_first_of('?', 0);
        if (host_end_pos != std::string::npos) {
            path_with_param_ = url_.substr(host_end_pos);
            if (url_end_pos == std::string::npos) {
                path_ = url_.substr(host_end_pos);
            } else {
                path_ = url_.substr(host_end_pos, url_end_pos - host_end_pos);
            }
        }

        size_t end_of_param = url_.find_last_of('#');
        if (end_of_param == std::string::npos)
            end_of_param = url_.size();
        else {
            fragment_ = Decode(url_.substr(end_of_param + 1));
            url_.erase(end_of_param);
        }

        if (url_end_pos != std::string::npos &&
            url_end_pos <= url_.size())  // param_list is not empty
        {
            size_t param_pos = url_end_pos + 1, next_param_pos;
            while (param_pos < end_of_param) {
                next_param_pos = url_.find_first_of('&', param_pos);
                if (next_param_pos == std::string::npos) next_param_pos = end_of_param;

                size_t equ_pos = url_.find_first_of('=', param_pos);
                // param_pos -> equ_pos 为第一个
                // equ_pos+1 -> next_param_pos 为第二个
                if (equ_pos == std::string::npos || equ_pos > next_param_pos) {
                    equ_pos = next_param_pos;
                }

                std::string key(url_.data() + param_pos, url_.data() + equ_pos);
                std::string key_decoded = Decode(key);

                if (equ_pos == next_param_pos) --equ_pos;

                std::string value(url_.data() + equ_pos + 1, next_param_pos - (equ_pos + 1));
                std::string value_decoded = Decode(value);

                parameters_[key_decoded] = value_decoded;

                param_pos = next_param_pos + 1;
            }
            url_.erase(url_end_pos);
        }

        url_ = Decode(url_);
    }

 public:
    // url and 3 components of url
    std::string url_;
    std::string protocol_;
    std::string host_;
    std::string path_;
    std::string path_with_param_;
    // parameters
    std::map<std::string, std::string> parameters_;
    // fragment
    std::string fragment_;

    bool parsed_;

 protected:
    static inline unsigned char ToHex(unsigned char x) {
        // 0  - 9   转 char的 '0' - '9'
        // 10 - 15  转 char的 'A' - 'F'
        return x > 9 ? x + ('A' - 10) : x + '0';
    }

    static inline std::optional<unsigned char> FromHex(unsigned char x) {
        unsigned char y;
        if (x >= 'A' && x <= 'F')
            y = x - 'A' + 10;
        else if (x >= 'a' && x <= 'f')
            y = x - 'a' + 10;
        else if (x >= '0' && x <= '9')
            y = x - '0';
        else
            return std::nullopt;  // not a hex number
        return y;
    }

 public:
    static std::string Encode(const std::string& str) { return Encode(str, "-_.~"); }

    static std::string Encode(const std::string& str, const std::string& char_excluded) {
        std::string strTemp;
        strTemp.reserve(str.size() * 2);
        for (auto const& m : str) {
            // 不替换的字符
            if (isalnum((unsigned char)m) || char_excluded.find_first_of(m) != std::string::npos)
                strTemp.push_back(m);
            // 2020.01.16 此处因为要求空格不转换为加号所以去除了
            //// 替换的
            // else if (m == ' ')
            //    strTemp += "+";
            // 转换的
            else {
                strTemp += '%';
                strTemp += ToHex((unsigned char)m >> 4);
                strTemp += ToHex((unsigned char)m % 16);
            }
        }
        return strTemp;
    }

    // parse 失败返回原始内容
    static std::string Decode(const std::string& str) {
        std::string strTemp;
        strTemp.reserve(str.size());
        size_t length = str.length();
        bool normal_end = true;
        for (size_t i = 0; i < length; i++) {
            // 2020.01.16 空格有两种encode方式。
            // 如果原本是加号，encode后应变成 % 形式的
            // 所以此处的加号是由空格encode来的。
            if (str[i] == '+')
                strTemp += ' ';
            else if (str[i] == '%' && i + 2 < length) {
                auto high = FromHex((unsigned char)str[++i]);
                auto low = FromHex((unsigned char)str[++i]);
                if (!high.has_value() || !low.has_value()) {
                    normal_end = false;
                    break;
                }
                strTemp += (high.value() << 4) + low.value();
            } else
                strTemp += str[i];
        }
        if (normal_end)
            return strTemp;
        else
            return str;
    }

    static std::string RemoveUrlProtocol(const std::string& input) {
        auto pos = input.find("://");
        if (pos != std::string::npos) {
            std::string protocol = input.substr(0, pos);
            return input.substr(pos + 3);
        } else {
            return input;
        }
    }

    static std::string ExtractHostFromUrl(const std::string& url) {
        std::string url_without_protocol = RemoveUrlProtocol(url);
        size_t pos = url_without_protocol.find_first_of('/');
        if (pos == std::string::npos) {
            return url_without_protocol;
        } else {
            return url_without_protocol.substr(0, pos);
        }
    }

    static std::string ExtractPathFromUrl(const std::string& url) {
        std::string url_without_protocol = RemoveUrlProtocol(url);
        size_t pos = url_without_protocol.find_first_of('/');
        if (pos == std::string::npos) {
            return std::string();
        } else {
            return url_without_protocol.substr(pos + 1);
        }
    }
};
