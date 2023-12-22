#include "utils/encoding_conv.h"

#include <string>
#include <cstring>
#include <locale>
#include <codecvt>

std::wstring_convert<std::codecvt_utf8<wchar_t>> Conver_UTF8;  // utf8-wchar
#ifdef WIN32
const char* GBK_LOCALE_NAME = ".936";  // GBK在windows下的locale name
#else
// GBK在linux下的locale名可能是"zh_CN.GBK"
const char* GBK_LOCALE_NAME = "zh_CN.GBK";  
#endif
class codecvt_gbk : public std::codecvt_byname<wchar_t, char, mbstate_t> {
 public:
    codecvt_gbk(const char* name) : std::codecvt_byname<wchar_t, char, mbstate_t>(name) {}
    ~codecvt_gbk() {}
};
std::wstring_convert<codecvt_gbk> Conver_GBK(new codecvt_gbk(GBK_LOCALE_NAME));  // GBK - whar

std::string GBKToUtf8(const char* src_str, size_t s) {
    std::wstring wDst = Conver_GBK.from_bytes(src_str, src_str + s);
    return Conver_UTF8.to_bytes(wDst);
}
std::string Utf8ToGBK(const char* src_str, size_t s) {
    std::wstring wDst = Conver_UTF8.from_bytes(src_str, src_str + s);
    return Conver_GBK.to_bytes(wDst);
}

std::wstring GBKToUCS2(const char* src_str, size_t s) {
    return Conver_GBK.from_bytes(src_str, src_str + s);
}
std::wstring Utf8ToUCS2(const char* src_str, size_t s) {
    return Conver_UTF8.from_bytes(src_str, src_str + s);
}

std::string UCS2ToUTF8(const wchar_t* src_str, size_t s) { 
    return Conver_UTF8.to_bytes(std::wstring(src_str, s));
}
std::string UCS2ToGBK(const wchar_t* src_str, size_t s) {
    return Conver_GBK.to_bytes(std::wstring(src_str, s));
}