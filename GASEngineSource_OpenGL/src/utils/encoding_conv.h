#pragma once
#include <string>
#include <string_view>

std::string GBKToUtf8(const char* src_str, size_t s);
std::string Utf8ToGBK(const char* src_str, size_t s);
std::wstring Utf8ToUCS2(const char* src_str, size_t s);
std::wstring GBKToUCS2(const char* src_str, size_t s);
std::string UCS2ToUTF8(const wchar_t* src_str, size_t s);
std::string UCS2ToGBK(const wchar_t* src_str, size_t s);
// std::string Utf8ToGB18030(const char *src_str, size_t s);

#define DECLARE_FUNCTION(tin, x, tout) \
inline tout x(tin str) { return x(str.data(), str.size()); }\

DECLARE_FUNCTION(const std::string&, GBKToUtf8, std::string);
DECLARE_FUNCTION(const std::string&, Utf8ToGBK, std::string);
DECLARE_FUNCTION(const std::string_view&, GBKToUtf8, std::string);
DECLARE_FUNCTION(const std::string_view&, Utf8ToGBK, std::string);
DECLARE_FUNCTION(const std::string&, Utf8ToUCS2, std::wstring);
DECLARE_FUNCTION(const std::string&, GBKToUCS2, std::wstring);
DECLARE_FUNCTION(const std::string_view&, Utf8ToUCS2, std::wstring);
DECLARE_FUNCTION(const std::string_view&, GBKToUCS2, std::wstring);
DECLARE_FUNCTION(const std::wstring&, UCS2ToUTF8, std::string);
DECLARE_FUNCTION(const std::wstring&, UCS2ToGBK, std::string);
DECLARE_FUNCTION(const std::wstring_view&, UCS2ToUTF8, std::string);
DECLARE_FUNCTION(const std::wstring_view&, UCS2ToGBK, std::string);

#undef DECLARE_FUNCTION