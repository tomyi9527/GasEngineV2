#include "online_resource_rule.h"
#include <filesystem>
#include <string>
#include <vector>
#include "httplib.h"

void ArthubAtAtResourceIdentifier::FromString(const std::string& in) {
    if (!StartsWith(in, "@@")) {
        return;
    }
    auto splited = StringSplit(in, '/');
    if (splited.size() < 4) {
        splited.resize(4);
    }
    depot = splited[0].substr(2);
    try {
        asset_id = std::stoull(splited[1]);
    } catch (...) {
        asset_id = 0;
    }
    asset_meta = splited[2];
    file_name = splited[3];
}

std::string ArthubAtAtResourceIdentifier::ToString() const {
    if (file_name.empty())
        return "@@" + StringVectorJoin({depot, std::to_string(asset_id), asset_meta}, '/');
    else
        return "@@" +
               StringVectorJoin({depot, std::to_string(asset_id), asset_meta, file_name}, '/');
}

bool ArthubAtAtResourceStrategy::Rule(const std::string& path,
                                      const std::vector<std::string>& path_hint) {
    return StartsWith(path, "@@") || (path_hint.size() > 0 && StartsWith(path_hint[0], "@@"));
}

std::string ArthubAtAtResourceStrategy::CreateUri(const std::string& path,
                                                  const std::vector<std::string>& path_hint) {
    ArthubAtAtResourceIdentifier id;
    id.FromString(path);
    if (!id.IsValid() && path_hint.size() > 0) {
        id.FromString(path_hint[0]);
        id.file_name = path;
    }
    return id.ToString();
}

std::string ArthubAtAtResourceStrategy::LoadContentMemory(const std::string& universal_identifier) {
    ArthubAtAtResourceIdentifier id;
    id.FromString(universal_identifier);
    if (!id.IsValid()) {
        LOG_ERROR("%s is not a valid arthub resource identifier", universal_identifier.c_str());
        return std::string();
    }

    ArthubClient client(ArthubConfig::GetGlobalConfig());
    GetSignatureReqItem req;
    req.object_id = id.asset_id;
    req.object_meta = id.asset_meta;
    req.file_name = id.file_name;
    return client.DownloadContent(id.depot, req);
}

bool OnlineHttpResourceStrategy::Rule(const std::string& path,
                                      const std::vector<std::string>& path_hint) {
    return StartsWith(path, "http://") || StartsWith(path, "https://") ||
           (path_hint.size() > 0 && StartsWith(path_hint[0], "http://")) ||
           (path_hint.size() > 0 && StartsWith(path_hint[0], "https://"));
}

std::string OnlineHttpResourceStrategy::CreateUri(const std::string& path,
                                                  const std::vector<std::string>& path_hint) {
    if (StartsWith(path, "http://") || StartsWith(path, "https://")) {
        return path;
    } else if ((path_hint.size() > 0 && StartsWith(path_hint[0], "http://")) ||
               (path_hint.size() > 0 && StartsWith(path_hint[0], "https://"))) {
        return std::filesystem::path(path_hint[0]).parent_path().string() + "/" + path;
    } else {
        return path;
    }
}

std::string OnlineHttpResourceStrategy::LoadContentMemory(const std::string& universal_identifier) {
    Url url(universal_identifier);
    std::string domain = url.protocol_ + "://" + url.host_;
    httplib::Client download(domain.c_str());
    auto res = download.Get(url.path_with_param_.c_str());
    if (res && res->status == 200)
        return res->body;
    else
        return std::string();
}