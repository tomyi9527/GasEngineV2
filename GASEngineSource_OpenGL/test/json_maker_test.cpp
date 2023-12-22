#include "utils/json_maker.h"
#include <limits>

int main() { return 0; }

//#include "gtest/gtest.h"
//
// using namespace storage::utils::json; // NOLINT
//
//#define PI 3.14159265
//#define INFO "this is pi"
//
// JsonDoc GenerateSampleDoc() {
//    JsonDoc js;
//    js.AddMember("code", 0);
//    auto& result = js.AddMember("result");
//    js.AddMemberTo(result, "id", -10);
//    js.AddMemberTo(result, "size", std::numeric_limits<uint64_t>::max());
//    js.AddMemberTo(result, "ratio", PI);
//    js.AddMemberTo(result, "info", INFO);
//    js.AddMemberTo(result, "info2", std::string(INFO));
//    auto & array_ = js.AddMemberTo(result, "array").SetArray();
//    js.PushBackTo(array_, INFO);
//    return js;
//}
//
// constexpr char expected_output[] =
// R"({"code":0,"result":{"id":-10,"size":18446744073709551615,"ratio":3.14159265,"info":"this is
// pi","info2":"this is pi","array":["this is pi"]}})";
//
//
// TEST(JsonMaker, doc_generate) {
//    ASSERT_EQ(GenerateSampleDoc().GetString(), expected_output);
//}
//
//// PointerTakeValue 调用了 GetElement
// TEST(JsonMaker, pointer_and_get_element) {
//    JsonDoc js = GenerateSampleDoc();
//    int id = 0;
//    PointerTakeValue(js.GetDocument(), rapidjson::Pointer("/result/id"), id);
//    ASSERT_EQ(id, -10);
//
//    double ratio = 0;
//    PointerTakeValue(js.GetDocument(), rapidjson::Pointer("/result/ratio"), ratio);
//    ASSERT_EQ(ratio, PI);
//
//    uint64_t size = 0;
//    PointerTakeValue(js.GetDocument(), rapidjson::Pointer("/result/size"), size);
//    ASSERT_EQ(size, std::numeric_limits<uint64_t>::max());
//
//    std::string info;
//    info.clear();
//    PointerTakeValue(js.GetDocument(), rapidjson::Pointer("/result/info"), info);
//    ASSERT_EQ(info, INFO);
//
//    info.clear();
//    PointerTakeValue(js.GetDocument(), rapidjson::Pointer("/result/info2"), info);
//    ASSERT_EQ(info, INFO);
//
//    const rapidjson::Value& v = js.GetDocument()["result"]["array"];
//    ASSERT_TRUE(v.IsArray());
//
//    std::vector<std::string> array_;
//    GetElement(v, array_);
//    ASSERT_EQ(array_.size(), 1);
//    ASSERT_EQ(array_[0], INFO);
//}
//
//
// TEST(JsonMaker, fail_test) {
//    JsonDoc js = GenerateSampleDoc();
//    {
//        double id = 0;
//        PointerTakeValue(js.GetDocument(), rapidjson::Pointer("/result/id"), id);
//        ASSERT_EQ(id, 0);
//    }
//    {
//        std::string id = "";
//        PointerTakeValue(js.GetDocument(), rapidjson::Pointer("/result/id"), id);
//        ASSERT_EQ(id, "");
//    }
//    {
//        bool id = 0;
//        PointerTakeValue(js.GetDocument(), rapidjson::Pointer("/result/id"), id);
//        ASSERT_EQ(id, false);
//    }
//    {
//        float id = 0;
//        PointerTakeValue(js.GetDocument(), rapidjson::Pointer("/result/id"), id);
//        ASSERT_EQ(id, 0);
//    }
//}
//
//
//#pragma region GeneratedCode
// using UNCERTAIN_TYPE = void*;
// struct _type_1 : public JsonSerializeInterface {
//    DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
//    int32_t code;
//    struct _type_2 : public JsonSerializeInterface {
//        DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
//        using _type_3 =
//            struct _type_4 : public JsonSerializeInterface {
//            DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
//            std::string id;
//            std::string name;
//            int32_t type;
//            std::string parent_id;
//            std::string preview_url;
//            std::string file_type;
//            int32_t convert_status;
//            using _type_5 =
//                struct _type_6 : public JsonSerializeInterface {
//                DERIVED_FROM_JSON_SERIALIZE_INTERFACE;
//                std::string tag_id;
//                std::string text;
//                int32_t x;
//                int32_t y;
//                int32_t w;
//                int32_t h;
//            };
//            std::vector<_type_5>  tags;
//            using _type_7 =
//                std::string;
//            std::vector<_type_7>  category_ids;
//        };
//        std::vector<_type_3>  nodes;
//    } result;
//};
// using SavedJsonSampleType = _type_1;
//
//// 提供allocator
// JsonDoc js;
//
// bool _type_1::FromJson(const rapidjson::Value& v) {
//    bool ret = true;
//    ret = GetMember(v, "code", code) && ret;
//    ret = GetMember(v, "result", result) && ret;
//    return ret;
//}
//
// void _type_1::AddToJson(rapidjson::Value& v) const {
//    if (!v.IsObject())
//        v.SetObject();
//    js.AddMemberTo(v, "code", code);
//    js.AddMemberTo(v, "result", result);
//    return;
//}
//
// bool _type_1::_type_2::FromJson(const rapidjson::Value& v) {
//    bool ret = true;
//    ret = GetMember(v, "nodes", nodes) && ret;
//    return ret;
//}
//
// void _type_1::_type_2::AddToJson(rapidjson::Value& v) const {
//    if (!v.IsObject())
//        v.SetObject();
//    js.AddMemberTo(v, "nodes", nodes);
//    return;
//}
//
// bool _type_1::_type_2::_type_4::FromJson(const rapidjson::Value& v) {
//    bool ret = true;
//    ret = GetMember(v, "id", id) && ret;
//    ret = GetMember(v, "name", name) && ret;
//    ret = GetMember(v, "type", type) && ret;
//    ret = GetMember(v, "parent_id", parent_id) && ret;
//    ret = GetMember(v, "preview_url", preview_url) && ret;
//    ret = GetMember(v, "file_type", file_type) && ret;
//    ret = GetMember(v, "convert_status", convert_status) && ret;
//    ret = GetMember(v, "tags", tags) && ret;
//    ret = GetMember(v, "category_ids", category_ids) && ret;
//    return ret;
//}
//
// void _type_1::_type_2::_type_4::AddToJson(rapidjson::Value& v) const {
//    if (!v.IsObject())
//        v.SetObject();
//    js.AddMemberTo(v, "id", id);
//    js.AddMemberTo(v, "name", name);
//    js.AddMemberTo(v, "type", type);
//    js.AddMemberTo(v, "parent_id", parent_id);
//    js.AddMemberTo(v, "preview_url", preview_url);
//    js.AddMemberTo(v, "file_type", file_type);
//    js.AddMemberTo(v, "convert_status", convert_status);
//    js.AddMemberTo(v, "tags", tags);
//    js.AddMemberTo(v, "category_ids", category_ids);
//    return;
//}
//
// bool _type_1::_type_2::_type_4::_type_6::FromJson(const rapidjson::Value& v) {
//    bool ret = true;
//    ret = GetMember(v, "tag_id", tag_id) && ret;
//    ret = GetMember(v, "text", text) && ret;
//    ret = GetMember(v, "x", x) && ret;
//    ret = GetMember(v, "y", y) && ret;
//    ret = GetMember(v, "w", w) && ret;
//    ret = GetMember(v, "h", h) && ret;
//    return ret;
//}
//
// void _type_1::_type_2::_type_4::_type_6::AddToJson(rapidjson::Value& v) const {
//    if (!v.IsObject())
//        v.SetObject();
//    js.AddMemberTo(v, "tag_id", tag_id);
//    js.AddMemberTo(v, "text", text);
//    js.AddMemberTo(v, "x", x);
//    js.AddMemberTo(v, "y", y);
//    js.AddMemberTo(v, "w", w);
//    js.AddMemberTo(v, "h", h);
//    return;
//}
//#pragma endregion
//
// TEST(JsonMaker, derived_from_json_serialize_interface) {
//    JsonDoc js_to_save;
//
//    SavedJsonSampleType to_save;
//    to_save.code = 1;
//    using NodeTy = SavedJsonSampleType::_type_2::_type_4;
//    NodeTy node;
//    node.category_ids.push_back("12345");
//    node.category_ids.push_back("67890");
//    node.convert_status = 100;
//    node.file_type = "png";
//    node.type = std::numeric_limits<decltype(node.type)>::max();
//    node.id = "a";
//
//    to_save.result.nodes.push_back(node);
//    to_save.result.nodes.push_back(node);
//
//    to_save.AddToJson(js_to_save.GetDocument());
//    std::string saved = js_to_save.GetPrettyString();
//
//    // std::cout << saved;
//
//    // 初始化-->序列化-->反序列化-->序列化
//    // 两次序列化结果应相同
//    SavedJsonSampleType loaded;
//    loaded.FromJson(saved);
//
//    JsonDoc new_js_to_save;
//    loaded.AddToJson(new_js_to_save.GetDocument());
//    std::string new_saved = js_to_save.GetPrettyString();
//
//    ASSERT_EQ(saved, new_saved);
//}