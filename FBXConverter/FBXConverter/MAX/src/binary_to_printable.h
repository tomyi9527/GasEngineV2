#pragma once

#include <string>
#include <string_view>

std::string BinaryToHexPrintable(unsigned char ch);
std::string BinaryToHexPrintable(const unsigned char* data, size_t size);
std::string BinaryToHexPrintable(const std::string& str);
std::string BinaryToAsciiPrintable(const std::string& str);

std::string BinaryToHexPrintable(const std::string_view& str);
std::string BinaryToAsciiPrintable(const std::string_view& str);