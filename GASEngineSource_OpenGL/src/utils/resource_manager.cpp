#include "utils/resource_manager.h"
#include <fstream>
#include <sstream>
#include "utils/findfile.h"
#include "utils/logger.h"

bool resource::DefaultResourceLoader::Rule(const std::string& path,
                                           const std::vector<std::string>& path_hint) {
    return !FindFileByPathHint(path, path_hint).empty();
}

std::string resource::DefaultResourceLoader::CreateUri(const std::string& path,
                                                       const std::vector<std::string>& path_hint) {
    return FrontOrDefault(FindFileByPathHint(path, path_hint));
}

std::string resource::DefaultResourceLoader::LoadContentMemory(
    const std::string& universal_identifier) {
    auto s = DefaultResourceLoader::LoadContentStream(universal_identifier);

    std::string content;
    if (s && !s->fail()) {
        content.assign(std::istreambuf_iterator<char>(*s), std::istreambuf_iterator<char>());
    }

    return content;
}

std::shared_ptr<std::istream> resource::DefaultResourceLoader::LoadContentStream(
    const std::string& universal_identifier) {
    std::ifstream* ptr = new std::ifstream;
    ptr->open(universal_identifier, std::ios::in | std::ios::binary);
    if (!ptr->is_open()) {
        LOG_ERROR("%s open failed.", universal_identifier.c_str());
    }
    return std::shared_ptr<std::istream>(ptr);
}

std::shared_ptr<std::istream> resource::ResourceLoaderInterface::LoadContentStream(
    const std::string& universal_identifier) {
    return std::shared_ptr<std::istream>(
        new std::istringstream(LoadContentMemory(universal_identifier)));
}
