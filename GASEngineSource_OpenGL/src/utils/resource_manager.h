#pragma once
#include <istream>
#include <memory>
#include <string>
#include <vector>

namespace resource {

class ResourceLoaderInterface {
 public:
    virtual std::string GetName() const = 0;
    virtual std::string CreateUri(const std::string& path,
                                  const std::vector<std::string>& path_hint) = 0;
    virtual bool Rule(const std::string& path, const std::vector<std::string>& path_hint) = 0;
    virtual std::string LoadContentMemory(const std::string& universal_identifier) = 0;
    virtual std::shared_ptr<std::istream> LoadContentStream(
        const std::string& universal_identifier);
    virtual ~ResourceLoaderInterface() {}
};

class DefaultResourceLoader : public ResourceLoaderInterface {
 public:
    std::string GetName() const override { return "defaut_local_loader"; }
    bool Rule(const std::string& path, const std::vector<std::string>& path_hint) override;
    std::string CreateUri(const std::string& path,
                          const std::vector<std::string>& path_hint) override;
    std::string LoadContentMemory(const std::string& universal_identifier) override;
    std::shared_ptr<std::istream> LoadContentStream(
        const std::string& universal_identifier) override;
};

class ResourceManager {
 public:
    static ResourceManager& Instance() {
        static ResourceManager instance;
        return instance;
    }
    void AddStrategy(std::unique_ptr<ResourceLoaderInterface>&& in) {
        if (in != nullptr) {
            for (const auto& m : resource_loader_strategy) {
                if (m->GetName() == in->GetName()) {
                    return;
                }
            }
            resource_loader_strategy.push_back(std::move(in));
        }
    }
    std::string Load(const std::string& path,
                     const std::vector<std::string>& path_hint = {}) const {
        std::string universal_identifier;
        for (const auto& m : resource_loader_strategy) {
            if (m->Rule(path, path_hint)) {
                universal_identifier = m->CreateUri(path, path_hint);
                return m->LoadContentMemory(universal_identifier);
            }
        }
        universal_identifier = default_loader_strategy->CreateUri(path, path_hint);
        return default_loader_strategy->LoadContentMemory(universal_identifier);
    }
    std::shared_ptr<std::istream> LoadStream(const std::string& path,
                                             const std::vector<std::string>& path_hint = {}) const {
        std::string universal_identifier;
        for (const auto& m : resource_loader_strategy) {
            if (m->Rule(path, path_hint)) {
                universal_identifier = m->CreateUri(path, path_hint);
                return m->LoadContentStream(universal_identifier);
            }
        }
        universal_identifier = default_loader_strategy->CreateUri(path, path_hint);
        return default_loader_strategy->LoadContentStream(universal_identifier);
    }
    std::string GenerateUri(const std::string& path,
                            const std::vector<std::string>& path_hint = {}) {
        for (const auto& m : resource_loader_strategy) {
            if (m->Rule(path, path_hint)) {
                return m->CreateUri(path, path_hint);
            }
        }
        return default_loader_strategy->CreateUri(path, path_hint);
    }

 protected:
    ResourceManager() { default_loader_strategy.reset(new DefaultResourceLoader); }

 protected:
    std::unique_ptr<ResourceLoaderInterface> default_loader_strategy;
    std::vector<std::unique_ptr<ResourceLoaderInterface>> resource_loader_strategy;
};

}  // namespace resource