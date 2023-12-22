#pragma once
#include <string>
#include <vector>
#include "utils/json_maker.h"

using IDType = uint64_t;
// 也可resp
class TaskRecordItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    IDType id = 0;
    IDType parent_id = 0;
    int64_t count = 0;
    std::string name;
    std::string type;
    std::string str_1;
    std::string str_2;
    std::string str_3;
    std::string str_4;
    uint64_t ui64_1 = 0;
    uint64_t ui64_2 = 0;
    int64_t i64_1 = 0;
    int64_t i64_2 = 0;
    double fp_1 = 0;
    double fp_2 = 0;
    bool finish_status = false;
    int64_t param_index = 0;
};

// request
class CreateAssetReqItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    std::string name;
    std::string description;
    std::string file_format;  // 没有'.'
    std::string origin_url;
    IDType parent_id = 0;
    IDType fake_id = 0;
    bool add_new_version = false;
};
class CreateDirectoryReqItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    std::string name;
    std::string description;
    std::string icon_url;  //  [文件名]（链接） 如 [www.xxxx.com.png](http://www.xxxx.com.png)
    IDType parent_id = 0;
};

class CreateTsaReqItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    IDType asset_id = 0;
    std::string title;
    std::string description;
    std::string company;
    std::string tsa_info;

    bool CommonFieldEmpty() const { return company.empty() || tsa_info.empty(); }
};

class UpdateAssetByIDReqItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    IDType id = 0;
    std::string name;
    std::string description;
    std::string origin_url;
    std::string preview_url;
};

class GetSignatureReqItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    IDType object_id = 0;
    std::string object_meta;
    std::string file_name;
    std::string content_type;
    std::string content_disposition;
    std::string content_encoding;
    std::string upload_id;
    int part_number = -1;
};

// response
template <typename ResultType, typename ErrorType>
class ReplyWithCode : public json::JsonSerializeInterface {
 public:
    bool FromJson(const std::string& str) override {
        return json::JsonSerializeInterface::FromJson(str);
    }
    bool FromJson(const rapidjson::Value& v) override {
        using json::GetMember;
        bool ret = true;
        ret = GetMember(v, "code", code) && ret;
        ret = GetMember(v, "result", result) && ret;
        ret = GetMember(v, "error", error) && ret;
        return ret;
    }
    void AddToJson(json::JsonDoc& js, rapidjson::Value& v) const override {
        if (!v.IsObject()) v.SetObject();
        js.AddMemberTo(v, "code", code);
        js.AddMemberTo(v, "result", result);
        js.AddMemberTo(v, "error", error);
        return;
    }

    int code = -100;
    ResultType result;
    ErrorType error;
};

template <typename ResultItem>
class ItemsResult : public json::JsonSerializeInterface {
 public:
    bool FromJson(const std::string& str) override {
        return json::JsonSerializeInterface::FromJson(str);
    }
    bool FromJson(const rapidjson::Value& v) override {
        using json::GetMember;
        bool ret = true;
        ret = GetMember(v, "items", items) && ret;
        return ret;
    }
    void AddToJson(json::JsonDoc& js, rapidjson::Value& v) const override {
        if (!v.IsObject()) v.SetObject();
        js.AddMemberTo(v, "items", items);
        return;
    }
    std::vector<ResultItem> items;
};

namespace CreateTaskRecordReply {
using ResultItem = TaskRecordItem;
using Result = ItemsResult<ResultItem>;

class ErrorItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    int64_t param_index = -1;  // 在原本参数中的位置
    std::string message;
};
}  // namespace CreateTaskRecordReply
using CreateTaskRecordReplyJson =
    ReplyWithCode<CreateTaskRecordReply::Result, std::vector<CreateTaskRecordReply::ErrorItem>>;

namespace UpdateTaskRecordReply {
using Result = CreateTaskRecordReply::Result;
using ErrorItem = CreateTaskRecordReply::ErrorItem;
}  // namespace UpdateTaskRecordReply
using UpdateTaskRecordReplyJson =
    ReplyWithCode<UpdateTaskRecordReply::Result, std::vector<UpdateTaskRecordReply::ErrorItem>>;

namespace QueryTaskRecordCountReply {
class Error : public CreateTaskRecordReply::ErrorItem {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    std::string task_id;
};
}  // namespace QueryTaskRecordCountReply
using QueryTaskRecordCountReplyJson = ReplyWithCode<int, QueryTaskRecordCountReply::Error>;

namespace QueryTaskRecordIDInRangeReply {
class Result : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    std::vector<IDType> ids;
};
using Error = QueryTaskRecordCountReply::Error;
}  // namespace QueryTaskRecordIDInRangeReply
using QueryTaskRecordIDInRangeReplyJson =
    ReplyWithCode<QueryTaskRecordIDInRangeReply::Result, QueryTaskRecordIDInRangeReply::Error>;

namespace QueryTaskRecordDetailReply {
using Result = ItemsResult<TaskRecordItem>;
class ErrorItem : public CreateTaskRecordReply::ErrorItem {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    std::string id;
};
}  // namespace QueryTaskRecordDetailReply
using QueryTaskRecordDetailReplyJson =
    ReplyWithCode<QueryTaskRecordDetailReply::Result,
                  std::vector<QueryTaskRecordDetailReply::ErrorItem>>;

namespace GetDownloadSignatureReply {
class ResultItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;

    IDType object_id;
    std::string object_meta;
    std::string file_name;
    std::string api_type;
    std::string origin_url;
    std::string signed_url;
    int64_t expire = 0;
    int64_t param_index = -1;  // 在原本参数中的位置
};
using Result = ItemsResult<ResultItem>;
class ErrorItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;

    IDType object_id;
    std::string object_meta;
    std::string file_name;
    int64_t param_index = -1;  // 在原本参数中的位置
    std::string message;
};
}  // namespace GetDownloadSignatureReply
using GetDownloadSignatureReplyJson =
    ReplyWithCode<GetDownloadSignatureReply::Result,
                  std::vector<GetDownloadSignatureReply::ErrorItem>>;

namespace GetUploadSignatureReply {
using Result = GetDownloadSignatureReply::Result;
using ErrorItem = GetDownloadSignatureReply::ErrorItem;
}  // namespace GetUploadSignatureReply
using GetUploadSignatureReplyJson =
    ReplyWithCode<GetUploadSignatureReply::Result, std::vector<GetUploadSignatureReply::ErrorItem>>;

namespace CreateMultipartUploadSignatureReply {
using Result = GetUploadSignatureReply::Result;
using ErrorItem = GetUploadSignatureReply::ErrorItem;
}  // namespace CreateMultipartUploadSignatureReply
using CreateMultipartUploadSignatureReplyJson =
    ReplyWithCode<CreateMultipartUploadSignatureReply::Result,
                  std::vector<CreateMultipartUploadSignatureReply::ErrorItem>>;

namespace CompleteMultipartUploadSignatureReply {
class ResultItem : public GetDownloadSignatureReply::ResultItem {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;

    std::string upload_id;
};
class ErrorItem : public CreateMultipartUploadSignatureReply::ErrorItem {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;

    std::string upload_id;
};
}  // namespace CompleteMultipartUploadSignatureReply
using CompleteMultipartUploadSignatureReplyJson =
    ReplyWithCode<std::vector<CompleteMultipartUploadSignatureReply::ResultItem>,
                  std::vector<CompleteMultipartUploadSignatureReply::ErrorItem>>;

namespace UploadPartSignatureReply {
class ResultItem : public CompleteMultipartUploadSignatureReply::ResultItem {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;

    int64_t part_number = -1;
};
class ErrorItem : public CompleteMultipartUploadSignatureReply::ErrorItem {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;

    int64_t part_number = -1;
};
}  // namespace UploadPartSignatureReply
using UploadPartSignatureReplyJson =
    ReplyWithCode<std::vector<UploadPartSignatureReply::ResultItem>,
                  std::vector<UploadPartSignatureReply::ErrorItem>>;

namespace CreateAssetReply {
class ResultItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;

    int64_t param_index = -1;
    IDType id;
};

using Result = ItemsResult<ResultItem>;

class ErrorItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;

    int64_t param_index = -1;
    IDType id;
    std::string message;
};
}  // namespace CreateAssetReply
using CreateAssetReplyJson =
    ReplyWithCode<CreateAssetReply::Result, std::vector<CreateAssetReply::ErrorItem>>;

namespace CreateDirectoryReply {
using Result = CreateAssetReply::Result;
using Error = std::vector<CreateAssetReply::ErrorItem>;
}  // namespace CreateDirectoryReply
using CreateDirectoryReplyJson =
    ReplyWithCode<CreateDirectoryReply::Result, CreateDirectoryReply::Error>;

namespace UpdateAssetByIDReply {
using Result = CreateAssetReply::Result;
using Error = std::vector<CreateAssetReply::ErrorItem>;
}  // namespace UpdateAssetByIDReply
using UpdateAssetByIDReplyJson =
    ReplyWithCode<UpdateAssetByIDReply::Result, UpdateAssetByIDReply::Error>;

namespace GetChildNodeCountReply {
class ResultItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    int64_t param_index = -1;
    int64_t count = -1;
};
using Result = ItemsResult<ResultItem>;
class ErrorItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    int64_t param_index = -1;
    std::string message;
};
}  // namespace GetChildNodeCountReply
using GetChildNodeCountReplyJson =
    ReplyWithCode<GetChildNodeCountReply::Result, std::vector<GetChildNodeCountReply::ErrorItem>>;

namespace GetChildNodeIDInRangeReply {
class Result : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    std::vector<IDType> nodes;
};
class Error : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    std::string message;
};
}  // namespace GetChildNodeIDInRangeReply
using GetChildNodeIDInRangeReplyJson =
    ReplyWithCode<GetChildNodeIDInRangeReply::Result, GetChildNodeIDInRangeReply::Error>;

namespace GetNodeBriefByIDReply {
class ResultItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    IDType id;
    std::string name;
    std::string type;
    std::string file_format;
    int64_t child_count = -1;
    std::string preview_url;
    std::string status;
    std::string permission;
    std::string created_date;
    std::string updated_date;
    std::vector<std::string> full_path_name;
    std::vector<IDType> full_path_id;
    // int64_t permission_mask = -1;
    std::string icon_url;
    std::string node_type;
    std::string creator;
    int64_t capacity = -1;
    int64_t direct_child_count = -1;
    int64_t total_leaf_count = -1;
    int64_t direct_directory_count = -1;
    std::string display_mode;
    std::string name_pinyin;
    int64_t param_index = -1;
    std::string GetFilename() const {
        if (file_format.empty())
            return name;
        else
            return name + "." + file_format;
    }
};
using Result = ItemsResult<ResultItem>;
class ErrorItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    IDType id = 0;
    int64_t param_index = -1;
    std::string message;
};
}  // namespace GetNodeBriefByIDReply
using GetNodeBriefByIDReplyJson =
    ReplyWithCode<GetNodeBriefByIDReply::Result, std::vector<GetNodeBriefByIDReply::ErrorItem>>;

namespace ConvertAssetReply {
using ResultItem = GetNodeBriefByIDReply::ErrorItem;  // 其实这两个格式差不多
using Result = ItemsResult<ResultItem>;
using ErrorItem = GetNodeBriefByIDReply::ErrorItem;
}  // namespace ConvertAssetReply
using ConvertAssetReplyJson =
    ReplyWithCode<ConvertAssetReply::Result, std::vector<ConvertAssetReply::ErrorItem>>;

namespace CreateTsaReply {
class ResultItem : public json::JsonSerializeInterface {
 public:
    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
    int param_index = 0;
    IDType asset_id = 0;
    IDType tsa_id = 0;
    std::string asset_hash;
    std::string tsa_url;
    IDType tsa_item_id = 0;
    std::string tsa_title;
    std::string company;
    std::string tsa_info;
    std::string tsa_description;
    std::string created_date;
};
using Result = ItemsResult<ResultItem>;
using ErrorItem = GetChildNodeCountReply::ErrorItem;
}  // namespace CreateTsaReply
using CreateTsaReplyJson =
    ReplyWithCode<CreateTsaReply::Result, std::vector<CreateTsaReply::ErrorItem>>;
