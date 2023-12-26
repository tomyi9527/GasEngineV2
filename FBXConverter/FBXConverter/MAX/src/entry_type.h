#pragma once
#include <string>
#include <vector>
#include <list>
#include <inttypes.h>

namespace max {

class DllEntry {
public:
    bool valid = false;
    std::string name;
    std::string description;
};

class ClassEntry {
public:
    bool valid = false;
    std::string name;
    int32_t dll_idx = -1;
    std::string dll_name;
    std::string dll_description;
    uint32_t data[3] = { 0, 0, 0 };
};

template<typename _3DSMaxType>
inline bool IsType_ClassID(const ClassEntry& entry) {
    constexpr int len_loaded = sizeof(entry.data) / sizeof(entry.data[0]);
    static_assert(len_loaded == 3, "classinfo.data should have length [3]");
    int len_class = sizeof(_3DSMaxType::class_id) / sizeof(_3DSMaxType::class_id[0]);
    if (len_loaded != len_class)
        return false;
    for (int i = 0; i < len_loaded; ++i) {
        if (entry.data[i] != _3DSMaxType::class_id[i]) {
            return false;
        }
    }
    return true;
}

template<typename _3DSMaxType>
inline bool IsType_DllName(const ClassEntry& entry) {
    if (entry.dll_idx < 0) {
        return sizeof(_3DSMaxType::class_dll) == 1;
    }
    if (entry.dll_name.size() != sizeof(_3DSMaxType::class_dll) - 1)
        return false;
    for (int i = 0; i < entry.dll_name.size(); ++i) {
        if (entry.dll_name[i] != _3DSMaxType::class_dll[i]) {
            return false;
        }
    }
    return true;
}

template<typename _3DSMaxType>
inline bool IsType_EnglishClassName(const ClassEntry& entry) {
    if (entry.name.size() == 0) {
        return false;
    }
    if (entry.name.size() != sizeof(_3DSMaxType::english_class_name) - 1)
        return false;
    for (int i = 0; i < entry.name.size(); ++i) {
        if (entry.name[i] != _3DSMaxType::english_class_name[i]) {
            return false;
        }
    }
    return true;
}

template<typename _3DSMaxType>
inline bool IsType(const ClassEntry& entry) {
    return IsType_EnglishClassName<_3DSMaxType>(entry) || (IsType_ClassID<_3DSMaxType>(entry) && IsType_DllName<_3DSMaxType>(entry));
}

class FileAssetEntry {
public:
    std::string id;
    std::string type;
    std::string path1;
    std::string path2;
};

class Linker {
public:
    static std::vector<DllEntry>& DllInfo() {
        static std::vector<DllEntry> instance;
        return instance;
    }
    static std::vector<ClassEntry>& ClassInfo() {
        static std::vector<ClassEntry> instance;
        return instance;
    }
    static const ClassEntry& GetClassEntryByIndex(int32_t class_idx) {
        static ClassEntry empty;
        if (Linker::ClassInfo().size() > class_idx && class_idx >= 0 && Linker::ClassInfo().at(class_idx).valid) {
            const auto& target = Linker::ClassInfo().at(class_idx);
            return target;
        } else {
            return empty;
        }
    }
    static std::vector<FileAssetEntry>& FileAssetInfo() {
        static std::vector<FileAssetEntry> instance;
        return instance;
    }
    static const FileAssetEntry& GetFileAssetEntryByID(const std::string& id) {
        static FileAssetEntry empty;
        for (const auto& m : FileAssetInfo()) {
            if (m.id == id) {
                return m;
            }
        }
        return empty;
    }
};
} // max