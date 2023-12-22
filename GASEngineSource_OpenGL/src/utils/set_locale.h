#pragma once

#include <clocale>
#include <locale>

inline void SetLocale(const std::string& locale_id) {
    // 并进行c/c++内的对应配置。
    // c部分:
    std::setlocale(LC_ALL, locale_id.c_str());
    // 配置数字处理方式，防止出现千分位逗号。
    // 比如libradosstriper就没考虑这点，如果设置为中国地区数字转字符串时出现问题。
    std::string local_sys = std::locale("").name();
    std::setlocale(LC_NUMERIC, local_sys.c_str());

    // C++部分:
    // std::filesystem::path也受此影响
    auto new_locale = std::locale(locale_id).combine<std::numpunct<char>>(std::locale::classic());
    std::locale::global(new_locale);  // for C++
    // 如果需要修改 cout/cerr 等:
    //std::cout.imbue(new_locale);
    //std::cerr.imbue(new_locale);
    // clog, wcout, wcerr, wclog
}