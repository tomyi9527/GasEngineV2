// Copyright 2020 Beanpliu or Tencent?
// author : njueebeanliu
// rapidjson 的一个包装
#ifndef SRC_UTILS_JSONMAKER_H_
#define SRC_UTILS_JSONMAKER_H_

#include <assert.h>
#include <string>
#include <utility>
#include <vector>
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"

//template <template <class...> class Target, class T>
//struct is_template_of {
//    static const bool value = false;
//};
//template <template <class...> class Target, class... Args>
//struct is_template_of<Target, Target<Args...>> {
//    static const bool value = true;
//};

template <typename T, typename = void>
struct is_iterable : std::false_type {};

// this gets used only when we can call std::begin() and std::end() on that type
template <typename T>
struct is_iterable<T, std::void_t<
        decltype(std::begin(std::declval<T>())),
        decltype(std::end(std::declval<T>()))
    >> : std::true_type {
};

// Here is a helper:
template <typename T>
constexpr bool is_iterable_v = is_iterable<T>::value;


namespace json {

// 本文件中规定的内容json数据存储接口
#define DERIVED_FROM_JSON_SERIALIZE_INTERFACE                              \
    bool FromJson(const rapidjson::Value& v) override;                     \
    void AddToJson(json::JsonDoc& js, rapidjson::Value& v) const override; \
    bool FromJson(const std::string& input) override {                     \
        return JsonSerializeInterface::FromJson(input);                    \
    }

class JsonDoc;

struct JsonSerializeInterface {
    virtual bool FromJson(const std::string& input) {
        rapidjson::Document doc;
        doc.Parse(input.data(), input.size());
        if (doc.IsObject() || doc.IsArray())
            return FromJson(doc);
        else
            return false;
    }
    virtual bool FromJson(const rapidjson::Value& v) = 0;
    virtual void AddToJson(JsonDoc& js, rapidjson::Value& v) const = 0;
};

// struct a : public JsonSerializeInterface { DERIVED_FROM_JSON_SERIALIZE_INTERFACE; }
// 需要自己写实现。
// 如果不想写请用 intellibase/ML/json_generator.h 生成代码。
// 由于命名方式修改了，json_generator.h中可能宏和基类名称会和本文件有所不同。

// generate json string of the document
inline std::string s_GetString(const rapidjson::Value& v) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    if (!v.Accept(writer)) {
        return "";
    } else {
        return std::string(buffer.GetString(), buffer.GetSize());
    }
}

// generate pretty json string of the document
inline std::string s_GetPrettyString(const rapidjson::Value& v) {
    rapidjson::StringBuffer buffer;
    // if need allowing nan or inf:
    // use type: rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::UTF8<>, rapidjson::CrtAllocator, rapidjson::kWriteNanAndInfFlag>
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    if (!v.Accept(writer)) {
        return "";
    } else {
        return std::string(buffer.GetString(), buffer.GetSize());
    }
}

// generate json
class JsonDoc {
    rapidjson::Document doc;

 public:
    // default initialize as Object
    JsonDoc() { SetObject(); }

    // move is okay
    JsonDoc(JsonDoc&& rhs) {
        SetObject();
        doc.Swap(rhs.doc);
    }

    // copy is not allowed
    JsonDoc(const JsonDoc& rhs) = delete;

    // move is okay
    JsonDoc& operator=(JsonDoc&& rhs) {
        SetObject();
        doc.Swap(rhs.doc);
        return *this;
    }

    // copy is not allowed
    JsonDoc& operator=(const JsonDoc& rhs) = delete;

    // set type for root doc.
    inline JsonDoc& SetObject() {
        doc.SetObject();
        return *this;
    }
    inline JsonDoc& SetArray() {
        doc.SetArray();
        return *this;
    }

    // AddMember
    // return reference of the NEW INSERTED rapidjson::Value.
    // differrent than rapidjson's fluent_api.
    rapidjson::Value& AddMember(const std::string& name) { return AddMemberTo(doc, name); }

    // return the reference of DOCUMENT ROOT
    template <typename ValueType>
    rapidjson::Value& AddMember(const std::string& name, ValueType&& value) {
        return AddMemberTo(doc, name, std::forward<ValueType>(value));
    }

    // AddMemberTo
    // two kinds of interface:
    //     AddMemberTo(rapidjson::Value&, "name", val);     insert with known value.
    //     AddMemberTo(rapidjson::Value&, "name");          insert as object, value not set.
    //

    // AddMemberTo(rapidjson::Value&, "name");              returns reference of NEW INSERTED Value.
    template <typename GenericValue>
    rapidjson::Value& AddMemberTo(GenericValue& v, const std::string& name) {
        assert(v.IsObject());
        // in rapidjson, AddMember inserts the value at tail. so we return thereference of the last
        // element here.
        v.AddMember(rapidjson::Value(name.data(), (uint32_t)name.size(), doc.GetAllocator()).Move(),
                    rapidjson::Value(rapidjson::kObjectType), doc.GetAllocator());
        return (*(v.MemberBegin() + (v.MemberCount() - 1))).value.SetObject();
    }

    template <typename GenericValue>
    rapidjson::Value& AddMemberTo(GenericValue& v, const std::string& name, const char* p) {
        assert(v.IsObject());
        v.AddMember(rapidjson::Value(name.data(), name.size(), doc.GetAllocator()).Move(),
                    rapidjson::Value(p, strlen(p), doc.GetAllocator()).Move(), doc.GetAllocator());
        return (*(v.MemberBegin() + (v.MemberCount() - 1))).value;
    }

    // // same as above const std::string&. commented
    // template<typename GenericValue>
    // rapidjson::Value& AddMemberTo(GenericValue& v, std::string&& name) {
    //    assert(v.IsObject());
    //    v.AddMember(
    //        rapidjson::Value(name.data(), name.size(), doc.GetAllocator()).Move(),
    //        rapidjson::Value(rapidjson::kObjectType),
    //        doc.GetAllocator());
    //    return (*(v.MemberBegin() + (v.MemberCount() - 1))).value.SetObject();
    //}

    //
    // AddMemberTo(rapidjson::Value&, "name", val);             returns reference of the first
    // parameter. val supports:
    template <typename GenericValue, class Iterable>
    std::enable_if_t<is_iterable<std::decay_t<Iterable>>::value && !std::is_same_v<std::string, std::decay_t<Iterable>>,
                     rapidjson::Value&>
    AddMemberTo(GenericValue& v, const std::string& name,
                                  const Iterable& value) {
        assert(v.IsObject());
        rapidjson::Value v_temp;
        v_temp.SetArray();
        for (auto& m : value) {
            PushBackTo(v_temp, m);
        }
        AddMemberTo(v, name, v_temp.Move());
        return v;
    }

    template <typename GenericValue, typename JsonOrOtherType>
    std::enable_if_t<std::is_same_v<std::string, std::decay_t<JsonOrOtherType>> || !is_iterable<std::decay_t<JsonOrOtherType>>::value,
                     rapidjson::Value&>
    AddMemberTo(GenericValue& v, const std::string& name, JsonOrOtherType&& value) {
        assert(v.IsObject());
        if constexpr (std::is_base_of_v<JsonSerializeInterface, std::decay_t<JsonOrOtherType>>) {
            rapidjson::Value v_temp;
            v_temp.SetObject();
            value.AddToJson(*this, v_temp);
            AddMemberTo(v, name, v_temp.Move());
        } else if constexpr (std::is_same_v<std::string, std::decay_t<JsonOrOtherType>>) {
            v.AddMember(rapidjson::Value(name.data(), name.size(), doc.GetAllocator()).Move(),
                        rapidjson::Value(value.data(), value.size(), doc.GetAllocator()).Move(),
                        doc.GetAllocator());
            return v;
        } else {
            v.AddMember(rapidjson::Value(name.data(), name.size(), doc.GetAllocator()).Move(),
                        std::forward<JsonOrOtherType>(value),  // 通过rapidjson提供的重载实现
                        doc.GetAllocator());
        }
        return v;
    }

    // return reference of v (first parameter)
    template <typename GenericValue, typename JsonOrOtherType>
    rapidjson::Value& PushBackTo(GenericValue& v, JsonOrOtherType&& value) {
        assert(v.IsArray());
        if constexpr (std::is_base_of_v<JsonSerializeInterface, std::decay_t<JsonOrOtherType>>) {
            rapidjson::Value v_temp;
            value.AddToJson(*this, v_temp);
            return v.PushBack(v_temp.Move(), doc.GetAllocator());
        } else if constexpr (std::is_same_v<std::string, std::decay_t<JsonOrOtherType>>) {
            return v.PushBack(
                rapidjson::Value(value.data(), value.size(), doc.GetAllocator()).Move(),
                doc.GetAllocator());
        } else if constexpr (std::is_same_v<rapidjson::Value, std::decay_t<JsonOrOtherType>>) {
            return v.PushBack(value.Move(), doc.GetAllocator());
        } else if constexpr (is_iterable_v<std::decay_t<JsonOrOtherType>>) {
            rapidjson::Value v_tmp;
            v_tmp.SetArray();
            for (const auto& m : value) {
                PushBackTo(v_tmp, m);
            }
            return v.PushBack(v_tmp.Move(), doc.GetAllocator());
        } else {
            return v.PushBack(std::forward<JsonOrOtherType>(value), doc.GetAllocator());
        }
    }
    // 局限：
    // 暂时不许push_back一个array到另一个array。

    // utility
    // return the allocator of the document.
    rapidjson::MemoryPoolAllocator<>& GetAllocator() { return doc.GetAllocator(); }

    rapidjson::Document& GetDocument() { return doc; }
    const rapidjson::Document& GetDocument() const { return doc; }

    // generate json string of the document
    std::string GetString() const { return s_GetString(doc); }

    // generate pretty json string of the document
    std::string GetPrettyString() const { return s_GetPrettyString(doc); }
};

// convert rapidjson value of stringType INTO std string
inline std::string ToString(const rapidjson::Value& value) {
    if (!value.IsString())
        return "";
    else
        return std::string(value.GetString(), value.GetStringLength());
}

// GetElement, take out value from "v" by the type of "out".
inline bool GetElement(const rapidjson::Value& v, std::string& out) {
    if (v.IsString()) {
        out = ToString(v);
        return true;
    } else
        return false;
}

inline bool GetElement(const rapidjson::Value& v, int32_t& out) {
    if (v.IsInt()) {
        out = v.GetInt();
        return true;
    } else
        return false;
}

inline bool GetElement(const rapidjson::Value& v, uint32_t& out) {
    if (v.IsUint()) {
        out = v.GetUint();
        return true;
    } else
        return false;
}

inline bool GetElement(const rapidjson::Value& v, int64_t& out) {
    if (v.IsInt64()) {
        out = v.GetInt64();
        return true;
    } else
        return false;
}

inline bool GetElement(const rapidjson::Value& v, uint64_t& out) {
    if (v.IsUint64()) {
        out = v.GetUint64();
        return true;
    } else
        return false;
}

inline bool GetElement(const rapidjson::Value& v, float& out) {
    if (v.IsFloat()) {
        out = v.GetFloat();
        return true;
    } else
        return false;
}

inline bool GetElement(const rapidjson::Value& v, double& out) {
    if (v.IsDouble()) {
        out = v.GetDouble();
        return true;
    } else
        return false;
}

inline bool GetElement(const rapidjson::Value& v, bool& out) {
    if (v.IsBool()) {
        out = v.GetBool();
        return true;
    } else
        return false;
}

template <typename JsonType>
inline bool GetElement(const rapidjson::Value& v, JsonType& out) {
    static_assert(std::is_base_of_v<JsonSerializeInterface, JsonType>,
                  "JsonType should be a type derived from JsonSerializeInterface.");
    if (v.IsObject()) {
        return out.FromJson(v);
    } else
        return false;
}

template <typename AnyType>
inline bool GetElement(const rapidjson::Value& v, std::vector<AnyType>& out) {
    if (v.IsArray()) {
        bool ret = true;
        out.reserve(v.Size());
        for (int i = 0; i < (int)v.Size(); ++i) {
            AnyType tmp;
            if constexpr (std::is_integral_v<AnyType>) {
                tmp = 0;
            } else {
                tmp = AnyType();
            }
            ret = GetElement(v.Begin()[i], tmp) && ret;
            out.push_back(std::move(tmp));
        }
        return ret;
    } else
        return false;
}

// GetMember
template <typename AnyType>
inline bool GetMember(const rapidjson::Value& v, const char* name, AnyType& out) {
    if (!v.IsObject()) return false;
    rapidjson::Value::ConstMemberIterator itr = v.FindMember(name);
    return itr != v.MemberEnd() ? GetElement(itr->value, out) : false;
}

// PointerTakeValue
template <typename AnyType>
inline bool PointerTakeValue(const rapidjson::Value& d, const rapidjson::Pointer& p, AnyType& out) {
    if (const rapidjson::Value* ptr = p.Get(d))
        return GetElement(*ptr, out);
    else
        return false;
}

template <typename T>
std::string stringify(const T& o) {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
    o.Accept(writer);
    return sb.GetString();
}

inline bool MemberIsType(const rapidjson::Value& value, const std::string& name,
                         rapidjson::Type type) {
    return MemberIsType(value, name.c_str(), type);
}

inline bool MemberIsType(const rapidjson::Value& value, const char* name, rapidjson::Type type) {
    if (!value.IsObject()) return false;
    auto it = value.FindMember(name);
    if (it == value.MemberEnd()) {
        return false;
    } else {
        return it->value.GetType() == type;
    }
}

}  // namespace json

#endif  // SRC_UTILS_JSONMAKER_H_