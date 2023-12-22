#include "arthub_client_format.h"

bool TaskRecordItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "id", id) && ret;
    ret = GetMember(v, "parent_id", parent_id) && ret;
    ret = GetMember(v, "name", name) && ret;
    ret = GetMember(v, "str_1", str_1) && ret;
    ret = GetMember(v, "str_2", str_2) && ret;
    ret = GetMember(v, "str_3", str_3) && ret;
    ret = GetMember(v, "str_4", str_4) && ret;
    ret = GetMember(v, "ui64_1", ui64_1) && ret;
    ret = GetMember(v, "ui64_2", ui64_2) && ret;
    ret = GetMember(v, "i64_1", i64_1) && ret;
    ret = GetMember(v, "i64_2", i64_2) && ret;
    ret = GetMember(v, "fp_1", fp_1) && ret;
    ret = GetMember(v, "fp_2", fp_2) && ret;
    ret = GetMember(v, "count", count) && ret;
    ret = GetMember(v, "type", type) && ret;
    ret = GetMember(v, "finish_status", finish_status) && ret;
    GetMember(v, "param_index", param_index);
    return ret;
}
void TaskRecordItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    if (id != 0) js.AddMemberTo(v, "id", id);
    if (parent_id != 0) js.AddMemberTo(v, "parent_id", parent_id);
    if (!name.empty()) js.AddMemberTo(v, "name", name);
    if (!type.empty()) js.AddMemberTo(v, "type", type);
    if (!str_1.empty()) js.AddMemberTo(v, "str_1", str_1);
    if (!str_2.empty()) js.AddMemberTo(v, "str_2", str_2);
    if (!str_3.empty()) js.AddMemberTo(v, "str_3", str_3);
    if (!str_4.empty()) js.AddMemberTo(v, "str_4", str_4);
    if (ui64_1 != 0) js.AddMemberTo(v, "ui64_1", ui64_1);
    if (ui64_2 != 0) js.AddMemberTo(v, "ui64_2", ui64_2);
    if (i64_1 != 0) js.AddMemberTo(v, "i64_1", i64_1);
    if (i64_2 != 0) js.AddMemberTo(v, "i64_2", i64_2);
    if (fp_1 != 0.0) js.AddMemberTo(v, "fp_1", fp_1);
    if (fp_2 != 0.0) js.AddMemberTo(v, "fp_2", fp_2);
    if (count != 0) js.AddMemberTo(v, "count", count);
    js.AddMemberTo(v, "finish_status", finish_status);
    return;
}

bool CreateAssetReqItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "name", name) && ret;
    ret = GetMember(v, "description", description) && ret;
    ret = GetMember(v, "file_format", file_format) && ret;
    ret = GetMember(v, "origin_url", origin_url) && ret;
    ret = GetMember(v, "parent_id", parent_id) && ret;
    ret = GetMember(v, "fake_id", fake_id) && ret;
    ret = GetMember(v, "add_new_version", add_new_version) && ret;
    return ret;
}
void CreateAssetReqItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "name", name);
    js.AddMemberTo(v, "description", description);
    if (!file_format.empty()) js.AddMemberTo(v, "file_format", file_format);
    if (!origin_url.empty()) js.AddMemberTo(v, "origin_url", origin_url);
    js.AddMemberTo(v, "parent_id", parent_id);
    js.AddMemberTo(v, "add_new_version", add_new_version);
    if (fake_id != 0) js.AddMemberTo(v, "fake_id", fake_id);
    return;
}

bool CreateDirectoryReqItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "name", name) && ret;
    ret = GetMember(v, "description", description) && ret;
    ret = GetMember(v, "icon_url", icon_url) && ret;
    ret = GetMember(v, "parent_id", parent_id) && ret;
    return ret;
}
void CreateDirectoryReqItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "name", name);
    js.AddMemberTo(v, "description", description);
    js.AddMemberTo(v, "icon_url", icon_url);
    js.AddMemberTo(v, "parent_id", parent_id);
    return;
}

bool CreateTsaReqItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "asset_id", asset_id) && ret;
    ret = GetMember(v, "title", title) && ret;
    ret = GetMember(v, "description", description) && ret;
    ret = GetMember(v, "company", company) && ret;
    ret = GetMember(v, "tsa_info", tsa_info) && ret;
    return ret;
}
void CreateTsaReqItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "asset_id", asset_id);
    js.AddMemberTo(v, "title", title);
    js.AddMemberTo(v, "description", description);
    js.AddMemberTo(v, "company", company);
    js.AddMemberTo(v, "tsa_info", tsa_info);
    return;
}

bool UpdateAssetByIDReqItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "id", id) && ret;
    ret = GetMember(v, "name", name) && ret;
    ret = GetMember(v, "description", description) && ret;
    ret = GetMember(v, "origin_url", origin_url) && ret;
    ret = GetMember(v, "preview_url", preview_url) && ret;
    return ret;
}
void UpdateAssetByIDReqItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "id", id);
    if (!name.empty()) js.AddMemberTo(v, "name", name);
    if (!description.empty()) js.AddMemberTo(v, "description", description);
    if (!origin_url.empty()) js.AddMemberTo(v, "origin_url", origin_url);
    if (!preview_url.empty()) js.AddMemberTo(v, "preview_url", preview_url);
    return;
}

bool GetSignatureReqItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "object_id", object_id) && ret;
    ret = GetMember(v, "object_meta", object_meta) && ret;
    ret = GetMember(v, "file_name", file_name) && ret;
    ret = GetMember(v, "content_type", content_type) && ret;
    ret = GetMember(v, "content_disposition", content_disposition) && ret;
    ret = GetMember(v, "content_encoding", content_encoding) && ret;
    ret = GetMember(v, "upload_id", upload_id) && ret;
    ret = GetMember(v, "part_number", part_number) && ret;
    return ret;
}
void GetSignatureReqItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "object_id", object_id);
    js.AddMemberTo(v, "object_meta", object_meta);
    if (!file_name.empty()) js.AddMemberTo(v, "file_name", file_name);
    if (!content_type.empty()) js.AddMemberTo(v, "content_type", content_type);
    if (!content_disposition.empty()) js.AddMemberTo(v, "content_disposition", content_disposition);
    if (!content_encoding.empty()) js.AddMemberTo(v, "content_encoding", content_encoding);
    if (!upload_id.empty()) js.AddMemberTo(v, "upload_id", upload_id);
    if (part_number >= 0) js.AddMemberTo(v, "part_number", part_number);
    return;
}

namespace CreateTaskRecordReply {
bool ErrorItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "param_index", param_index) && ret;
    ret = GetMember(v, "message", message) && ret;
    return ret;
}
void ErrorItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "param_index", param_index);
    js.AddMemberTo(v, "message", message);
    return;
}
}  // namespace CreateTaskRecordReply

namespace GetDownloadSignatureReply {
bool ResultItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "object_id", object_id) && ret;
    ret = GetMember(v, "object_meta", object_meta) && ret;
    ret = GetMember(v, "file_name", file_name) && ret;
    ret = GetMember(v, "api_type", api_type) && ret;
    ret = GetMember(v, "origin_url", origin_url) && ret;
    ret = GetMember(v, "signed_url", signed_url) && ret;
    ret = GetMember(v, "expire", expire) && ret;
    ret = GetMember(v, "param_index", param_index) && ret;
    return ret;
}
void ResultItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "object_id", object_id);
    js.AddMemberTo(v, "object_meta", object_meta);
    js.AddMemberTo(v, "file_name", file_name);
    js.AddMemberTo(v, "api_type", api_type);
    js.AddMemberTo(v, "origin_url", origin_url);
    js.AddMemberTo(v, "signed_url", signed_url);
    js.AddMemberTo(v, "expire", expire);
    js.AddMemberTo(v, "param_index", param_index);
    return;
}
bool ErrorItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "object_id", object_id) && ret;
    ret = GetMember(v, "object_meta", object_meta) && ret;
    ret = GetMember(v, "file_name", file_name) && ret;
    ret = GetMember(v, "message", message) && ret;
    ret = GetMember(v, "param_index", param_index) && ret;
    return ret;
}
void ErrorItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "object_id", object_id);
    js.AddMemberTo(v, "object_meta", object_meta);
    js.AddMemberTo(v, "file_name", file_name);
    js.AddMemberTo(v, "message", message);
    js.AddMemberTo(v, "param_index", param_index);
    return;
}
}  // namespace GetDownloadSignatureReply

namespace UpdateTaskRecordReply {}

namespace QueryTaskRecordCountReply {
bool Error::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = CreateTaskRecordReply::ErrorItem::FromJson(v);
    ret = GetMember(v, "task_id", task_id) && ret;
    return ret;
}
void Error::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "task_id", task_id);
    CreateTaskRecordReply::ErrorItem::AddToJson(js, v);
    return;
}
}  // namespace QueryTaskRecordCountReply

namespace QueryTaskRecordIDInRangeReply {
bool Result::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "ids", ids) && ret;
    return ret;
}
void Result::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "ids", ids);
    return;
}
}  // namespace QueryTaskRecordIDInRangeReply

namespace QueryTaskRecordDetailReply {
bool ErrorItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = CreateTaskRecordReply::ErrorItem::FromJson(v);
    ret = GetMember(v, "id", id) && ret;
    return ret;
}
void ErrorItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "id", id);
    CreateTaskRecordReply::ErrorItem::AddToJson(js, v);
    return;
}
}  // namespace QueryTaskRecordDetailReply

namespace CompleteMultipartUploadSignatureReply {
bool ResultItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = GetDownloadSignatureReply::ResultItem::FromJson(v);
    ret = GetMember(v, "upload_id", upload_id) && ret;
    return ret;
}
void ResultItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    GetDownloadSignatureReply::ResultItem::AddToJson(js, v);
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "upload_id", upload_id);
    return;
}
bool ErrorItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = CreateMultipartUploadSignatureReply::ErrorItem::FromJson(v);
    ret = GetMember(v, "upload_id", upload_id) && ret;
    return ret;
}
void ErrorItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    CreateMultipartUploadSignatureReply::ErrorItem::AddToJson(js, v);
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "upload_id", upload_id);
    return;
}
}  // namespace CompleteMultipartUploadSignatureReply

namespace UploadPartSignatureReply {
bool ResultItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = CompleteMultipartUploadSignatureReply::ResultItem::FromJson(v);
    ret = GetMember(v, "part_number", part_number) && ret;
    return ret;
}
void ResultItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    CompleteMultipartUploadSignatureReply::ResultItem::AddToJson(js, v);
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "part_number", part_number);
    return;
}
bool ErrorItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = CompleteMultipartUploadSignatureReply::ErrorItem::FromJson(v);
    ret = GetMember(v, "part_number", part_number) && ret;
    return ret;
}
void ErrorItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    CompleteMultipartUploadSignatureReply::ErrorItem::AddToJson(js, v);
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "part_number", part_number);
    return;
}
}  // namespace UploadPartSignatureReply

namespace CreateAssetReply {
bool ResultItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "param_index", param_index) && ret;
    ret = GetMember(v, "id", id) && ret;
    return ret;
}
void ResultItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "param_index", param_index);
    js.AddMemberTo(v, "id", id);
    return;
}
bool ErrorItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "param_index", param_index) && ret;
    ret = GetMember(v, "id", id) && ret;
    ret = GetMember(v, "message", message) && ret;
    return ret;
}
void ErrorItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "param_index", param_index);
    js.AddMemberTo(v, "id", id);
    js.AddMemberTo(v, "message", message);
    return;
}
}  // namespace CreateAssetReply

namespace GetChildNodeCountReply {
bool ResultItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "param_index", param_index) && ret;
    ret = GetMember(v, "count", count) && ret;
    return ret;
}
void ResultItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "param_index", param_index);
    js.AddMemberTo(v, "count", count);
    return;
}
bool ErrorItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "param_index", param_index) && ret;
    ret = GetMember(v, "message", message) && ret;
    return ret;
}
void ErrorItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "param_index", param_index);
    js.AddMemberTo(v, "message", message);
    return;
}
}  // namespace GetChildNodeCountReply

namespace GetChildNodeIDInRangeReply {
bool Result::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "nodes", nodes) && ret;
    return ret;
}
void Result::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "nodes", nodes);
    return;
}
bool Error::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "message", message) && ret;
    return ret;
}
void Error::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "message", message);
    return;
}
}  // namespace GetChildNodeIDInRangeReply

namespace GetNodeBriefByIDReply {
bool ResultItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "id", id) && ret;
    ret = GetMember(v, "name", name) && ret;
    ret = GetMember(v, "type", type) && ret;
    ret = GetMember(v, "file_format", file_format) && ret;
    ret = GetMember(v, "child_count", child_count) && ret;
    ret = GetMember(v, "preview_url", preview_url) && ret;
    ret = GetMember(v, "status", status) && ret;
    ret = GetMember(v, "permission", permission) && ret;
    ret = GetMember(v, "created_date", created_date) && ret;
    ret = GetMember(v, "updated_date", updated_date) && ret;
    // ret = GetMember(v, "permission_mask", permission_mask) && ret;
    ret = GetMember(v, "icon_url", icon_url) && ret;
    ret = GetMember(v, "node_type", node_type) && ret;
    ret = GetMember(v, "creator", creator) && ret;
    ret = GetMember(v, "capacity", capacity) && ret;
    ret = GetMember(v, "full_path_name", full_path_name) && ret;
    ret = GetMember(v, "full_path_id", full_path_id) && ret;
    ret = GetMember(v, "direct_child_count", direct_child_count) && ret;
    ret = GetMember(v, "total_leaf_count", total_leaf_count) && ret;
    ret = GetMember(v, "direct_directory_count", direct_directory_count) && ret;
    ret = GetMember(v, "display_mode", display_mode) && ret;
    ret = GetMember(v, "name_pinyin", name_pinyin) && ret;
    ret = GetMember(v, "param_index", param_index) && ret;
    return ret;
}
void ResultItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "id", id);
    if (!name.empty()) js.AddMemberTo(v, "name", name);
    if (!type.empty()) js.AddMemberTo(v, "type", type);
    if (!file_format.empty()) js.AddMemberTo(v, "file_format", file_format);
    if (!preview_url.empty()) js.AddMemberTo(v, "preview_url", preview_url);
    if (!status.empty()) js.AddMemberTo(v, "status", status);
    if (!permission.empty()) js.AddMemberTo(v, "permission", permission);
    if (!created_date.empty()) js.AddMemberTo(v, "created_date", created_date);
    if (!updated_date.empty()) js.AddMemberTo(v, "updated_date", updated_date);
    if (!icon_url.empty()) js.AddMemberTo(v, "icon_url", icon_url);
    if (!node_type.empty()) js.AddMemberTo(v, "node_type", node_type);
    if (!creator.empty()) js.AddMemberTo(v, "creator", creator);
    if (!display_mode.empty()) js.AddMemberTo(v, "display_mode", display_mode);
    if (!name_pinyin.empty()) js.AddMemberTo(v, "name_pinyin", name_pinyin);
    if (!full_path_id.empty()) js.AddMemberTo(v, "full_path_id", full_path_id);
    if (!full_path_name.empty()) js.AddMemberTo(v, "full_path_name", full_path_name);
    // if(permission_mask > 0) js.AddMemberTo(v, "permission_mask", permission_mask);
    if (param_index >= 0) js.AddMemberTo(v, "param_index", param_index);
    if (child_count >= 0) js.AddMemberTo(v, "child_count", child_count);
    if (capacity >= 0) js.AddMemberTo(v, "capacity", capacity);
    if (direct_child_count >= 0) js.AddMemberTo(v, "direct_child_count", direct_child_count);
    if (total_leaf_count >= 0) js.AddMemberTo(v, "total_leaf_count", total_leaf_count);
    if (direct_directory_count >= 0)
        js.AddMemberTo(v, "direct_directory_count", direct_directory_count);
    return;
}
bool ErrorItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "id", id) && ret;
    ret = GetMember(v, "param_index", param_index) && ret;
    ret = GetMember(v, "message", message) && ret;
    return ret;
}
void ErrorItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "id", id);
    js.AddMemberTo(v, "param_index", param_index);
    js.AddMemberTo(v, "message", message);
    return;
}
}  // namespace GetNodeBriefByIDReply

namespace CreateTsaReply {
bool ResultItem::FromJson(const rapidjson::Value& v) {
    using json::GetMember;
    bool ret = true;
    ret = GetMember(v, "param_index", param_index) && ret;
    ret = GetMember(v, "asset_id", asset_id) && ret;
    ret = GetMember(v, "tsa_id", tsa_id) && ret;
    ret = GetMember(v, "asset_hash", asset_hash) && ret;
    ret = GetMember(v, "tsa_url", tsa_url) && ret;
    ret = GetMember(v, "tsa_item_id", tsa_item_id) && ret;
    ret = GetMember(v, "tsa_title", tsa_title) && ret;
    ret = GetMember(v, "company", company) && ret;
    ret = GetMember(v, "tsa_info", tsa_info) && ret;
    ret = GetMember(v, "tsa_description", tsa_description) && ret;
    ret = GetMember(v, "created_date", created_date) && ret;
    return ret;
}
void ResultItem::AddToJson(json::JsonDoc& js, rapidjson::Value& v) const {
    if (!v.IsObject()) v.SetObject();
    js.AddMemberTo(v, "param_index", param_index);
    js.AddMemberTo(v, "asset_id", asset_id);
    js.AddMemberTo(v, "tsa_id", tsa_id);
    js.AddMemberTo(v, "asset_hash", asset_hash);
    js.AddMemberTo(v, "tsa_url", tsa_url);
    js.AddMemberTo(v, "tsa_item_id", tsa_item_id);
    js.AddMemberTo(v, "tsa_title", tsa_title);
    js.AddMemberTo(v, "company", company);
    js.AddMemberTo(v, "tsa_info", tsa_info);
    js.AddMemberTo(v, "tsa_description", tsa_description);
    js.AddMemberTo(v, "created_date", created_date);
    return;
}
}  // namespace CreateTsaReply