#pragma once

#include <inttypes.h>
#include <algorithm>
#include <sstream>
#include <string>
#include <array>
#include <set>
#include <list>
#include <vector>
#include <memory>
#include <iomanip>
#include <mutex>
#include <assert.h>
#include "utf.h"
#include "entry_type.h"
#include "param_block_types.h"
#include "binary_to_printable.h"
#include "error_printer.h"

namespace max {

template<typename T>
inline const T* GetAs(const std::string& data, int pos) {
    static_assert(std::is_trivial_v<T>, "should use a trival type.");
    if (data.size() < pos + sizeof(T)) {
        PrintError(MaxBinaryParserError, LEVEL_FATAL, "out-of-range access", true);
    }
    return (const T*)&(data[pos]);
}

// basic class
class mNode {
public:
    virtual ~mNode() {}
    enum _Type {
        Value, Container
    } type;

    uint16_t id = 0;
    std::string GetHexIDString() const {
        std::stringstream ss;
        ss << "0x" << std::hex << std::setw(4) << std::setfill('0') << id;
        return ss.str();
    }
    virtual std::string ToString(int indent = 0) const = 0;
};

class mContainer : public mNode {
public:
    mContainer() { type = Container; }
    std::vector<std::unique_ptr<mNode>> content;

    using mNodeIterator = std::vector<std::unique_ptr<mNode>>::iterator;
    using mNodeConstIterator = std::vector<std::unique_ptr<mNode>>::const_iterator;

    mNodeConstIterator ContentBegin() const {
        return content.cbegin();
    }
    mNodeIterator ContentBegin() {
        return content.begin();
    }

    mNodeConstIterator ContentEnd() const {
        return content.cend();
    }
    mNodeIterator ContentEnd() {
        return content.end();
    }

    template<typename Pred>
    mNodeConstIterator GetFirstContentIter(Pred pred) const {
        return GetFirstContentIter(pred, content.begin());
    }
    template<typename Pred>
    mNodeConstIterator GetFirstContentIter(Pred pred, mNodeConstIterator start_pos) const {
        for (mNodeConstIterator it = start_pos; it != content.end(); ++it) {
            if (pred(it->get())) {
                return it;
            }
        }
        return content.end();
    }

    const mNode* GetFirstContentByID(uint16_t id) const {
        auto ret = GetFirstContentIterByID(id);
        if (ret == ContentEnd()) {
            return nullptr;
        } else {
            return ret->get();
        }
    }

    mNodeConstIterator GetFirstContentIterByID(uint16_t id) const {
        return GetFirstContentIterByID(id, ContentBegin());
    }

    mNodeConstIterator GetFirstContentIterByID(uint16_t id, mNodeConstIterator start_pos) const {
        for (mNodeConstIterator it = start_pos; it != content.end(); ++it) {
            if ((*it)->id == id) {
                return it;
            }
        }
        return content.end();
    }

    std::string ToString(int indent) const override {
        std::stringstream ss;
        std::string indent_str(indent, '\t');
        ss << GetHexIDString() << ": {" << std::endl;
        int index = 0;
        for (const auto& m : content) {
            ss << indent_str << '\t' << "<" << index++ << ">  " << m->ToString(indent + 1) << std::endl;
        }
        ss << indent_str << "}";
        return ss.str();
    }
};

class mValue : public mNode {
public:
    mValue() { type = Value; }
    std::string data;

    std::string ToString(int indent) const override {
        std::stringstream ss;
        ss << GetHexIDString() << ": [size: " << data.size() << ", content: " << GuessContent() << "]";
        return ss.str();
    }

protected:
    virtual std::string GuessContent() const {
        if (data.size() == 0) {
            return "''";
        } else if (data.size() == 1) {
            return std::string(data[0] ? "true" : "false") + " or " + BinaryToHexPrintable(data[0]);
        } else if (data.size() == 2) {
            return std::to_string(*((int16_t*)(data.data())));
        } else if (data.size() == 4) {
            static_assert(sizeof(float) == 4, "float should be 4 bytes");
            return std::to_string(*((int32_t*)(data.data()))) + " or " + std::to_string(*((float*)(data.data())));
        } else if (data.size() == 8) {
            return std::to_string(*((int64_t*)(data.data()))) + " or " + std::to_string(*((double*)(data.data())));
        } else {
            return BinaryToHexPrintable(data) + " or " + BinaryToAsciiPrintable(data);
        }
    }
};


// extended class

class mStringWithLengthValue : public mValue {
public:
    std::string GuessContent() const override {
        std::stringstream ss;
        ss << "'" << GetValue().second << "'";
        return ss.str();
    }
    std::pair<uint32_t, std::string> GetValue() const {
        auto str_size = *((uint32_t*)&(data[0]));
        std::string wdata = data;
        wdata.push_back('\0');
        return { str_size, UTF16ToUTF8((int16_t*)(wdata.data() + 4)) };
    }
};

template<typename Type>
class mSingleValue : public mValue {
public:
    std::string GuessContent() const override {
        return std::to_string(GetValue());
    }
    Type GetValue() const {
        return *((Type*)&(data[0]));
    }
};
using mUint32Value = mSingleValue<uint32_t>;
using mFloatValue = mSingleValue<float>;

class mBoolValue : public mValue {
public:
    std::string GuessContent() const override {
        auto value = *((uint32_t*)&(data[0]));
        return (value ? "True" : "False");
    }
};

class mUCSStringValue : public mValue {
public:
    std::string GuessContent() const override {
        std::stringstream ss;
        ss << "'" << GetValue() << "'";
        return ss.str();
    }
    std::string GetValue() const {
        std::string wdata = data;
        wdata.push_back('\0');
        return UTF16ToUTF8((int16_t*)(wdata.data()));
    }
};

class mStringValue : public mValue {
public:
    std::string GuessContent() const override {
        std::stringstream ss;
        ss << "'" << GetValue() << "'";
        return ss.str();
    }
    std::string GetValue() const {
        return data;
    }
};

class mClassIDValue : public mValue {
public:
    std::string GuessContent() const override {
        std::stringstream ss;
        if (data.size() != 12) {
            return mValue::GuessContent();
        }
        auto id1 = *((uint32_t*)&(data[0]));// classID1
        auto id2 = *((uint32_t*)&(data[4]));// classID2
        auto id3 = *((uint32_t*)&(data[8]));// superClassId
        ss << "0x" << std::hex << std::setw(4) << std::setfill('0')
            << id1 << ", 0x"
            << id2 << ", 0x"
            << id3;
        return ss.str();
    }
};

class mDllClassIDValue : public mValue {
public:
    struct IDWithDllIndex {
        int32_t dll_index; // maybe -1
        uint32_t id1;// classID1
        uint32_t id2;// classID2
        uint32_t id3;// superClassId

        std::string dll_name;
        std::string dll_description = "internal class";

        void InitFromData(const char* memory) {
            dll_index = *((int32_t*)&(memory[0]));// DllIndex
            id1 = *((uint32_t*)&(memory[4]));// classID1
            id2 = *((uint32_t*)&(memory[8]));// classID2
            id3 = *((uint32_t*)&(memory[12]));// superClassId
            if (Linker::DllInfo().size() > dll_index && dll_index >= 0 && Linker::DllInfo().at(dll_index).valid) {
                dll_name = Linker::DllInfo().at(dll_index).name;
                dll_description = Linker::DllInfo().at(dll_index).description;
            }
        }
    };
    std::string GuessContent() const override {
        std::stringstream ss;
        IDWithDllIndex ids = GetValue();
        ss << ids.dll_index << std::hex << std::setw(4) << std::setfill('0') << ", 0x"
            << ids.id1 << ", 0x"
            << ids.id2 << ", 0x"
            << ids.id3 << " (dll info: " << ids.dll_description << ")";
        return ss.str();
    }
    IDWithDllIndex GetValue() const {
        IDWithDllIndex ids;
        ids.InitFromData(data.data());
        return ids;
    }
};

struct uint8_printable {
    uint8_t value;
};

inline std::ostream& operator<<(std::ostream& s, const uint8_printable& v) {
    s << (int)v.value;
    return s;
}

template<typename Type>
class mArrayValue : public mValue {
public:
    std::string GuessContent() const override {
        static_assert(std::is_trivial_v<Type>, "not supported type");
        auto values = GetValue();
        std::stringstream ss;
        for (const auto& m : values) {
            ss << m << ", ";
        }
        auto ret = ss.str();
        if (ret.size() >= 2) {
            ret.pop_back();
            ret.pop_back();
        }
        return ret;
    }
    std::vector<Type> GetValue() const {
        std::vector<Type> ret;
        auto items = data.size() / sizeof(Type);
        ret.reserve(items);
        for (int i = 0; i < items; ++i) {
            ret.push_back(*((Type*)&(data[i * sizeof(Type)])));
        }
        return ret;
    }
};
using mReferenceIndexValue = mArrayValue<int32_t>;

class mPrintLessUint8Array : public mArrayValue<uint8_printable> {
public:
    constexpr static int max_length = 10;
    std::string GuessContent() const override {
        auto values = GetValue();
        std::stringstream ss;
        int idx = 0;
        for (const auto& m : values) {
            idx++;
            ss << m << ", ";
            if (idx > max_length) {
                ss << "..., ";
                break;
            }
        }
        auto ret = ss.str();
        if (ret.size() >= 2) {
            ret.pop_back();
            ret.pop_back();
        }
        return ret;
    }
};

inline std::ostream& operator<<(std::ostream& s, const FileAssetEntry& v) {
    s << BinaryToHexPrintable(v.id) << ", ";
    s << v.type << ", ";
    s << v.path1 << ", ";
    s << v.path2;

    return s;
}

class mFileAssetResourceValue : public mValue {
public:
    std::string GuessContent() const override {
        auto values = GetValue();
        std::stringstream ss;
        for (const auto& m : values) {
            ss << "[" << m << "], ";
        }
        ss << BinaryToHexPrintable(data);
        auto ret = ss.str();
        if (ret.size() >= 2) {
            ret.pop_back();
            ret.pop_back();
        }
        return ret;
    }
    std::vector<FileAssetEntry> GetValue() const {
        std::vector<FileAssetEntry> ret;
        int index = 0;
        for (; index < data.size();) {
            // header
            FileAssetEntry entry;
            if (data.size() < index + 16) {
                std::cerr << "FileAssetEntry out of range" << std::endl;
                throw std::runtime_error("FileAssetEntry out of range");
            }
            entry.id = data.substr(index, 16);
            index += 16;

            // take string
            auto take_string = [&index, this]() {
                int char_count = *(uint32_t*)(&(data[index]));
                index += sizeof(uint32_t);
                int next_text_len = (1 + char_count) * 2;
                // assign
                std::string wdata = data.substr(index, next_text_len);
                index += next_text_len;
                return UTF16ToUTF8((int16_t*)(wdata.data()));
            };

            entry.type = take_string();
            entry.path1 = take_string();
            entry.path2 = take_string();

            ret.push_back(std::move(entry));
        }
        return ret;
    }
};

template<typename hType, typename Type>
class mArrayValueWithHeader : public mValue {
public:
    std::string GuessContent() const override {
        static_assert(std::is_trivial_v<hType>, "not supported type");
        static_assert(std::is_trivial_v<Type>, "not supported type");
        auto values = GetValue();
        std::stringstream ss;
        ss << values.first << ", ";
        for (const auto& m : values.second) {
            ss << m << ", ";
        }
        auto ret = ss.str();
        if (ret.size() >= 2) {
            ret.pop_back();
            ret.pop_back();
        }
        return ret;
    }
    std::pair<hType, std::vector<Type>> GetValue() const {
        if (data.empty()) {
            return { 0, {} };
        }
        hType h = *((hType*)&(data[0]));
        std::vector<Type> ret;
        if (data.size() > sizeof(hType)) {
            auto items = (data.size() - sizeof(hType)) / sizeof(Type);
            ret.reserve(items);
            for (int i = 0; i < items; ++i) {
                ret.push_back(*((Type*)&(data[sizeof(hType) + i * sizeof(Type)])));
            }
        }
        return { h, std::move(ret) };
    }
};

// size is fixed
struct MeshFaceStructure {
    uint32_t v1;
    uint32_t v2;
    uint32_t v3;
    uint32_t smGroup;
    uint16_t unknown1;
    uint16_t submesh_index;
};
static_assert(sizeof(MeshFaceStructure) == 20, "error size of MeshFaceStructure");

inline std::ostream& operator << (std::ostream& s, const MeshFaceStructure& mesh) {
    s << "[smooth group: 0x"
        << BinaryToHexPrintable((const unsigned char*)&mesh.smGroup, sizeof(mesh.smGroup)) << ", submesh_index:"
        << mesh.submesh_index << ", unknown flag: 0x" 
        << BinaryToHexPrintable((const unsigned char*)&mesh.unknown1, sizeof(mesh.unknown1)) << ", vertices: [" 
        << mesh.v1 << ", " << mesh.v2 << ", " << mesh.v3 << "]]";
    //s << std::endl;
    return s;
}


class FlagInterpreter {
public:
    FlagInterpreter() {}
    FlagInterpreter(const std::vector<uint16_t>& flags) {
        Process(flags);
    }
    bool Process(const std::vector<uint16_t>& flags);

    bool has_smooth_group = false;
    uint32_t smooth_group = 0;  // 1-32对应于位
    bool has_submesh_index = false;
    uint16_t submesh_index = 0;
    bool has_unknown2 = false;
    uint32_t unknown2 = 0;
};

// size is not fixed
struct PolygonFaceStructure {
    std::vector<uint32_t> polygon_vertex;
    std::vector<std::pair<uint32_t, uint32_t>> edges;
    std::vector<uint16_t> flags;
    FlagInterpreter flag_interpreted;
};

inline std::ostream& operator << (std::ostream& s, const PolygonFaceStructure& poly) {
    s << "[";
    if (poly.polygon_vertex.size()) {
        s << "idx: [";
    }
    for (int i = 0; i < poly.polygon_vertex.size(); ++i) {
        s << poly.polygon_vertex[i];
        if (i + 1 != poly.polygon_vertex.size()) {
            s << ", ";
        } else {
            s << "], ";
        }
    }
    if (poly.edges.size()) {
        s << "edges: [";
    }
    for (int i = 0; i < poly.edges.size(); ++i) {
        s << poly.edges[i].first << "<->" << poly.edges[i].second;
        if (i + 1 != poly.edges.size()) {
            s << ", ";
        } else {
            s << "], ";
        }
    }
    s << "flags: [";
    for (int i = 0; i < poly.flags.size(); ++i) {
        s << poly.flags[i];
        if (i + 1 != poly.flags.size()) {
            s << ", ";
        } else {
            s << "], ";
        }
    }
    s << "flag_interpreted: [";
    s << "submesh: " << poly.flag_interpreted.submesh_index << ", ";
    s << "smooth_group: 0x" << BinaryToHexPrintable((const unsigned char*)&poly.flag_interpreted.smooth_group, sizeof(poly.flag_interpreted.smooth_group)) << ", ";
    s << "unknown2: " << poly.flag_interpreted.unknown2;
    s << "]";

    s << "]";
    //s << std::endl;
    return s;
}

class mPolygonFaceStrctureValue : public mValue {
public:
    std::string GuessContent() const override {
        auto values = GetValue();
        std::stringstream ss;
        for (const auto& m : values) {
            ss << m << ", ";
        }
        auto ret = ss.str();
        if (ret.size() >= 2) {
            ret.pop_back();
            ret.pop_back();
        }
        return ret;
    }
    std::vector<PolygonFaceStructure> GetValue() const {
        std::vector<PolygonFaceStructure> ret;
        uint32_t face_length = *((uint32_t*)&data[0]);
        for (int i = 4; i < data.size();) {
            // 32 bit per value
            // header:
            PolygonFaceStructure item;
            uint32_t index_length = *((uint32_t*)&data[i]);
            i += 4;
            item.polygon_vertex.reserve(index_length);
            while (item.polygon_vertex.size() < index_length && i + 4 <= data.size()) {
                item.polygon_vertex.push_back(*((uint32_t*)&data[i]));
                i += 4;
            }
            // flags (?)
            item.flags.push_back(*((uint16_t*)&data[i]));
            i += 2;
            int pending_flags = 0;
            switch (item.flags[0]) {
            case 57: // 00111001b
            case 25: // 00011001b
                pending_flags = 5; break;
            case 49: // 00110001b
            case 17: // 00010001b
                pending_flags = 4; break;
            case 56: // 00111000b
            case 41: // 00101001b
            case 24: // 00011000b
            case 9:  // 00001001b
                pending_flags = 3; break;
            case 48: // 00110000b
            case 33: // 00100001b
            case 16: // 00010000b
            case 1:  // 00000001b
                pending_flags = 2; break;
            case 40: // 00101000b
            case 8:  // 00001000b
                pending_flags = 1; break;
            case 32: // 00100000b
            case 0:  // 00000000b
                pending_flags = 0; break;
            default:
                // 根据上面总结的规律：（不一定正确）
                // 值的形式为: 00xxx00x
                // 其中每位值: 00021002
                // 由此: pending_flags = ((f[0] & 00010000b) > 0) * 2 + （(f[0] & 00001000b) > 0) + （(f[0] & 00000001b) > 0) * 2
                pending_flags = ((item.flags[0] & 0x0010) > 0) * 2 + ((item.flags[0] & 0x0008) > 0) + ((item.flags[0] & 0x0001) > 0) * 2;
                std::stringstream ss;
                ss << "unknown flag: " << item.flags[0] << ", guess flag length is " << pending_flags + 1; // (+1 including [0])
                PrintError(MaxBinaryParserError, LEVEL_WARNING, ss.str());
            }
            for (int j = 0; j < pending_flags && i + 2 <= data.size(); ++j) {
                item.flags.push_back(*((uint16_t*)&data[i]));
                i += 2;
            }
            item.flag_interpreted.Process(item.flags);
            // edges  has 2 * (index_length - 3) items
            for (int j = 0; j + 3 < index_length && i + 8 <= data.size(); ++j) {
                uint32_t v1, v2;
                v1 = *((uint32_t*)&data[i]);
                i += 4;
                v2 = *((uint32_t*)&data[i]);
                i += 4;
                item.edges.push_back({v1, v2});
            }
            ret.push_back(std::move(item));
        }
        if (ret.size() != face_length) {
            PrintError(MaxBinaryParserError, LEVEL_FATAL, "poly face parse failed.");
        }
        return ret;
    }
};

class mPolygonFaceEdgeGroupStrctureValue : public mValue {
public:
    std::string GuessContent() const override {
        auto values = GetValue();
        std::stringstream ss;
        for (const auto& m : values) {
            ss << "[";
            int idx = 0;
            for (const auto& n : m) {
                idx++;
                ss << n;
                if (idx != m.size()) {
                    ss << ", ";
                }
            }
            ss << "], ";
        }
        auto ret = ss.str();
        if (ret.size() >= 2) {
            ret.pop_back();
            ret.pop_back();
        }
        return ret;
    }
    std::vector<std::vector<uint32_t>> GetValue() const {
        std::vector<std::vector<uint32_t>> ret;
        int32_t all_length = data.size() / sizeof(uint32_t);
        const uint32_t* data_ptr = ((uint32_t*)&data[0]);
        int current_index = 0;
        while(current_index < all_length) {
            std::vector<uint32_t> edgelist;
            uint32_t edgelist_length = data_ptr[current_index];
            for (int i = 0; i < edgelist_length; ++i) {
                edgelist.push_back(data_ptr[current_index + 1 + i]);
            }
            current_index += 1 + edgelist_length;
            ret.push_back(edgelist);
        }
        return ret;
    }
};

struct ParamBlockValueHeader {
    explicit ParamBlockValueHeader(const std::string& data) {
        for (int i = 0; i < 15 && i < data.size(); ++i) {
            headers[i].value = data[i];
        }
    }
    int16_t GetIndex() const { return *(int16_t*)(headers); }
    uint32_t GetType() const { return *(uint32_t*)(headers + 2); }
    std::array<uint8_printable, 9> GetFlags() const {
        std::array<uint8_printable, 9> ret;
        std::copy(headers + 6, headers + 15, ret.begin());
        return ret;
    }

    uint8_printable headers[15] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
};

template<typename Type>
inline void PrintArray(std::ostream& ss, const std::vector<Type>& content) {
    ss << "[";
    for (const auto& m : content) {
        ss << m;
        if (&m != &content.back()) {
            ss << ", ";
        }
    }
    ss << "]";
}
template<>
inline void PrintArray(std::ostream& ss, const std::vector<std::string>& content) {
    ss << "[";
    for (const auto& m : content) {
        ss << "\"" << m << "\"";
        if (&m != &content.back()) {
            ss << ", ";
        }
    }
    ss << "]";
}

struct ParamBlockValueContent {
    std::string data;
    int16_t type = 0;

    explicit ParamBlockValueContent(const std::string& data_in) {
        data = data_in;
    }

    template<typename Type>
    std::vector<Type> GetAsType() const {
        mArrayValue<Type> tmp;
        tmp.data = data;
        return tmp.GetValue();
    }

    template<typename StdIterable>
    void AssignToIterable(StdIterable& array_or_list) const {
        using std::begin;
        using std::end;
        auto values = GetAsType<typename StdIterable::value_type>();
        auto it_src = values.begin();
        auto it_src_end = values.end();
        auto it_dest = begin(array_or_list);
        auto it_dest_end = end(array_or_list);
        for (; it_src != it_src_end && it_dest != it_dest_end; ++it_src, ++it_dest) {
            *it_dest = *it_src;
        }
    }
    template<typename Type>
    void AssignToValue(Type& out) const {
        auto values = GetAsType<Type>();
        if (values.size()) {
            out = values.front();
        }
    }

    std::string GetAsString() const {
        mStringWithLengthValue tmp;
        tmp.data = data;
        return tmp.GetValue().second;
    }

    template<typename TrivalType>
    std::vector<TrivalType> GetAsTrivalTypeArray() const {
        static_assert(!std::is_same_v<bool, TrivalType>, "use GetAsBoolTypeArray instead");
        std::vector<TrivalType> ret;
        uint32_t sz = *GetAs<uint32_t>(data, 0);
        ret.reserve(sz);
        int current_pos = sizeof(uint32_t);
        int step = 1 + sizeof(TrivalType);
        assert((data.size() % step) == (current_pos % step));
        while (current_pos < data.size()) {
            uint8_t header = *GetAs<uint8_t>(data, current_pos); // discarded
            ret.push_back(*GetAs<TrivalType>(data, current_pos + 1));
            current_pos += step;
        }
        return ret;
    }

    // its dangerous to use vector<bool>
    std::vector<int32_t> GetAsBoolTypeArray() const {
        std::vector<int32_t> ret;
        uint32_t sz = *GetAs<uint32_t>(data, 0);
        ret.reserve(sz);
        int current_pos = sizeof(uint32_t);
        int step = 1 + sizeof(int32_t);
        assert((data.size() % step) == (current_pos % step));
        while (current_pos < data.size()) {
            uint8_t header = *GetAs<uint8_t>(data, current_pos); // discarded
            ret.push_back(*GetAs<int32_t>(data, current_pos + 1));
            current_pos += step;
        }
        return ret;
    }

    std::vector<std::string> GetAsStringArray() const {
        std::vector<std::string> ret;
        uint32_t sz = *GetAs<uint32_t>(data, 0);
        ret.reserve(sz);
        int current_pos = sizeof(uint32_t);
        while (current_pos < data.size()) {
            uint8_t header = *GetAs<uint8_t>(data, current_pos); // discarded
            uint32_t binary_len = *GetAs<uint32_t>(data, current_pos + 1); // ending zero bytes included
            mUCSStringValue tmp;
            tmp.data = data.substr(current_pos + 5, binary_len);
            ret.push_back(tmp.GetValue());
            current_pos += 5 + binary_len;
        }
        return ret;
    }

    static std::string TypeToString(uint16_t _type) {
        bool is_array = false;
        if (_type & TYPE_TAB) {
            _type &= ~TYPE_TAB;
            is_array = true;
        }
        std::stringstream ss;
        switch (_type) {
        case TYPE_FLOAT:
            ss << "float"; break;
        case TYPE_ANGLE:
            ss << "angle"; break;
        case TYPE_BOOL:
            ss << "bool"; break;
        case TYPE_INT:
            ss << "int32"; break;
        case TYPE_INT64:
            ss << "int64"; break;
        case TYPE_STRING:
            ss << "string"; break;
        case TYPE_RGBA:
            ss << "RGB float"; break;
        case TYPE_POINT3:
            ss << "Point3 float"; break;
        case TYPE_WORLD:
            ss << "world unit"; break;
        case TYPE_PCNT_FRAC:
            ss << "percent"; break;
        case TYPE_TEXMAP:
            ss << "texmap"; break;
        case TYPE_FRGBA:
            ss << "RGBA"; break;
        default:
            ss << "unsupported";  break;
        }
        if (is_array) ss << "[]";
        return ss.str();
    }

    std::string ToString() const {
        if (data.empty()) {
            return "empty";
        }
        std::stringstream ss;
        bool is_array = false;
        ss << TypeToString(type) << ":";
        uint16_t type_mod = type;
        if (type_mod & TYPE_TAB) {
            type_mod &= ~TYPE_TAB;
            is_array = true;
        }
        if (is_array) {
            // array only support string now.
            if (type_mod == TYPE_FLOAT || type_mod == TYPE_ANGLE || type_mod == TYPE_WORLD || type_mod == TYPE_PCNT_FRAC) {
                auto content = GetAsTrivalTypeArray<float>();
                PrintArray(ss, content);
            } else if (type_mod == TYPE_BOOL || type_mod == TYPE_INT || type_mod == TYPE_TEXMAP) {
                auto content = GetAsTrivalTypeArray<int32_t>();
                PrintArray(ss, content);
            } else if (type_mod == TYPE_INT64) {
                auto content = GetAsTrivalTypeArray<int64_t>();
                PrintArray(ss, content);
            } else if (type_mod == TYPE_STRING) {
                auto content = GetAsStringArray();
                PrintArray(ss, content);
            } else {
                ss << "  " << BinaryToHexPrintable(data);
            }
        } else {
            if (type_mod == TYPE_FLOAT || type_mod == TYPE_ANGLE || type_mod == TYPE_WORLD || type_mod == TYPE_PCNT_FRAC || type_mod == TYPE_FRGBA) {
                auto content = GetAsType<float>();
                for (const auto& m : content) {
                    ss << "  " << m;
                }
            } else if (type_mod == TYPE_BOOL || type_mod == TYPE_INT) {
                auto content = GetAsType<int32_t>();
                for (const auto& m : content) {
                    ss << "  " << m;
                }
            } else if (type_mod == TYPE_TEXMAP) {
                auto content = GetAsType<int32_t>();
                for (const auto& m : content) {
                    ss << "  " << m;
                }
            } else if (type_mod == TYPE_INT64) {
                auto content = GetAsType<int64_t>();
                for (const auto& m : content) {
                    ss << "  " << m;
                }
            } else if (type_mod == TYPE_STRING) {
                auto content = GetAsString();
                ss << content;
            } else if (type_mod == TYPE_RGBA || type_mod == TYPE_POINT3) {
                auto content = GetAsType<float>();
                for (int i = 0; i < content.size() / 3; ++i) {
                    ss << "  " << content[i * 3] << ", " << content[i * 3 + 1] << ", " << content[i * 3 + 2];
                }
            } else {
                ss << "  " << BinaryToHexPrintable(data);
            }
        }
        return ss.str();
    }
};

class mParamBlockValue : public mValue {
public:
    std::string GuessContent() const override {
        auto value = GetValue();
        std::stringstream ss;
        ss << "idx: " << value.first.GetIndex() << ", ";
        ss << "type: " << std::setfill('0') << std::setw(4) << std::hex << value.first.GetType();
        ss.clear();
        ss << ", flags: ";
        ss << std::setfill('0') << std::setw(2) << std::hex;
        for (const auto& m : value.first.GetFlags()) {
            ss << m;
        }
        ss.clear();
        ss << ", ";
        ss << value.second.ToString();
        return ss.str();
    }
    std::pair<ParamBlockValueHeader, ParamBlockValueContent> GetValue() const {
        std::pair<ParamBlockValueHeader, ParamBlockValueContent> ret { data , std::string()};
        ret.second.data = data.substr(sizeof(ParamBlockValueHeader));
        ret.second.type = ret.first.GetType();
        return ret;
    }
};



class IValueParseStrategy {
public:
    virtual ~IValueParseStrategy() {}
    virtual std::unique_ptr<mValue> Generate(const std::list<uint16_t>& id_path) = 0;
};

class DefaultValueClassStrategy : public IValueParseStrategy {
public:
    std::unique_ptr<mValue> Generate(const std::list<uint16_t>& id_path) override {
        if (id_path.empty()) {
            return std::make_unique<mValue>();
        }
        // 常见cases
        uint16_t id = id_path.back();
        switch (id) {
        case 0x2039:
        case 0x2037:
        case 0x2042:
        case 0x0962:
            return std::make_unique<mUCSStringValue>();
        case 0x2110:
            return std::make_unique<mClassIDValue>();
        case 0x2060:
            return std::make_unique<mDllClassIDValue>();
            //case 0x0005:
            //case 0x0006:
        case 0x001A:
            return std::make_unique<mStringWithLengthValue>();
        case 0x0001:
            return std::make_unique<mBoolValue>();
        case 0x0003:
            return std::make_unique<mUint32Value>();
        case 0x0004:
            return std::make_unique<mFloatValue>();
        case 0x2034:
        case 0x2035:
            return std::make_unique<mReferenceIndexValue>();
        case 0x0060:
        case 0x0960:
            return std::make_unique<mArrayValue<uint32_t>>();
        case 0x2505:
            return std::make_unique<mArrayValue<float>>();
        case 0x0100:
        case 0x0128:
            return std::make_unique<mArrayValueWithHeader<int32_t, float>>();
        case 0x012b:
        case 0x0310:
        case 0x010a:
            return std::make_unique<mArrayValueWithHeader<int32_t, uint32_t>>();
        default:
            return std::make_unique<mValue>();
        }
    }
};

class ValueClassFactory {
public:
    static ValueClassFactory& Instance() {
        static ValueClassFactory instance;
        return instance;
    }
    static void AddStrategy(const std::shared_ptr<IValueParseStrategy>& strategy) {
        Instance().strategy_list.push_front(strategy);
    }
    static void RemoveStrategy(const std::shared_ptr<IValueParseStrategy>& strategy) {
        auto& strategy_list = Instance().strategy_list;
        auto it = std::find(strategy_list.begin(), strategy_list.end(), strategy);
        if (it != strategy_list.end()) {
            strategy_list.erase(it);
        }
    }
    static std::unique_ptr<mValue> Generate(const std::list<uint16_t>& id_path) {
        std::unique_ptr<mValue> ret;
        for (auto& m : Instance().strategy_list) {
            ret = m->Generate(id_path);
            if (ret) {
                break;
            }
        }
        return ret;
    }
    template<typename mDerivedType>
    static std::unique_ptr<mValue> Generate() {
        std::unique_ptr<mValue> ret = std::make_unique<mDerivedType>();
        return ret;
    }

    std::list<std::shared_ptr<IValueParseStrategy>> strategy_list;
};

// deprecated it's slow and not recommanded to use
inline bool HasSubPath(const std::list<uint16_t>& id_path, const std::list<uint16_t>& sub_path) {
    if (id_path.size() < sub_path.size() || sub_path.empty()) {
        return false;
    }
    auto start = sub_path.front();
    auto it_current = id_path.begin();
    while (it_current != id_path.end()) {
        auto it_temp = std::find(it_current, id_path.end(), start);
        if (it_temp != id_path.end()) {
            it_current = it_temp;
            auto it_target = sub_path.begin();
            while (*it_temp == *it_target) {
                ++it_target; ++it_temp;
                if (it_target == sub_path.end()) {
                    return true;
                }
                if (it_temp == id_path.end()) {
                    return false;
                }
            }
        }
        ++it_current; 
    }
    return false;
}

inline bool HasStartPath(const std::list<uint16_t>& id_path, const std::list<uint16_t>& sub_path, int offset = 2) {
    if (id_path.size() < offset + sub_path.size() || sub_path.empty()) {
        return false;
    }
    auto it_current = id_path.begin();
    for (int i = 0; i < offset; ++i) {
        it_current++;
    }
    auto it_temp = sub_path.begin();
    while (it_current != id_path.end()) {
        if (*it_current != *it_temp) {
            return false;
        }
        ++it_current; ++it_temp;
        if (it_temp == sub_path.end()) {
            return true;
        }
    }
    return false;
}

//inline const mNode* TakeNodeByPath(const std::list<uint16_t>& id_path, const mNode* node) {
//    auto it = id_path.begin();
//    while (it != id_path.end() && node) {
//        if (node->type != mNode::Container) {
//            return nullptr;
//        }
//        node = ((const mContainer*)node)->GetFirstContentByID(*it);
//    }
//    return node;
//}
//
//inline const mNode* TakeNodeByPath(const std::list<std::pair<uint16_t, uint16_t>>& id_index_path, const mNode* node) {
//    auto it = id_index_path.begin();
//    while (it != id_index_path.end() && node) {
//        if (node->type != mNode::Container) {
//            return nullptr;
//        }
//        const mNode* target = nullptr;
//        for (auto idx = 0; idx < it->second; ++idx) {
//            target = ((const mContainer*)node)->GetFirstContentByID(it->first, target);
//            if (target == nullptr) {
//                break;
//            }
//        }
//        node = target;
//    }
//    return node;
//}

class MaxValueClassStrategyBase : public IValueParseStrategy {
public:
    std::unique_ptr<mValue> Generate(const std::list<uint16_t>& id_path) override {
        if (id_path.size() <= 1) {
            return nullptr;
        }
        uint16_t class_node_id = *(++id_path.begin());
        auto class_info = Linker::GetClassEntryByIndex(class_node_id);
        return GenerateByClass(id_path, class_node_id, class_info);
    }

    virtual std::unique_ptr<mValue> GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) = 0;
};

class BezierFloatValueClassStrategy : public MaxValueClassStrategyBase {
public:
    std::unique_ptr<mValue> GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) override;
};

class EditableMeshValueClassStrategy : public MaxValueClassStrategyBase {
public:
    std::unique_ptr<mValue> GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) override;
};

class ModifierListEditNormalsValueClassStrategy : public MaxValueClassStrategyBase {
public:
    std::unique_ptr<mValue> GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) override {
        if (class_id == 0x2032) {
            if (HasStartPath(id_path, { 0x2500, 0x2512, 0x0250, 0x0110 })) {
                // edit normal 相关
                return std::make_unique<mArrayValueWithHeader<int32_t, float>>();
            } else if (HasStartPath(id_path, { 0x2500, 0x2512, 0x0250, 0x0120 })) {
                // 后续索引个数
                return std::make_unique<mUint32Value>();
            } else if (HasStartPath(id_path, { 0x2500, 0x2512, 0x0250, 0x0124 })) {
                // 当前面片索引
                return std::make_unique<mUint32Value>();
            } else if (HasStartPath(id_path, { 0x2500, 0x2512, 0x0250, 0x0128 })) {
                // 当前面片三个点索引
                return std::make_unique<mArrayValue<uint32_t>>();
            } else if (HasStartPath(id_path, { 0x2500, 0x2512, 0x0110 })) { 
                // vertex paint 相关
                // 9 float / 面
                return std::make_unique<mArrayValue<float>>();
            } else if (HasStartPath(id_path, { 0x2500, 0x2512, 0x0120 })) {
                // 面片三点颜色
                // 3 float / 面
                return std::make_unique<mArrayValue<float>>();
            }
        }
        return nullptr;
    }
};

class EditablePolyValueClassStrategy : public MaxValueClassStrategyBase {
public:
    std::unique_ptr<mValue> GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) override;
};

class NodeValueClassStrategy : public MaxValueClassStrategyBase {
public:
    std::unique_ptr<mValue> GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) override;
};

class StandardAndColorMapClassStrategy : public MaxValueClassStrategyBase {
public:
    std::unique_ptr<mValue> GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) override;
};

class ParamBlock2ClassStrategy : public MaxValueClassStrategyBase {
public:
    std::unique_ptr<mValue> GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) override;
};

class BaseLayerClassStrategy : public MaxValueClassStrategyBase {
public:
    std::unique_ptr<mValue> GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) override;
};

inline void DefaultInitStrategy() {
    static std::once_flag once;
    std::call_once(once, []() {
        ValueClassFactory::Instance().AddStrategy(std::make_shared<DefaultValueClassStrategy>());
    });
}

inline void InitMaxClassStrategy() {
    ValueClassFactory::Instance().AddStrategy(std::make_shared<BezierFloatValueClassStrategy>());
    ValueClassFactory::Instance().AddStrategy(std::make_shared<EditableMeshValueClassStrategy>());
    ValueClassFactory::Instance().AddStrategy(std::make_shared<ModifierListEditNormalsValueClassStrategy>());
    ValueClassFactory::Instance().AddStrategy(std::make_shared<EditablePolyValueClassStrategy>());
    ValueClassFactory::Instance().AddStrategy(std::make_shared<NodeValueClassStrategy>());
    ValueClassFactory::Instance().AddStrategy(std::make_shared<StandardAndColorMapClassStrategy>());
    ValueClassFactory::Instance().AddStrategy(std::make_shared<ParamBlock2ClassStrategy>());
    ValueClassFactory::Instance().AddStrategy(std::make_shared<BaseLayerClassStrategy>());
}
} // max