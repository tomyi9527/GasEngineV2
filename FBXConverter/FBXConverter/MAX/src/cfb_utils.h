#pragma once
#include <string>
#include <iostream>
#include "compoundfilereader.h"
#include "utf.h"
#include "error_printer.h"

class Property {
public:
    struct PropertyEntry {
        uint32_t id = 0;
        std::string data;
    };
    std::vector<PropertyEntry> entries;
};

class CFBUtils {
public:
    CFBUtils(const void* buffer, size_t len)
        : reader(buffer, len) {
    }

    void ListDirectory() const {
        reader.EnumFiles(reader.GetRootEntry(), -1,
            [&](const CFB::COMPOUND_FILE_ENTRY* entry, const CFB::utf16string& dir, int level)->void {
            bool isDirectory = !reader.IsStream(entry);
            std::string name = UTF16ToUTF8(entry->name);
            std::string indent_str(level * 4 - 4, ' ');
            std::cout << indent_str.c_str() << (isDirectory ? "[" : "") << name.c_str() << (isDirectory ? "]" : "") << std::endl;
        });
    }

    std::string GetBuffer(const std::string& streamName) const {
        const CFB::COMPOUND_FILE_ENTRY* entry = FindStream(streamName.c_str());
        if (entry == nullptr) {
            PrintError(OleParserError, LEVEL_ERROR, "stream " + streamName + " not found.");
            return std::string();
        }
        // std::cout << "size: " << entry->size << std::endl;
        size_t size = static_cast<size_t>(entry->size);
        std::string stream_content(size, '\0');
        try {
            reader.ReadFile(entry, 0, stream_content.data(), stream_content.size());
        } catch (std::runtime_error& e) {
            PrintError(OleParserError, LEVEL_FATAL, "error parsing " + streamName + ": " + e.what());
            stream_content.resize(0);
        }
        return stream_content;
    }

    std::vector<Property> GetProperties(const std::string& streamName) const {
        std::vector<Property> ret;
        if (streamName.front() != '\x05') {
            PrintError(OleParserError, LEVEL_ERROR, "stream " + streamName + " is not property stream.");
            return ret;
        }
        auto buffer = GetBuffer(streamName);
        if (!buffer.empty()) {
            CFB::PropertySetStream properties(buffer.data(), buffer.size());
            for (int i = 0; i < properties.GetPropertySetCount(); ++i) {
                Property property_iter;
                auto property_item = properties.GetPropertySet(i);
                for (int j = 0; j < property_item.GetPropertyCount(); ++j) {
                    Property::PropertyEntry entry;
                    auto data = property_item.GetPropertyData(j);
                    //std::cout << "[" << i << ", " << j << "] , id: " << property_item.GetPropertyID(j) << std::endl;
                    //std::cout << BinaryToAsciiPrintable(data) << std::endl;
                    //std::cout << BinaryToHexPrintable(data) << std::endl;
                    entry.id = property_item.GetPropertyID(j);
                    entry.data = property_item.GetPropertyData(j);
                    property_iter.entries.push_back(std::move(entry));
                }
                ret.push_back(std::move(property_iter));
            }
        }
        return ret;
    }

protected:
    const CFB::COMPOUND_FILE_ENTRY* FindStream(const char* streamName) const {
        const CFB::COMPOUND_FILE_ENTRY* ret = nullptr;
        reader.EnumFiles(reader.GetRootEntry(), -1,
            [&](const CFB::COMPOUND_FILE_ENTRY* entry, const CFB::utf16string& u16dir, int level)->void {
            if (reader.IsStream(entry)) {
                std::string name = UTF16ToUTF8(entry->name);
                if (u16dir.length() > 0) {
                    std::string dir = UTF16ToUTF8(u16dir.c_str());
                    if (strncmp(streamName, dir.c_str(), dir.length()) == 0 &&
                        streamName[dir.length()] == '\\' &&
                        strcmp(streamName + dir.length() + 1, name.c_str()) == 0) {
                        ret = entry;
                    }
                } else {
                    if (strcmp(streamName, name.c_str()) == 0) {
                        ret = entry;
                    }
                }
            }
        });
        return ret;
    }

    CFB::CompoundFileReader reader;
};