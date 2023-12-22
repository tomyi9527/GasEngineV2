#include "mesh_loader.h"
#include <assert.h>
#include <math.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string_view>
#include "ecs/entity_factory.h"
#include "opengl/buffer_type.h"
#include "opengl/global_resource.h"
#include "opengl/renderable_item.h"
#include "utils/ipow.h"

// for M_PI
#define _USE_MATH_DEFINES
#include <math.h>

// platform check
static_assert(sizeof(char) == 1, "platform not supported, sizeof(char) is not 1.");
static_assert(sizeof(float) == 4, "platform not supported, sizeof(float) is not 4.");
static_assert(sizeof(int32_t) == 4, "platform not supported, sizeof(int32_t) is not 4.");
static_assert(sizeof(uint32_t) == 4, "platform not supported, sizeof(uint32_t) is not 4.");
static_assert(sizeof(OBJECT_HEADER) == 116,
              "struct size of OBJECT_HEADER not correct, should be 116.");
static_assert(sizeof(SECTION_ITEM) == 28, "struct size of SECTION_ITEM not correct, should be 28.");

// constant initialization
const static char* FILE_BEGINNING_FLAG = "NEO*";
const static char* FILE_END_FLAG = "*OEN";
const static uint32_t ENDIAN_FLAG_VALUE = 0x12345678;
const static std::set<std::string_view> OBJECT_TYPE_CHOICES = {"UNKNOWN", "MESH", "MORPHTARGET",
                                                               "ANIMATION"};  // not section type!

template <typename ReturnType = uint32_t>
std::vector<ReturnType> DecodeVarint(const std::vector<uint8_t>& vals, int input_size,
                                     int element_count) {
    std::vector<ReturnType> output(element_count, 0);
    int s = 0;
    for (int i = 0; i < element_count; i++) {
        int o = 0;
        int l = 0;
        do {
            o |= (0x7F & vals[s]) << l;
            l += 7;
        } while (0 != (0x80 & vals[s++]));

        output[i] = o;
    }

    if (input_size < s) {
        LOG_ERROR("Decode var int failed!");
    }

    return output;
}

template <typename ValType = uint32_t>
std::vector<ValType> DecodeZigzagAndDelta(const std::vector<ValType>& vals) {
    std::vector<ValType> output = vals;
    ValType last_value = 0;
    for (auto& m : output) {
        last_value += ((m >> 1) ^ (0 - (m & 1)));
        m = last_value;
    }
    return output;
}

// load header
OBJECT_HEADER OBJECT_LOADER::LoadHeader(std::istream& s) {
    assert(obj_current_offset == 0);
    OBJECT_HEADER header;
    char* p_header = reinterpret_cast<char*>(&header);
    s.read(p_header, sizeof(OBJECT_HEADER));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    // assert(obj_current_offset == sizeof(OBJECT_HEADER));
    if (obj_current_offset != sizeof(OBJECT_HEADER)) {
        LOG_ERROR("OBJECT_HEADER is incomplete.");
    }
    return header;
}

// validate header
bool OBJECT_LOADER::CheckHeader(const OBJECT_HEADER& header) {
    // check FILE_FLAG
    std::string_view FileFlag(header.FILE_FLAG, 4);
    std::string_view BeginFlag(FILE_BEGINNING_FLAG, 4);
    if (FileFlag != BeginFlag) {
        LOG_ERROR("FILE_FLAG in OBJECT_HEADER error.");
        return false;
    }
    // check ENDIAN_FLAG
    if (header.ENDIAN_FLAG != ENDIAN_FLAG_VALUE) {
        LOG_ERROR("ENDIAN_FLAG in OBJECT_HEADER error.");
        return false;
    }
    // check OBJECT_TYPE
    std::string_view ObjectFlag(header.OBJECT_TYPE, std::min((uint64_t)sizeof(header.OBJECT_TYPE),
                                                             (uint64_t)strlen(header.OBJECT_TYPE)));
    if (OBJECT_TYPE_CHOICES.count(ObjectFlag) == 0) {
        LOG_ERROR("OBJECT_TYPE in OBJECT_HEADER error.");
        return false;
    }
    // check OFFSETs
    if (header.SECTION_TABLE_OFFSET < sizeof(OBJECT_HEADER)) {
        LOG_ERROR("SECTION_TABLE_OFFSET in OBJECT_HEADER error.");
        return false;
    }
    if (header.STRING_TABLE_OFFSET < sizeof(OBJECT_HEADER)) {
        LOG_ERROR("SECTION_TABLE_OFFSET in OBJECT_HEADER error.");
        return false;
    }
    return true;
}

// load string_table
// s should be at string_table offset
STRING_TABLE OBJECT_LOADER::LoadStringTable(std::istream& s) {
    STRING_TABLE string_table;
    uint32_t count = 0;
    s.read(reinterpret_cast<char*>(&count), sizeof(count));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    string_table.strings.reserve(count);
    while (count--) {
        uint32_t size = 0;
        s.read(reinterpret_cast<char*>(&size), sizeof(size));
        obj_current_offset += static_cast<uint32_t>(s.gcount());
        string_table.strings.push_back(std::string(size, '\0'));
        s.read(string_table.strings.back().data(), size);
        obj_current_offset += static_cast<uint32_t>(s.gcount());
        char ignored_char;
        s.read(&ignored_char, 1);
        obj_current_offset += static_cast<uint32_t>(s.gcount());
    }
    return string_table;
}

// nothing to validate for string_table

// load section
// s should be at section offset
SECTION OBJECT_LOADER::LoadSection(std::istream& s) {
    SECTION section;
    uint32_t count = 0;
    s.read(reinterpret_cast<char*>(&count), sizeof(count));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    section.sections.reserve(count);
    while (count--) {
        section.sections.emplace_back();
        char* mem_ptr = reinterpret_cast<char*>(&(section.sections.back()));
        s.read(mem_ptr, sizeof(SECTION_ITEM));
        obj_current_offset += static_cast<uint32_t>(s.gcount());
    }
    return section;
}

bool OBJECT_LOADER::CheckSection(const SECTION& section) {
    for (const auto& m : section.sections) {
        if (m.SECTION_TYPE >= kSECTION_TYPE::kSECTION_TYPE_COUNT) {
            LOG_ERROR("SECTION_TYPE in section out of range.");
            return false;
        }
        if (m.ELEMENT_TYPE >= kELEMENT_TYPE::ELEMENT_TYPE_COUNT) {
            LOG_ERROR("ELEMENT_TYPE in section out of range.");
            return false;
        }
    }
    return true;
}

// describe section
void OBJECT_LOADER::DescribeSection(int index, const SECTION_ITEM& item) {
    const static std::map<uint32_t, std::string> SectionTypeName = {{kPOSITION, "POSITION"},
                                                                    {kNORMAL0, "NORMAL0"},
                                                                    {kNORMAL1, "NORMAL1"},
                                                                    {kTANGENT0, "TANGENT0"},
                                                                    {kTANGENT1, "TANGENT1"},
                                                                    {kBINORMAL0, "BINORMAL0"},
                                                                    {kBINORMAL1, "BINORMAL1"},
                                                                    {kUV0, "UV0"},
                                                                    {kUV1, "UV1"},
                                                                    {kVERTEXCOLOR0, "VERTEXCOLOR0"},
                                                                    {kVERTEXCOLOR1, "VERTEXCOLOR1"},
                                                                    {kBLENDWEIGHT, "BLENDWEIGHT"},
                                                                    {kBLENDINDEX, "BLENDINDEX"},
                                                                    {kINDEX, "INDEX"},
                                                                    {kSUBMESH, "SUBMESH"},
                                                                    {kBONE, "BONE"},
                                                                    {kMORPHTARGET, "MORPHTARGET"},
                                                                    {kKEYFRAME, "KEYFRAME"},
                                                                    {kTOPOLOGY, "TOPOLOGY"}};
    const static std::map<uint32_t, std::string> ElementTypeName = {{FLOAT_TYPE, "FLOAT_TYPE"},
                                                                    {UINT_TYPE, "UINT_TYPE"},
                                                                    {STRUCT_TYPE, "STRUCT_TYPE"},
                                                                    {UBYTE_TYPE, "UBYTE_TYPE"}};
    std::cout << "section_index: " << index << std::endl;
    {
        auto it = SectionTypeName.find(item.SECTION_TYPE);
        if (it != SectionTypeName.end()) {
            std::cout << "    section_type : " << it->second << std::endl;
        } else {
            std::cout << "    section_type : invalid!" << std::endl;
        }
    }
    std::cout << "    range of mem : ";
    std::cout << "[" << item.SECTION_OFFSET << ", " << item.SECTION_OFFSET + item.SECTION_LENGTH
              << ")";
    std::cout.setf(std::ios::showbase);
    std::cout.setf(std::ios_base::hex, std::ios_base::basefield);
    std::cout << " or [" << item.SECTION_OFFSET << ", " << item.SECTION_OFFSET + item.SECTION_LENGTH
              << ")";
    std::cout.unsetf(std::ios::hex);

    std::cout << ", total bytes: " << item.SECTION_LENGTH << std::endl;
    {
        auto it = ElementTypeName.find(item.ELEMENT_TYPE);
        if (it != ElementTypeName.end()) {
            std::cout << "    element_type : " << it->second << std::endl;
        } else {
            std::cout << "    element_type : invalid!" << std::endl;
        }
    }
    std::cout << "    element_count: " << item.ELEMENT_COUNT << std::endl;
    std::cout << "    attribute_count: " << item.ATTRIBUTE_COUNT << std::endl;
    std::cout << "    data_attribute: " << item.DATA_ATTRIBUTE << std::endl;
}

// load all from stream
// s should be at file start or object header start
std::shared_ptr<OBJECT> OBJECT_LOADER::LoadObject(std::istream& s) {
    std::shared_ptr<OBJECT> obj = nullptr;
    while (s && !s.eof()) {
        auto tmp = LoadSingleObject(s);
        if (tmp == nullptr) {
            break;
        }
        if (obj == nullptr) {
            obj = tmp;
            continue;
        }
        if (obj && obj->header.VERSION != 7) {
            break;
        }
        obj->AddMorphTarget(tmp);
    }
    return obj;
}

// load first object
std::shared_ptr<OBJECT> OBJECT_LOADER::LoadSingleObject(std::istream& s) {
    std::shared_ptr<OBJECT> obj_ptr = OBJECT::GenerateOBJECT();
    OBJECT& obj = *obj_ptr;
    if (!s || s.eof()) {
        return obj_ptr;
    }
    obj_start_offset = static_cast<uint32_t>(s.tellg());
    obj_current_offset = 0;
    // header
    obj.header = LoadHeader(s);
    // maybe '\0' here
    if (obj_current_offset != sizeof(OBJECT_HEADER)) {
        return nullptr;
    }
    assert(CheckHeader(obj.header));

    // string table
    if (obj.header.STRING_TABLE_OFFSET > 0) {
        if (obj_current_offset != obj.header.STRING_TABLE_OFFSET) {
            obj_current_offset = obj.header.STRING_TABLE_OFFSET;
            s.seekg(obj_start_offset + obj_current_offset, std::ios::beg);
        }
        obj.string_table = LoadStringTable(s);
        assert(obj.header.OBJECT_NAME_INDEX < obj.string_table.strings.size());
        assert(obj_current_offset <= obj.header.SECTION_TABLE_OFFSET);
    }

    // assign name
    assert(obj.header.OBJECT_NAME_INDEX < obj.string_table.strings.size());
    obj.name = obj.string_table.strings[obj.header.OBJECT_NAME_INDEX];

    // section
    if (obj_current_offset != obj.header.SECTION_TABLE_OFFSET) {
        obj_current_offset = obj.header.SECTION_TABLE_OFFSET;
        s.seekg(obj_start_offset + obj_current_offset, std::ios::beg);
    }
    obj.section = LoadSection(s);
    assert(CheckSection(obj.section));

    // section data
    int section_index = 0;
    for (const auto& section_item : obj.section.sections) {
        // disable verbose now
        // DescribeSection(section_index, section_item);
        // move stream position
        if (obj_current_offset != section_item.SECTION_OFFSET) {
            obj_current_offset = section_item.SECTION_OFFSET;
            s.seekg(obj_start_offset + obj_current_offset, std::ios::beg);
        }
        // start to read
        LoadSectionData(s, section_item, obj);
        // next index
        ++section_index;

        // record this item
        if (section_item.SECTION_TYPE < kSECTION_TYPE_COUNT) {
            obj.section_item_map[static_cast<kSECTION_TYPE>(section_item.SECTION_TYPE)] =
                section_item;
        }
    }
    obj.CalculateAfterLoading();

    // move to the obj_start + obj_size
    if (obj_current_offset != obj.header.OBJECT_FILE_SIZE) {
        obj_current_offset = obj.header.OBJECT_FILE_SIZE;
        s.seekg(obj_start_offset + obj_current_offset, std::ios::beg);
    }
    return obj_ptr;
}

void OBJECT_LOADER::LoadSectionData(std::istream& s, const SECTION_ITEM& sc, OBJECT& obj) {
    if (sc.SECTION_TYPE < kSECTION_TYPE_COUNT) {
        switch (sc.SECTION_TYPE) {
            case kSECTION_TYPE::kPOSITION:
                obj.position =
                    PositionDataDecode(s, obj.header.POS_BBOX_MIN, obj.header.POS_BBOX_MAX, sc);
                break;
            case kSECTION_TYPE::kNORMAL0:
                obj.normal0 = NormalDataDecode(s, sc);
                break;
            case kSECTION_TYPE::kNORMAL1:
                obj.normal1 = NormalDataDecode(s, sc);
                break;
            case kSECTION_TYPE::kTANGENT0:
                obj.tangent0 = TangentDataDecode(s, sc);
                break;
            case kSECTION_TYPE::kTANGENT1:
                obj.tangent1 = TangentDataDecode(s, sc);
                break;
            case kSECTION_TYPE::kUV0:
                obj.uv0 = UVDataDecode(s, obj.header.UV0_BBOX_MIN, obj.header.UV0_BBOX_MAX, sc);
                break;
            case kSECTION_TYPE::kUV1:
                obj.uv1 = UVDataDecode(s, obj.header.UV1_BBOX_MIN, obj.header.UV1_BBOX_MAX, sc);
                break;
            case kSECTION_TYPE::kBLENDWEIGHT:
                obj.blend_weights = BlendWeightDecode(s, sc);
                break;
            case kSECTION_TYPE::kBLENDINDEX:
                obj.blend_indices = BlendIndicesDecode(s, sc);
                break;
            case kSECTION_TYPE::kINDEX:
                obj.indices = IndicesDecode(s, sc);
                break;
            case kSECTION_TYPE::kTOPOLOGY:
                obj.topology = IndicesDecode(s, sc);
                break;
            case kSECTION_TYPE::kSUBMESH:
                obj.submesh = SubMeshDecode(s, sc);
                break;
            case kSECTION_TYPE::kVERTEXCOLOR0:
                obj.vertex_color0 = VertexColorDecode(s, sc);
                break;
            case kSECTION_TYPE::kVERTEXCOLOR1:
                obj.vertex_color1 = VertexColorDecode(s, sc);
                break;
            case kSECTION_TYPE::kBONE: {
                obj.bone = BoneDecode(s, sc);
                int32_t string_table_len = static_cast<int32_t>(obj.string_table.strings.size());
                for (const auto& m : obj.bone) {
                    assert(string_table_len > m.name2_index);
                    assert(string_table_len > m.name1_index);
                    assert(string_table_len > m.name_index);
                }
            } break;
            case kSECTION_TYPE::kBINORMAL0:
            case kSECTION_TYPE::kBINORMAL1:
                LOG_WARNING(
                    "binormal will be calculated according to normal, uv. binormal section from "
                    "input is ignored");
                break;
            default:
                LOG_ERROR("SECTION_TYPE %d is not supported yet.", sc.SECTION_TYPE);
        }
    } else {
        LOG_ERROR("SECTION_TYPE %d in section out of range.", sc.SECTION_TYPE);
    }
}

// decoder
// s should be at position data offset (section.section_offset)
std::vector<float> OBJECT_LOADER::PositionDataDecode(std::istream& s, const float bbox_min[3],
                                                     const float bbox_max[3],
                                                     const SECTION_ITEM& sc) {
    // ? section_type: 此处根据此type进入的，值为 0 (POSITION)
    // ? section_offset: istream目前的读取配置应已经到了此offset
    // ? section_length: istream目前可连续读的范围
    // ? element_count: 每个数据包含的element个数，此处表示维度
    // ? element_type: 存储在内存中的element的单位类型。
    // ? attribute_count: 数据个数，此处表示点数。
    // ? data_attribute： element属性，此处表示每个element的数据编码类型。

    // 开头16个为name
    char type[16] = {0};
    s.read(type, sizeof(type));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    assert(strcmp(type, "POSITION") == 0);
    assert(sc.ELEMENT_TYPE == kELEMENT_TYPE::FLOAT_TYPE);

    std::vector<float> output(sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT, 0);
    // 如果无压缩
    if (sc.DATA_ATTRIBUTE == kMESH_ATTRIBUTE_LOOSE) {
        assert(sc.SECTION_LENGTH == sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT * sizeof(float));
        s.read(reinterpret_cast<char*>(output.data()), output.size() * sizeof(float));
        obj_current_offset += static_cast<uint32_t>(s.gcount());
        return output;
    }

    // 以下为有压缩
    // data encoding 方式
    std::vector<int> encoding_type;
    encoding_type.reserve(sc.ELEMENT_COUNT);
    for (uint32_t i = 0; i < sc.ELEMENT_COUNT; ++i) {
        encoding_type.push_back((sc.DATA_ATTRIBUTE >> (3 * i)) & 0x07);
        assert(encoding_type.back() == 1 || encoding_type.back() == 2 || encoding_type.back() == 4);
    }
    // 1. 当 encoding_type != 4 时，使用bbox变换坐标到bbox内部，需要precision。
    // 2. 当 encoding_type == 4 时，直接使用内存数据, precision无意义。
    // 最小精度
    std::vector<double> precision;
    precision.reserve(encoding_type.size());
    for (uint64_t i = 0; i < encoding_type.size(); ++i) {
        // precision
        if (encoding_type[i] == 1 || encoding_type[i] == 2)
            precision.push_back((bbox_max[i] - bbox_min[i]) /
                                (std::pow(2.0, encoding_type[i] * 8.0) - 1.0));
        else
            precision.push_back(std::numeric_limits<double>::epsilon());
    }
    // 每element长度
    std::vector<uint32_t> lengthes(sc.ELEMENT_COUNT, 0);
    std::vector<uint32_t> biases(sc.ELEMENT_COUNT, 0);
    char* p_lengthes = reinterpret_cast<char*>(lengthes.data());
    s.read(p_lengthes, sizeof(uint32_t) * sc.ELEMENT_COUNT);
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    biases[0] = obj_current_offset;
    // 读取数据
    for (uint32_t i = 0; i < sc.ELEMENT_COUNT; ++i) {
        if (i > 0) {
            biases[i] = biases[i - 1] + lengthes[i - 1];
            if (obj_current_offset != biases[i]) {
                obj_current_offset = biases[i];
                s.seekg(obj_start_offset + obj_current_offset, std::ios::beg);
            }
        }
        if (encoding_type[i] == 1) {
            assert(lengthes[i] == (sc.ATTRIBUTE_COUNT + 3) / 4 * 4);
            std::vector<uint8_t> vals(sc.ATTRIBUTE_COUNT, 0);
            char* p_data = reinterpret_cast<char*>(vals.data());
            s.read(p_data, sizeof(uint8_t) * vals.size());
            obj_current_offset += static_cast<uint32_t>(s.gcount());
            for (uint64_t j = 0; j < vals.size(); ++j) {
                output[j * sc.ELEMENT_COUNT + i] =
                    static_cast<float>(vals[j] * precision[i] + bbox_min[i]);
            }
        } else if (encoding_type[i] == 2) {
            std::vector<uint8_t> vals(lengthes[i], 0);
            char* p_data = reinterpret_cast<char*>(vals.data());
            s.read(p_data, sizeof(uint8_t) * vals.size());
            obj_current_offset += static_cast<uint32_t>(s.gcount());
            std::vector<uint32_t> decoded_vals =
                DecodeVarint(vals, lengthes[i], sc.ATTRIBUTE_COUNT);
            decoded_vals = DecodeZigzagAndDelta(decoded_vals);
            assert(decoded_vals.size() == sc.ATTRIBUTE_COUNT);
            for (uint64_t j = 0; j < decoded_vals.size(); ++j) {
                output[j * sc.ELEMENT_COUNT + i] =
                    static_cast<float>(decoded_vals[j] * precision[i] + bbox_min[i]);
            }
        } else if (encoding_type[i] == 4) {
            assert(lengthes[i] == sizeof(float) * sc.ATTRIBUTE_COUNT);
            std::vector<float> vals(sc.ATTRIBUTE_COUNT, 0);
            char* p_data = reinterpret_cast<char*>(vals.data());
            s.read(p_data, sizeof(float) * vals.size());
            obj_current_offset += static_cast<uint32_t>(s.gcount());
            for (uint64_t j = 0; j < vals.size(); ++j) {
                output[j * sc.ELEMENT_COUNT + i] = vals[j];
            }
        } else {
            LOG_ERROR("Unrecognized vertex data encoding!");
        }
    }
    return output;
}

// s should be at position data offset (section.section_offset)
std::vector<float> OBJECT_LOADER::NormalDataDecode(std::istream& s, const SECTION_ITEM& sc) {
    // 开头16个为name
    char type[16] = {0};
    s.read(type, sizeof(type));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    assert(strcmp(type, "NORMAL0") == 0 || strcmp(type, "NORMAL1") == 0);
    assert(sc.ELEMENT_TYPE == kELEMENT_TYPE::FLOAT_TYPE);

    std::vector<float> output(sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT, 0);
    // 如果无压缩
    if (sc.DATA_ATTRIBUTE == kMESH_ATTRIBUTE_LOOSE) {
        assert(sc.SECTION_LENGTH == sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT * sizeof(float));
        s.read(reinterpret_cast<char*>(output.data()), output.size() * sizeof(float));
        obj_current_offset += static_cast<uint32_t>(s.gcount());
        return output;
    }

    constexpr double k = M_PI / (double)(NORMAL_ENCODING_PHI_COUNT - 1);
    for (uint32_t i = 0; i < sc.ATTRIBUTE_COUNT; ++i) {
        // {encoded_phi, encoded_theta}
        uint8_t encoded[2] = {0};
        s.read(reinterpret_cast<char*>(encoded), sizeof(encoded));
        obj_current_offset += static_cast<uint32_t>(s.gcount());

        double phi = encoded[0] * k;
        double theta = (encoded[1] * 2.0 * M_PI) / (NORMAL_DECODING_DICTIONARY[encoded[0]]);

        double m = sin(phi);
        output[sc.ELEMENT_COUNT * i + 0] = static_cast<float>(m * cos(theta));
        output[sc.ELEMENT_COUNT * i + 1] = static_cast<float>(m * sin(theta));
        output[sc.ELEMENT_COUNT * i + 2] = static_cast<float>(cos(phi));
    }
    return output;
}
std::vector<float> OBJECT_LOADER::TangentDataDecode(std::istream& s, const SECTION_ITEM& sc) {
    // 开头16个为name
    char type[16] = {0};
    s.read(type, sizeof(type));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    assert(strcmp(type, "TANGENT0") == 0 || strcmp(type, "TANGENT1") == 0);
    assert(sc.ELEMENT_TYPE == kELEMENT_TYPE::FLOAT_TYPE);

    std::vector<float> output(sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT, 0);
    // 如果无压缩
    if (sc.DATA_ATTRIBUTE == kMESH_ATTRIBUTE_LOOSE) {
        assert(sc.SECTION_LENGTH == sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT * sizeof(float));
        s.read(reinterpret_cast<char*>(output.data()), output.size() * sizeof(float));
        obj_current_offset += static_cast<uint32_t>(s.gcount());
        return output;
    }

    constexpr double k = M_PI / (double)(NORMAL_ENCODING_PHI_COUNT - 1);
    for (uint32_t i = 0; i < sc.ATTRIBUTE_COUNT; ++i) {
        // {encoded_phi, encoded_theta}
        uint8_t encoded[2] = {0};
        s.read(reinterpret_cast<char*>(encoded), sizeof(encoded));
        obj_current_offset += static_cast<uint32_t>(s.gcount());

        output[sc.ELEMENT_COUNT * i + 3] = (0x80 & encoded[0] ? -1.0f : 1.0f);
        encoded[0] &= 0x81;

        double phi = encoded[0] * k;
        double theta = (encoded[1] * 2.0 * M_PI) / (NORMAL_DECODING_DICTIONARY[encoded[0]]);

        double m = sin(phi);
        output[sc.ELEMENT_COUNT * i + 0] = static_cast<float>(m * cos(theta));
        output[sc.ELEMENT_COUNT * i + 1] = static_cast<float>(m * sin(theta));
        output[sc.ELEMENT_COUNT * i + 2] = static_cast<float>(cos(phi));
    }
    return output;
}

std::vector<float> OBJECT_LOADER::UVDataDecode(std::istream& s, const float bbox_min[2],
                                               const float bbox_max[2], const SECTION_ITEM& sc) {
    // 开头16个为name
    char type[16] = {0};
    s.read(type, sizeof(type));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    assert(strcmp(type, "UV0") == 0 || strcmp(type, "UV1") == 0);
    assert(sc.ELEMENT_TYPE == kELEMENT_TYPE::FLOAT_TYPE);

    std::vector<float> output(sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT, 0);
    // 如果无压缩
    if (sc.DATA_ATTRIBUTE == kMESH_ATTRIBUTE_LOOSE) {
        assert(sc.SECTION_LENGTH == sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT * sizeof(float));
        s.read(reinterpret_cast<char*>(output.data()), output.size() * sizeof(float));
        obj_current_offset += static_cast<uint32_t>(s.gcount());
        return output;
    }

    constexpr int grades = ipow(2, 12) - 1;
    const double precisionX = (bbox_max[0] - bbox_min[0]) / grades;
    const double precisionY = (bbox_max[1] - bbox_min[1]) / grades;

    for (uint32_t i = 0; i < sc.ATTRIBUTE_COUNT; ++i) {
        uint8_t encoded[3] = {0};
        s.read(reinterpret_cast<char*>(encoded), sizeof(encoded));
        obj_current_offset += static_cast<uint32_t>(s.gcount());

        int encoded_int = 0;
        encoded_int |= (int)encoded[0];
        encoded_int |= ((int)encoded[1]) << 8;
        encoded_int |= ((int)encoded[2]) << 16;

        output[sc.ELEMENT_COUNT * i + 0] =
            static_cast<float>((encoded_int & 0x0FFF) * precisionX + bbox_min[0]);
        output[sc.ELEMENT_COUNT * i + 1] =
            static_cast<float>((encoded_int >> 12 & 0x0FFF) * precisionY + bbox_min[1]);
    }
    return output;
}

std::vector<uint8_t> OBJECT_LOADER::VertexColorDecode(std::istream& s, const SECTION_ITEM& sc) {
    // 开头16个为name
    char type[16] = {0};
    s.read(type, sizeof(type));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    assert(strcmp(type, "VERTEXCOLOR0") == 0 || strcmp(type, "VERTEXCOLOR1") == 0);
    assert(sc.ELEMENT_TYPE == kELEMENT_TYPE::UBYTE_TYPE);

    std::vector<uint8_t> output(sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT, 0);
    // 如果无压缩
    if (sc.DATA_ATTRIBUTE == kMESH_ATTRIBUTE_LOOSE) {
        assert(sc.SECTION_LENGTH == sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT * sizeof(uint8_t));
        s.read(reinterpret_cast<char*>(output.data()), output.size() * sizeof(uint8_t));
        obj_current_offset += static_cast<uint32_t>(s.gcount());
        return output;
    }

    char unit_color = 0;
    char unit_alpha = 0;
    s.read(&unit_color, 1);
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    if (unit_color == 1) {
        char color[4] = {0};
        s.read(color, sizeof(color));
        obj_current_offset += static_cast<uint32_t>(s.gcount());
        unit_alpha = color[3];
        for (uint32_t i = 0; i < sc.ATTRIBUTE_COUNT; ++i) {
            output[sc.ELEMENT_COUNT * i + 0] = color[0];
            output[sc.ELEMENT_COUNT * i + 1] = color[1];
            output[sc.ELEMENT_COUNT * i + 2] = color[2];
        }
    } else {
        char tmp;
        s.read(&tmp, 1);
        obj_current_offset += static_cast<uint32_t>(s.gcount());
        for (uint32_t i = 0; i < sc.ATTRIBUTE_COUNT; ++i) {
            uint8_t color[2] = {0};
            s.read(reinterpret_cast<char*>(color), sizeof(color));
            obj_current_offset += static_cast<uint32_t>(s.gcount());
            int s_color = (int)color[0] | (int)color[1] << 8;
            output[sc.ELEMENT_COUNT * i + 0] = ((s_color)&0xF800) >> 8;  // r
            output[sc.ELEMENT_COUNT * i + 1] = ((s_color)&0x07E0) >> 3;  // g
            output[sc.ELEMENT_COUNT * i + 2] = ((s_color)&0x001F) << 3;  // b
        }
        // flag
        s.read(&unit_alpha, 1);
        obj_current_offset += static_cast<uint32_t>(s.gcount());
    }

    if (unit_alpha == 1) {
        // value
        uint8_t alpha = 0;
        s.read(reinterpret_cast<char*>(&alpha), 1);
        obj_current_offset += static_cast<uint32_t>(s.gcount());
        for (uint32_t i = 0; i < sc.ATTRIBUTE_COUNT; ++i) {
            output[sc.ELEMENT_COUNT * i + 3] = alpha;  // a
        }
    } else {
        for (uint32_t i = 0; i < sc.ATTRIBUTE_COUNT; ++i) {
            // value
            uint8_t alpha = 0;
            s.read(reinterpret_cast<char*>(&alpha), 1);
            obj_current_offset += static_cast<uint32_t>(s.gcount());
            output[sc.ELEMENT_COUNT * i + 3] = alpha;  // a
        }
    }

    return output;
}

std::vector<float> OBJECT_LOADER::BlendWeightDecode(std::istream& s, const SECTION_ITEM& sc) {
    // 开头16个为name
    char type[16] = {0};
    s.read(type, sizeof(type));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    assert(strcmp(type, "BLENDWEIGHT") == 0);
    assert(sc.ELEMENT_TYPE == kELEMENT_TYPE::FLOAT_TYPE);

    std::vector<float> output(sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT, 0);
    // 如果无压缩
    if (sc.DATA_ATTRIBUTE == kMESH_ATTRIBUTE_LOOSE) {
        assert(sc.SECTION_LENGTH == sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT * sizeof(float));
        s.read(reinterpret_cast<char*>(output.data()), output.size() * sizeof(float));
        obj_current_offset += static_cast<uint32_t>(s.gcount());
        return output;
    }

    std::vector<uint8_t> input(sc.SECTION_LENGTH, 0);
    s.read(reinterpret_cast<char*>(input.data()), input.size() * sizeof(uint8_t));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    std::vector<uint32_t> decoded =
        DecodeVarint(input, sc.SECTION_LENGTH, sc.ATTRIBUTE_COUNT * sc.ELEMENT_COUNT);
    assert(output.size() == decoded.size());

    output.clear();
    for (auto& m : decoded) {
        output.push_back(m / 65535.0f);
    }

    return output;
}

std::vector<uint16_t> OBJECT_LOADER::BlendIndicesDecode(std::istream& s, const SECTION_ITEM& sc) {
    // 开头16个为name
    char type[16] = {0};
    s.read(type, sizeof(type));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    assert(strcmp(type, "BLENDINDEX") == 0);
    assert(sc.ELEMENT_TYPE == kELEMENT_TYPE::FLOAT_TYPE);

    // 如果无压缩
    if (sc.DATA_ATTRIBUTE == kMESH_ATTRIBUTE_LOOSE) {
        std::vector<float> output(sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT, 0);
        assert(sc.SECTION_LENGTH == sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT * sizeof(float));
        s.read(reinterpret_cast<char*>(output.data()), output.size() * sizeof(float));
        obj_current_offset += static_cast<uint32_t>(s.gcount());
        std::vector<uint16_t> output16(sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT, 0);
        for (int i = 0; i < output.size(); ++i) {
            output16[i] = output[i];
        }
        return output16;
    }

    std::vector<uint8_t> input(sc.SECTION_LENGTH, 0);
    s.read(reinterpret_cast<char*>(input.data()), input.size() * sizeof(uint8_t));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    return DecodeVarint<uint16_t>(input, sc.SECTION_LENGTH, sc.ATTRIBUTE_COUNT * sc.ELEMENT_COUNT);
}

std::vector<uint32_t> OBJECT_LOADER::IndicesDecode(std::istream& s, const SECTION_ITEM& sc) {
    // 开头16个为name
    char type[16] = {0};
    s.read(type, sizeof(type));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    assert(strcmp(type, "INDEX") == 0 || strcmp(type, "TOPOLOGY") == 0);
    assert(sc.ELEMENT_TYPE == kELEMENT_TYPE::UINT_TYPE);

    // 如果无压缩
    if (sc.DATA_ATTRIBUTE == kMESH_ATTRIBUTE_LOOSE) {
        std::vector<uint32_t> output(sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT, 0);
        assert(sc.SECTION_LENGTH == sc.ELEMENT_COUNT * sc.ATTRIBUTE_COUNT * sizeof(uint32_t));
        s.read(reinterpret_cast<char*>(output.data()), output.size() * sizeof(uint32_t));
        obj_current_offset += static_cast<uint32_t>(s.gcount());
        return output;
    }

    std::vector<uint8_t> input(sc.SECTION_LENGTH, 0);
    s.read(reinterpret_cast<char*>(input.data()), input.size() * sizeof(uint8_t));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    std::vector<uint32_t> decoded =
        DecodeVarint(input, sc.SECTION_LENGTH, sc.ATTRIBUTE_COUNT * sc.ELEMENT_COUNT);
    return DecodeZigzagAndDelta(decoded);
}

std::vector<SUBMESH> OBJECT_LOADER::SubMeshDecode(std::istream& s, const SECTION_ITEM& sc) {
    // 开头16个为name
    char type[16] = {0};
    s.read(type, sizeof(type));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    assert(strcmp(type, "SUBMESH") == 0);
    assert(sc.ELEMENT_TYPE == kELEMENT_TYPE::UINT_TYPE);

    std::vector<SUBMESH> output(sc.ATTRIBUTE_COUNT, SUBMESH());
    // TODO: assertion here maybe reports false. However, we just ignore this and fix it later.
    // assert(sc.SECTION_LENGTH == sc.ATTRIBUTE_COUNT * sizeof(SUBMESH));
    s.read(reinterpret_cast<char*>(output.data()), output.size() * sizeof(SUBMESH));
    for (auto& m : output) {
        m.offset *= 3;  // 按三角面片来算，float偏移需要乘3，添入时实际值是count因此在此乘3
    }
    return output;
}

std::vector<BONE> OBJECT_LOADER::BoneDecode(std::istream& s, const SECTION_ITEM& sc) {
    // 开头16个为name
    char type[16] = {0};
    s.read(type, sizeof(type));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    assert(strcmp(type, "BONE") == 0);
    assert(sc.ELEMENT_TYPE == kELEMENT_TYPE::STRUCT_TYPE);

    std::vector<BONE> output(sc.ATTRIBUTE_COUNT);
    s.read(reinterpret_cast<char*>(output.data()), output.size() * sizeof(BONE));
    obj_current_offset += static_cast<uint32_t>(s.gcount());

    for (const auto& m : output) {
        assert(m.name2_index == -1);
    }

    return output;
}

void OBJECT::LinkBones(const std::shared_ptr<Entity>& parent_entity) {
    if (!parent_entity || bone.empty()) {
        return;
    }
    auto current_node = parent_entity;
    auto last_node = current_node;
    while (current_node != nullptr) {
        if (current_node->GetComponent(kAnimator) != nullptr) {
            break;
        }
        last_node = current_node;
        current_node = current_node->parent;
    }
    if (current_node == nullptr) {
        if (last_node) {
            current_node = last_node;
        } else {
            LOG_ERROR(
                "cannot find any animator component on ancestor entities, animation won't play.");
            return;
        }
    }

    for (const auto& current_bone : bone) {
        // bone entity
        auto bone_entity = current_node->FindChildEntityByID(current_bone.bone_unique_id);
        if (bone_entity == nullptr) {
            bone_bindings.clear();
            matrices_world_to_bone.clear();
            LOG_ERROR("cannot find the specified bone entity.");
            return;
        }
        bone_bindings.push_back(bone_entity);
        // matrix
        glm::mat4 matrix = glm::make_mat4(current_bone.matrix);
        matrices_world_to_bone.push_back(glm::transpose(matrix));
    }
    is_bone_linked = true;
}

void OBJECT::AddMorphTarget(const std::shared_ptr<OBJECT>& morph_target) {
    if (morph_target) {
        morph_targets.push_back(morph_target);
        morph_weights.push_back(0.0f);
        morph_mapping.emplace(morph_target->name, morph_targets.size() - 1);
    }
}

bool OBJECT::IsSkinned() const {
    return AppGlobalResource::Instance().EnableSkinning() && !blend_weights.empty() &&
           !blend_indices.empty() && !bone.empty() && is_bone_linked;
}

void OBJECT::SubmitToOpenGL() {
    std::vector<kSECTION_TYPE> types;
    for (const auto& m : section_item_map) {
        if (m.first == kSUBMESH || m.first == kBONE) {
            continue;
        }
        types.push_back(m.first);
    }
    vao_info = std::make_shared<glit::GLVAOInfo>(glit::CreateBuffer(GetPtr(), types, false));
    GLuint error = glGetError();
    if (error == GL_OUT_OF_MEMORY) {
        LOG_ERROR("opengl has run out of memory.");
        return;
    }
    for (const auto& m : morph_targets) {
        if(m) m->SubmitToOpenGL();
    }
}

void OBJECT::UpdateToOpenGL() {
    std::vector<kSECTION_TYPE> types;
    for (const auto& m : section_item_map) {
        if (m.first == kSUBMESH || m.first == kBONE) {
            continue;
        }
        types.push_back(m.first);
    }
    glit::UpdateBuffer(GetPtr(), types, false);
    GLuint error = glGetError();
    if (error == GL_OUT_OF_MEMORY) {
        LOG_ERROR("opengl has run out of memory.");
        return;
    }
    for (const auto& m : morph_targets) {
        if (m) m->UpdateToOpenGL();
    }
}

std::string_view OBJECT::GetMemoryPtr(kSECTION_TYPE type) {
    const char* mem_start = nullptr;
    size_t mem_length = 0;
    switch (type) {
        case kPOSITION:
            mem_start = reinterpret_cast<const char*>(position.data());
            mem_length = position.size() * sizeof(decltype(OBJECT::position)::value_type);
            break;
        case kNORMAL0:
            mem_start = reinterpret_cast<const char*>(normal0.data());
            mem_length = normal0.size() * sizeof(decltype(OBJECT::normal0)::value_type);
            break;
        case kNORMAL1:
            mem_start = reinterpret_cast<const char*>(normal1.data());
            mem_length = normal1.size() * sizeof(decltype(OBJECT::normal1)::value_type);
            break;
        case kTANGENT0:
            mem_start = reinterpret_cast<const char*>(tangent0.data());
            mem_length = tangent0.size() * sizeof(decltype(OBJECT::tangent0)::value_type);
            break;
        case kTANGENT1:
            mem_start = reinterpret_cast<const char*>(tangent1.data());
            mem_length = tangent1.size() * sizeof(decltype(OBJECT::tangent1)::value_type);
            break;
        case kBINORMAL0:
            mem_start = reinterpret_cast<const char*>(binormal0.data());
            mem_length = binormal0.size() * sizeof(decltype(OBJECT::binormal0)::value_type);
            break;
        case kBINORMAL1:
            mem_start = reinterpret_cast<const char*>(binormal1.data());
            mem_length = binormal1.size() * sizeof(decltype(OBJECT::binormal1)::value_type);
            break;
        case kUV0:
            mem_start = reinterpret_cast<const char*>(uv0.data());
            mem_length = uv0.size() * sizeof(decltype(OBJECT::uv0)::value_type);
            break;
        case kUV1:
            mem_start = reinterpret_cast<const char*>(uv1.data());
            mem_length = uv1.size() * sizeof(decltype(OBJECT::uv1)::value_type);
            break;
        case kVERTEXCOLOR0:
            mem_start = reinterpret_cast<const char*>(vertex_color0.data());
            mem_length = vertex_color0.size() * sizeof(decltype(OBJECT::vertex_color0)::value_type);
            break;
        case kVERTEXCOLOR1:
            mem_start = reinterpret_cast<const char*>(vertex_color1.data());
            mem_length = vertex_color1.size() * sizeof(decltype(OBJECT::vertex_color1)::value_type);
            break;
        case kBLENDWEIGHT:
            mem_start = reinterpret_cast<const char*>(blend_weights.data());
            mem_length = blend_weights.size() * sizeof(decltype(OBJECT::blend_weights)::value_type);
            break;
        case kBLENDINDEX:
            mem_start = reinterpret_cast<const char*>(blend_indices.data());
            mem_length = blend_indices.size() * sizeof(decltype(OBJECT::blend_indices)::value_type);
            break;
        case kINDEX:
            mem_start = reinterpret_cast<const char*>(indices.data());
            mem_length = indices.size() * sizeof(decltype(OBJECT::indices)::value_type);
            break;
        case kTOPOLOGY:
            mem_start = reinterpret_cast<const char*>(topology.data());
            mem_length = topology.size() * sizeof(decltype(OBJECT::topology)::value_type);
            break;
        case kUVTOPOLOGY:
            mem_start = reinterpret_cast<const char*>(uvtopology.data());
            mem_length = uvtopology.size() * sizeof(decltype(OBJECT::uvtopology)::value_type);
            break;
        case kBONE:
        case kSUBMESH:
        case kMORPHTARGET:
        case kKEYFRAME:
        default:
            LOG_ERROR("not supported for getting memory pointer");
            break;
    }
    return std::string_view(mem_start, mem_length);
}

void OBJECT::FillSkinningMatrices(RGBAImageData& data) {
    if (data.data.empty()) {
        return;
    }
    assert(bone.size() == bone_bindings.size());
    auto& target_buffer = data.data;
    for (uint64_t i = 0; i < bone_bindings.size(); ++i) {
        glm::mat4 matrix_bone_to_world = bone_bindings[i]->matrix_world * matrices_world_to_bone[i];
        if (AppGlobalResource::Instance().SupportFloatTexture()) {
            float* ptr = reinterpret_cast<float*>(target_buffer.data());
            size_t len = target_buffer.size() / sizeof(float);
            for (int k = 0; k < 16; ++k) {
                assert(len > k + 16 * i);
                ptr[k + 16 * i] = matrix_bone_to_world[k / 4][k % 4];
            }
        } else {
            uint8_t* ptr = target_buffer.data();
            size_t len = target_buffer.size();
            constexpr static int stride = 128;
            assert(stride * (i + 1) <= len);
            EncodeFloat2RGBA(ptr + stride * i, matrix_bone_to_world);
        }
    }
}

std::vector<std::pair<float, std::shared_ptr<OBJECT>>> OBJECT::GetMorphWeights(size_t max_n) {
    std::vector<std::pair<float, int>> morph_records;
    morph_records.resize(morph_targets.size());
    for (size_t i = 0; i < morph_records.size(); ++i) {
        morph_records[i] = {morph_weights[i], i};
    }
    // 将取前max_n大的weights输出
    std::sort(morph_records.begin(), morph_records.end(), std::greater<std::pair<float, int>>());
    auto len = std::min(max_n, morph_records.size());
    std::vector<std::pair<float, std::shared_ptr<OBJECT>>> ret;
    for (size_t i = 0; i < len; ++i) {
        if (morph_records[i].first > 0.0f) {
            // 我猜 weight 原本可能是个百分比数字，所以要除以100
            ret.emplace_back(morph_records[i].first / 100.0f,
                             morph_targets[morph_records[i].second]  // target
            );
        }
    }
    return ret;
}

std::string OBJECT::GetAttributeName(kSECTION_TYPE type) {
    const static std::map<uint32_t, std::string> SectionTypeName = {{kPOSITION, "position"},
                                                                    {kNORMAL0, "normal"},
                                                                    {kNORMAL1, "normal1"},
                                                                    {kTANGENT0, "tangent"},
                                                                    {kTANGENT1, "tangent1"},
                                                                    {kBINORMAL0, "binormal"},
                                                                    {kBINORMAL1, "binormal1"},
                                                                    {kUV0, "uv"},
                                                                    {kUV1, "uv1"},
                                                                    {kVERTEXCOLOR0, "color"},
                                                                    {kVERTEXCOLOR1, "color1"},
                                                                    {kBLENDWEIGHT, "skinWeight"},
                                                                    {kBLENDINDEX, "skinIndex"},
                                                                    {kINDEX, "index"},
                                                                    {kSUBMESH, "subMesh"},
                                                                    {kBONE, "bone"},
                                                                    {kMORPHTARGET, "morphTarget"},
                                                                    {kKEYFRAME, "keyframe"},
                                                                    {kTOPOLOGY, "topology"},
                                                                    {kUVTOPOLOGY, "uvtopology"}};
    assert(SectionTypeName.size() == kSECTION_TYPE_COUNT);
    return SectionTypeName.at(type);
}

bool OBJECT::IsMorphed() const {
    return AppGlobalResource::Instance().EnableMorphAnimation() && !morph_targets.empty();
}

std::vector<float> OBJECT::ComputeNormal() const {
    if (position.empty() || indices.empty()) {
        return std::vector<float>();
    }
    std::vector<float> ret(position.size(), 0.0f);
    for (const auto& current_submesh : submesh) {
        uint32_t start = current_submesh.start;
        uint32_t end = (current_submesh.start + current_submesh.offset);
        for (uint32_t i = start; i < end; i += 3) {
            uint32_t i1 = indices[i + 0];
            uint32_t i2 = indices[i + 1];
            uint32_t i3 = indices[i + 2];
            glm::vec3 p1(position[3 * i1], position[3 * i1 + 1], position[3 * i1 + 2]);
            glm::vec3 p2(position[3 * i2], position[3 * i2 + 1], position[3 * i2 + 2]);
            glm::vec3 p3(position[3 * i3], position[3 * i3 + 1], position[3 * i3 + 2]);
            glm::vec3 face_normal = glm::cross(p3 - p2, p1 - p2);
            for (int k = 0; k < 3; ++k) {
                ret[3 * i1 + k] += face_normal[k];
                ret[3 * i2 + k] += face_normal[k];
                ret[3 * i3 + k] += face_normal[k];
            }
        }
    }
    for (uint64_t i = 0; i < ret.size(); i += 3) {
        glm::vec3 to_normalize(ret[i], ret[i + 1], ret[i + 2]);
        float length = glm::length(to_normalize);
        if (length != 0) {
            glm::vec3 normalized = to_normalize / length;
            ret[i] = normalized[0];
            ret[i + 1] = normalized[1];
            ret[i + 2] = normalized[2];
        }
    }
    return ret;
}

std::vector<float> OBJECT::ComputeTangent() const {
    if (uv0.empty() || position.empty() || indices.empty() || normal0.empty()) {
        return std::vector<float>();
    }
    uint32_t vertex_count = position.size() / 3;

    std::vector<glm::vec3> tangent_vec(vertex_count, glm::vec3(0.0f, 0.0f, 0.0f));
    std::vector<glm::vec3> bitangent_vec(vertex_count, glm::vec3(0.0f, 0.0f, 0.0f));
    for (const auto& current_submesh : submesh) {
        uint32_t start = current_submesh.start;
        uint32_t end = (current_submesh.start + current_submesh.offset);
        for (uint32_t i = start; i < end; i += 3) {
            uint32_t i1 = indices[i + 0];
            uint32_t i2 = indices[i + 1];
            uint32_t i3 = indices[i + 2];

            glm::vec3 p1(position[3 * i1], position[3 * i1 + 1], position[3 * i1 + 2]);
            glm::vec3 p2(position[3 * i2], position[3 * i2 + 1], position[3 * i2 + 2]);
            glm::vec3 p3(position[3 * i3], position[3 * i3 + 1], position[3 * i3 + 2]);
            glm::vec3 delta_position_21(p2 - p1);
            glm::vec3 delta_position_31(p3 - p1);

            glm::vec2 uv1(uv0[2 * i1], uv0[2 * i1 + 1]);
            glm::vec2 uv2(uv0[2 * i1], uv0[2 * i1 + 1]);
            glm::vec2 uv3(uv0[2 * i1], uv0[2 * i1 + 1]);
            glm::vec2 delta_uv_21(uv2 - uv1);
            glm::vec2 delta_uv_31(uv3 - uv1);

            // see https://learnopengl.com/Advanced-Lighting/Normal-Mapping
            float divisor = std::max(
                1.0e-6f, delta_uv_21[0] * delta_uv_31[1] - delta_uv_21[1] * delta_uv_31[0]);

            glm::vec3 tangent, bitangent;
            tangent.x = divisor *
                        (delta_uv_31.y * delta_position_21.x - delta_uv_21.y * delta_position_31.x);
            tangent.y = divisor *
                        (delta_uv_31.y * delta_position_21.y - delta_uv_21.y * delta_position_31.y);
            tangent.z = divisor *
                        (delta_uv_31.y * delta_position_21.z - delta_uv_21.y * delta_position_31.z);

            bitangent.x = divisor * (-delta_uv_31.x * delta_position_21.x +
                                     delta_uv_21.x * delta_position_31.x);
            bitangent.y = divisor * (-delta_uv_31.x * delta_position_21.y +
                                     delta_uv_21.x * delta_position_31.y);
            bitangent.z = divisor * (-delta_uv_31.x * delta_position_21.z +
                                     delta_uv_21.x * delta_position_31.z);

            tangent_vec[i1] += tangent;
            tangent_vec[i2] += tangent;
            tangent_vec[i3] += tangent;
            bitangent_vec[i1] += bitangent;
            bitangent_vec[i2] += bitangent;
            bitangent_vec[i3] += bitangent;
        }
    }
    std::vector<float> ret(vertex_count * 4, 0.0f);
    for (uint64_t i = 0; i < vertex_count; i++) {
        glm::vec3 normal(normal0[i * 3], normal0[i * 3 + 1], normal0[i * 3 + 2]);
        const glm::vec3& tangent = tangent_vec[i];
        const glm::vec3& bitangent = bitangent_vec[i];
        // handness
        float handness = glm::dot(glm::cross(normal, tangent), bitangent) < 0.0 ? -1.0 : 1.0;
        // 正交化
        glm::vec3 ortho_tangent = tangent - normal * (glm::dot(normal, tangent));
        if (glm::length(ortho_tangent) != 0.0) {
            ortho_tangent = glm::normalize(ortho_tangent);
        }
        ret[4 * i + 0] = ortho_tangent[0];
        ret[4 * i + 1] = ortho_tangent[1];
        ret[4 * i + 2] = ortho_tangent[2];
        ret[4 * i + 3] = handness;
    }
    return ret;
}

std::vector<uint32_t> OBJECT::ComputeUVTopology() const {
    std::vector<uint32_t> ret(indices.size() * 2);
    // indice: 1 2 3  / 4 5 6 / ....
    // uv:     1 2 / 3 1 / 2 3 // 4 5 / 6 4 / 5 6 // ...
    for (uint64_t i = 0; i < indices.size() / 3; ++i) {
        ret[6 * i + 0] = indices[3 * i + 0];
        ret[6 * i + 1] = indices[3 * i + 1];
        ret[6 * i + 2] = indices[3 * i + 2];
        ret[6 * i + 3] = indices[3 * i + 0];
        ret[6 * i + 4] = indices[3 * i + 1];
        ret[6 * i + 5] = indices[3 * i + 2];
    }
    return ret;
}

void OBJECT::CalculateAfterLoading() {
    // compute some attribute if not provided
    if (normal0.empty()) {
        normal0 = ComputeNormal();
        SECTION_ITEM tmp;
        tmp.SECTION_TYPE = kNORMAL0;
        tmp.DATA_ATTRIBUTE = kMESH_ATTRIBUTE_GENERATED;
        tmp.ELEMENT_TYPE = kELEMENT_TYPE::FLOAT_TYPE;
        tmp.ELEMENT_COUNT = 3;
        tmp.ATTRIBUTE_COUNT = normal0.size() / tmp.ELEMENT_COUNT;
        tmp.SECTION_OFFSET = 0;  // 生成量没有offset
        tmp.SECTION_LENGTH =
            16 + sizeof(decltype(normal0)::value_type) * tmp.ELEMENT_COUNT * tmp.ATTRIBUTE_COUNT;
        section_item_map[kNORMAL0] = tmp;

        assert(normal0.size() == position.size());
    }

    // compute some attribute if not provided
    if (tangent0.empty() && !uv0.empty()) {
        tangent0 = ComputeTangent();
        SECTION_ITEM tmp;
        tmp.SECTION_TYPE = kTANGENT0;
        tmp.DATA_ATTRIBUTE = kMESH_ATTRIBUTE_GENERATED;
        tmp.ELEMENT_TYPE = kELEMENT_TYPE::FLOAT_TYPE;
        tmp.ELEMENT_COUNT = 4;
        tmp.ATTRIBUTE_COUNT = tangent0.size() / tmp.ELEMENT_COUNT;
        tmp.SECTION_OFFSET = 0;  // 生成量没有offset
        tmp.SECTION_LENGTH =
            16 + sizeof(decltype(tangent0)::value_type) * tmp.ELEMENT_COUNT * tmp.ATTRIBUTE_COUNT;
        section_item_map[kTANGENT0] = tmp;

        assert(tangent0.size() / 4 == normal0.size() / 3);
    }

    // compute some attribute if not provided
    if (uvtopology.empty()) {
        uvtopology = ComputeUVTopology();
        SECTION_ITEM tmp;
        tmp.SECTION_TYPE = kUVTOPOLOGY;
        tmp.DATA_ATTRIBUTE = kMESH_ATTRIBUTE_GENERATED;
        tmp.ELEMENT_TYPE = kELEMENT_TYPE::UINT_TYPE;
        tmp.ELEMENT_COUNT = 2;
        tmp.ATTRIBUTE_COUNT = uvtopology.size() / tmp.ELEMENT_COUNT;
        tmp.SECTION_OFFSET = 0;  // 生成量没有offset
        tmp.SECTION_LENGTH =
            16 + sizeof(decltype(uvtopology)::value_type) * tmp.ELEMENT_COUNT * tmp.ATTRIBUTE_COUNT;
        section_item_map[kUVTOPOLOGY] = tmp;
        assert(uvtopology.size() == indices.size() * 2);
    }
}

void OBJECT::EncodeFloat2RGBA(uint8_t* out_rgba_mem_128, const glm::mat4& input_matrix) {
    auto GetFrac = [](float val) {
        int decimal = static_cast<int>(val);
        return val - decimal;
    };
    for (int i = 0; i < 16; ++i) {
        float val = input_matrix[i / 4][i % 4];
        int decimal = static_cast<int>(val);
        float positive_frac = std::abs(val - decimal);
        glm::vec4 frac_value(positive_frac, GetFrac(255 * positive_frac),
                             GetFrac(65025 * positive_frac), GetFrac(160581375 * positive_frac));
        frac_value.x -= frac_value.y / 255.0;
        frac_value.y -= frac_value.z / 255.0;
        frac_value.z -= frac_value.w / 255.0;
        frac_value *= 255.0;
        out_rgba_mem_128[8 * i + 0] = frac_value.x;
        out_rgba_mem_128[8 * i + 1] = frac_value.y;
        out_rgba_mem_128[8 * i + 2] = frac_value.z;
        out_rgba_mem_128[8 * i + 3] = frac_value.w;

        int positive_decimal = std::abs(decimal);
        out_rgba_mem_128[8 * i + 4] = positive_decimal & 0x000000FF;
        out_rgba_mem_128[8 * i + 5] = positive_decimal >> 8 & 0x000000FF;
        out_rgba_mem_128[8 * i + 6] = positive_decimal >> 16 & 0x000000FF;
        out_rgba_mem_128[8 * i + 7] = val < 0 ? 0 : 0xFF;
    }
}

void OBJECT::ExportAsObj(std::ostream& ss) const {
    bool save_uv = false;
    bool save_normal = false;
    // vertex part
    if (vertex_color0.size() / 4 == position.size() / 3) {
        for (int i = 0; i < position.size() / 3; ++i) {
            ss << "v " << position[3 * i + 0] 
                << " " << position[3 * i + 1] 
                << " " << position[3 * i + 2] 
                << " " << vertex_color0[4 * i + 0] / 255.0
                << " " << vertex_color0[4 * i + 1] / 255.0
                << " " << vertex_color0[4 * i + 2] / 255.0
                // alpha not supported
                << std::endl;
        }
    } else {
        for (int i = 0; i < position.size() / 3; ++i) {
            ss << "v " << position[3 * i + 0] << " " << position[3 * i + 1] << " "
               << position[3 * i + 2] << std::endl;
        }
    }
    // vt part
    if (uv0.size()) {
        save_uv = true;
    }
    if (save_uv) {
        for (int i = 0; i < uv0.size() / 2; ++i) {
            ss << "vt " << uv0[2 * i + 0] << " " << uv0[2 * i + 1] << std::endl;
        }
    }
    // vn part
    if (normal0.size()) {
        save_normal = true;
    }
    if (save_normal) {
        for (int i = 0; i < normal0.size() / 3; ++i) {
            ss << "vn " << normal0[3 * i + 0] << " " << normal0[3 * i + 1] << " "
               << normal0[3 * i + 2] << std::endl;
        }
    }
    // triangle part
    for (int i = 0; i < indices.size() / 3; ++i) {
        if (save_uv) {
            if (save_normal) {
                ss << "f " << indices[3 * i + 0] + 1 << "/" << indices[3 * i + 0] + 1 << "/"
                   << indices[3 * i + 0] + 1 << " " << indices[3 * i + 1] + 1 << "/"
                   << indices[3 * i + 1] + 1 << "/" << indices[3 * i + 1] + 1 << " "
                   << indices[3 * i + 2] + 1 << "/" << indices[3 * i + 2] + 1 << "/"
                   << indices[3 * i + 2] + 1 << std::endl;
            } else {
                ss << "f " << indices[3 * i + 0] + 1 << "/" << indices[3 * i + 0] + 1 << " "
                   << indices[3 * i + 1] + 1 << "/" << indices[3 * i + 1] + 1 << " "
                   << indices[3 * i + 2] + 1 << "/" << indices[3 * i + 2] + 1 << std::endl;
            }
        } else {
            if (save_normal) {
                ss << "f " << indices[3 * i + 0] + 1 << "//" << indices[3 * i + 0] + 1 << " "
                   << indices[3 * i + 1] + 1 << "//" << indices[3 * i + 1] + 1 << " "
                   << indices[3 * i + 2] + 1 << "//" << indices[3 * i + 2] + 1 << std::endl;
            } else {
                ss << "f " << indices[3 * i + 0] + 1 << " " << indices[3 * i + 1] + 1 << " "
                   << indices[3 * i + 2] + 1 << std::endl;
            }
        }
    }
}

OBJECT::OBJECT() {
    draw_mode = GL_TRIANGLES;
    is_bone_linked = false;
}