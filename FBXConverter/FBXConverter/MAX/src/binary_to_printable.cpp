#include "binary_to_printable.h"
#include <string>

const char k_hexToChar[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

std::string BinaryToHexPrintable(unsigned char ch) {
    std::string hex_string(2, ' ');
    hex_string[1] = k_hexToChar[ch & 0x0F];
    hex_string[0] = k_hexToChar[ch >> 4];
    return hex_string;
}

std::string BinaryToHexPrintable(const unsigned char* data, size_t size) {
    std::string hex_string(size * 2, ' ');
    for (size_t i = 0; i < size; ++i) {
        hex_string[2 * i + 0] = k_hexToChar[data[i] >> 4];
        hex_string[2 * i + 1] = k_hexToChar[data[i] & 0x0F];
    }
    return hex_string;
}

std::string BinaryToHexPrintable(const std::string& str) {
    std::string hex_string(str.size() * 2, ' ');
    for (size_t i = 0; i < str.size(); ++i) {
        hex_string[2 * i + 0] = k_hexToChar[(unsigned char)(str[i]) >> 4];
        hex_string[2 * i + 1] = k_hexToChar[(unsigned char)(str[i]) & 0x0F];
    }
    return hex_string;
}

std::string BinaryToAsciiPrintable(const std::string& str) {
    std::string ret = str;
    for (auto& m : ret) {
        if (m < 0 || !isprint(m)) {
            m = '.';
        }
    }
    return ret;
}

std::string BinaryToHexPrintable(const std::string_view & str) {
    std::string cpy(str.data(), str.size());
    return BinaryToHexPrintable(cpy);
}

std::string BinaryToAsciiPrintable(const std::string_view & str) {
    std::string cpy(str.data(), str.size());
    return BinaryToAsciiPrintable(cpy);
}
