#pragma once
#include <string>
#include <vector>
#include "arthub_client.h"
#include "url.h"
#include "utils/logger.h"
#include "utils/resource_manager.h"
#include "utils/string_util.h"

// 用于表示一个arthub远程资源
// from:
//   @@depot_id/asset_id/asset_file_meta/file_name
//   @@depot_id/asset_id/asset_file_meta
// to:
//   http://arthub.rgw.ied.com/depot_id/xxxxxxxxxxxxxxxxxxxx
class ArthubAtAtResourceIdentifier {
 public:
    void FromString(const std::string& in);
    std::string ToString() const;
    inline bool IsValid() const { return !depot.empty() && asset_id > 0 && !asset_meta.empty(); }

 public:
    std::string depot;
    uint64_t asset_id = 0;
    std::string asset_meta;
    std::string file_name;
};

class ArthubAtAtResourceStrategy : public resource::ResourceLoaderInterface {
 public:
    std::string GetName() const override { return "Arthub @@ resource pointer"; }
    bool Rule(const std::string& path, const std::vector<std::string>& path_hint) override;
    std::string CreateUri(const std::string& path,
                          const std::vector<std::string>& path_hint) override;
    std::string LoadContentMemory(const std::string& universal_identifier) override;
};

class OnlineHttpResourceStrategy : public resource::ResourceLoaderInterface {
 public:
    std::string GetName() const override { return "http uri resource"; }
    bool Rule(const std::string& path, const std::vector<std::string>& path_hint) override;
    std::string CreateUri(const std::string& path,
                          const std::vector<std::string>& path_hint) override;
    std::string LoadContentMemory(const std::string& universal_identifier) override;
};

inline void AddOnlineResourceLoader() {
    resource::ResourceManager::Instance().AddStrategy(
        std::make_unique<ArthubAtAtResourceStrategy>());
    resource::ResourceManager::Instance().AddStrategy(
        std::make_unique<OnlineHttpResourceStrategy>());
}