#include "max_reader.h"
#include "max_parser.h"
#include "encoding_conv.h"

using namespace max;

std::unique_ptr<max::MaxCracker> LoadFromMax(const std::string& full_file_path) {
#ifdef _MSC_VER
    std::wstring unicodePath = Utf8ToUCS2(full_file_path);
    std::fstream file_stream(unicodePath, std::ios::binary | std::ios::in);
#else
    std::fstream file_stream(full_file_path, std::ios::binary | std::ios::in);
#endif
    if (!file_stream) {
        PrintError(ApplicationError, LEVEL_FATAL, "file open failed: " + full_file_path);
        return nullptr;
    }

    std::string content;

    file_stream.seekg(0, std::ios::end);
    size_t file_size = file_stream.tellg();
    file_stream.seekg(0, std::ios::beg);

    content.reserve(file_size);
    content.assign(std::istreambuf_iterator<char>(file_stream), std::istreambuf_iterator<char>());

    if (content.empty()) {
        PrintError(ApplicationError, LEVEL_FATAL, "file is empty: " + full_file_path);
        return nullptr;
    }
    PrintInfo("file loaded, size is: " + std::to_string(file_size));

    try {
        auto ret = std::make_unique<max::MaxCracker>(content.data(), content.size());
        // reader.ListDirectory();
        // reader.Checker_ParseAllUsedStreams();
        ret->run();
        return ret;
    } catch (std::exception& e) {
        PrintError(ApplicationError, LEVEL_FATAL, e.what());
        return nullptr;
    }
}
