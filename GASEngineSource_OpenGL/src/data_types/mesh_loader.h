// Copyright 2020 beanpliu tencent
#pragma once
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>
#include "glm/glm.hpp"
#include "utils/logger.h"

class RGBAImageData;
class Entity;
namespace glit {
class GLVAOInfo;
class GLTextureInfo;
}  // namespace glit

// layout of file:
/*
GEOMETRY_BIN_HEADER

STRING_TABLE_COUNT
STRING_TABLE ...

SECTION_TABLE_ENTRY_COUNT
SECTION_TABLE_ENTRY ...

SECTION_TYPE
SECTION_DATA ...

FILE_END_FLAG
*/

// datatypes:
// header memory layout:
// | 00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F  |      //
// | [  FileFlag  ]  [ ObjectSize ]  [ EndianFlag ]  [  Version   ]  |      //
// | [ SectionOff ]  [ ParentUID  ]  [  UniqueID  ]  [               | ↓    //
// |       ObjectType             ]  [  NameIndex ]  [               | ↓    //
// |       PosBboxMin             ]  [                   PosBboxMax  | ↓    //
// |              ]  [       UV0BboxMin           ]  [               | ↓    //
// |  UV0BboxMax  ]  [ StrTableOff]  [        UV1BboxMin          ]  |      //
// | [        UV1BboxMax          ]  [                     MD5       | ↓    //
// |              ]  #end (begin of stringTable)                     |      //
struct OBJECT_HEADER {
    char FILE_FLAG[4] = {0};
    uint32_t OBJECT_FILE_SIZE = 0;
    uint32_t ENDIAN_FLAG = 0;
    uint32_t VERSION = 0;
    uint32_t SECTION_TABLE_OFFSET = 0;
    int32_t PARENT_UNIQUE_ID = 0;
    int32_t UNIQUE_ID = 0;       // Offset 24bytes
    char OBJECT_TYPE[12] = {0};  // CAMERA  MESH	LIGHT
    uint32_t OBJECT_NAME_INDEX = 0;
    float POS_BBOX_MIN[3] = {0.0f, 0.0f, 0.0f};
    float POS_BBOX_MAX[3] = {0.0f, 0.0f, 0.0f};
    float UV0_BBOX_MIN[2] = {0.0f, 0.0f};
    float UV0_BBOX_MAX[2] = {0.0f, 0.0f};
    uint32_t STRING_TABLE_OFFSET = 0;
    float UV1_BBOX_MIN[2] = {0.0f, 0.0f};
    float UV1_BBOX_MAX[2] = {0.0f, 0.0f};
    char MD5[12] = {0};
};

class STRING_TABLE {
 public:
    std::vector<std::string> strings;
};

// fixed size
struct SECTION_ITEM {
 public:
    uint32_t SECTION_TYPE = 0;  // POSITION NORMAL0 NORMAL1 UV0 UV1 VERTEXCOLOR SUBMESH..
    uint32_t DATA_ATTRIBUTE = 0;
    uint32_t SECTION_OFFSET = 0;
    uint32_t SECTION_LENGTH = 0;
    uint32_t ATTRIBUTE_COUNT = 0;
    uint32_t ELEMENT_COUNT = 0;
    uint32_t ELEMENT_TYPE = 0;
};

enum kSECTION_TYPE {
    kPOSITION = 0,
    kNORMAL0 = 1,
    kNORMAL1 = 2,
    kTANGENT0 = 3,
    kTANGENT1 = 4,
    kBINORMAL0 = 5,
    kBINORMAL1 = 6,
    kUV0 = 7,
    kUV1 = 8,
    kVERTEXCOLOR0 = 9,
    kVERTEXCOLOR1 = 10,
    kBLENDWEIGHT = 11,
    kBLENDINDEX = 12,
    kINDEX = 13,
    kSUBMESH = 14,
    kBONE = 15,
    kMORPHTARGET = 16,
    kKEYFRAME = 17,
    kTOPOLOGY = 18,
    kUVTOPOLOGY = 19,  // will not in file, this is calculated after indices loading
    kSECTION_TYPE_COUNT = 20,
    kSECTION_TYPE_UNKNOWN = 255
};

enum kMESH_ATTRIBUTE {
    kMESH_ATTRIBUTE_LOOSE,
    kMESH_ATTRIBUTE_COMPRESSED,
    kMESH_ATTRIBUTE_GENERATED
};

enum kELEMENT_TYPE {
    FLOAT_TYPE = 0,
    UINT_TYPE = 1,
    STRUCT_TYPE = 2,
    UBYTE_TYPE = 3,
    ELEMENT_TYPE_COUNT = 4
};

constexpr int NORMAL_DECODING_DICTIONARY[] = {
    1,   8,   15,  22,  28,  35,  41,  47,  54,  60,  67,  73,  79,  85,  91,  97,  103, 109,
    115, 121, 126, 132, 137, 143, 148, 153, 158, 163, 168, 173, 178, 182, 186, 191, 195, 199,
    203, 206, 210, 213, 216, 219, 222, 225, 228, 230, 233, 235, 237, 239, 240, 242, 243, 244,
    245, 246, 247, 247, 247, 248, 248, 247, 247, 246, 246, 245, 244, 242, 241, 239, 238, 236,
    234, 232, 229, 227, 224, 221, 218, 215, 211, 208, 204, 201, 197, 193, 189, 184, 180, 175,
    171, 166, 161, 156, 151, 145, 140, 135, 129, 124, 118, 112, 106, 100, 94,  88,  82,  76,
    70,  63,  57,  51,  44,  38,  31,  25,  18,  12,  5,   1};

constexpr int NORMAL_ENCODING_PHI_COUNT = 120;

class SECTION {
 public:
    std::vector<SECTION_ITEM> sections;
};

// data loader
typedef void data_loader(std::istream&, const OBJECT_HEADER&, const STRING_TABLE&,
                         const SECTION_ITEM&);

class SUBMESH {
 public:
    SUBMESH() {}
    SUBMESH(uint32_t in_start, uint32_t in_triangle_count)
        : start(in_start), offset(in_triangle_count * 3) {}
    uint32_t start = 0;
    uint32_t offset = 0;
};

static_assert(sizeof(SUBMESH) == 8, "SUB_MESH size error.");

class BONE {
 public:
    int32_t bone_unique_id = -1;
    int32_t name_index = -1;
    int32_t name1_index = -1;
    int32_t name2_index = -1;   // should be always -1
    float matrix[16] = {0.0f};  // col first matrix storge
};

static_assert(sizeof(BONE) == 80, "BONE size error.");

//============================================================================================
// Object Structure
class OBJECT : public std::enable_shared_from_this<OBJECT> {
    friend class OBJECT_LOADER;

 public:
    std::string name;
    std::vector<float> position;
    std::vector<float> normal0;
    std::vector<float> normal1;
    std::vector<float> tangent0;
    std::vector<float> tangent1;
    std::vector<float> binormal0;
    std::vector<float> binormal1;
    std::vector<float> uv0;
    std::vector<float> uv1;
    std::vector<uint8_t> vertex_color0;
    std::vector<uint8_t> vertex_color1;
    std::vector<float> blend_weights;
    std::vector<uint16_t> blend_indices;
    std::vector<uint32_t> indices;
    std::vector<uint32_t> topology;
    std::vector<uint32_t> uvtopology;
    std::vector<SUBMESH> submesh;
    std::vector<BONE> bone;
    std::vector<std::shared_ptr<OBJECT>> morph_targets;
    std::vector<float> morph_weights;
    std::map<std::string, int> morph_mapping;
    std::map<kSECTION_TYPE, SECTION_ITEM> section_item_map;
    uint32_t draw_mode;
    std::shared_ptr<glit::GLVAOInfo> vao_info;

    // store some ptr in scene, each entity corresponds to bone in mesh.
    std::vector<std::shared_ptr<Entity>> bone_bindings;
    std::vector<glm::mat4> matrices_world_to_bone;

 public:
    std::shared_ptr<OBJECT> GetPtr() { return shared_from_this(); }
    static std::shared_ptr<OBJECT> GenerateOBJECT() { return std::shared_ptr<OBJECT>(new OBJECT); }
    bool IsSkinned() const;
    // 将大部分数据信息提交到opengl的buffer内，提交结果保存在vao_info内
    void SubmitToOpenGL();
    void UpdateToOpenGL();
    // 将 matrix_bone_to_world 的数据填充到 data 内
    void FillSkinningMatrices(RGBAImageData& data);
    // 在entity结构关联好后调用此函数，从parent_entity开始向上查找，找到最近的动画component节点或到最上方后停止，
    // 而后向下递归查找bone内指定id对应的entity，结果将存储在 bone_bindings 和
    // matrices_world_to_bone
    void LinkBones(const std::shared_ptr<Entity>& parent_entity);

    void AddMorphTarget(const std::shared_ptr<OBJECT>& morph_target);

    // return pair: weight and target
    std::vector<std::pair<float, std::shared_ptr<OBJECT>>> GetMorphWeights(size_t max_n = 4);

    static std::string GetAttributeName(kSECTION_TYPE type);
    std::string_view GetMemoryPtr(kSECTION_TYPE type);
    void SetMorphWeight(int index, float weight) {
        if (index < morph_weights.size()) morph_weights[index] = weight;
    }
    void SetMorphWeightByName(const std::string& name, float weight) {
        auto it = morph_mapping.find(name);
        if (it != morph_mapping.end()) {
            SetMorphWeight(it->second, weight);
        }
    }
    bool IsMorphed() const;

    std::vector<float> ComputeNormal() const;
    std::vector<float> ComputeTangent() const;
    std::vector<uint32_t> ComputeUVTopology() const;
    void CalculateAfterLoading();

    static void EncodeFloat2RGBA(uint8_t* out_rgba_mem_128, const glm::mat4& input_matrix);

    void ExportAsObj(std::ostream& s) const;

 private:
    OBJECT();
    OBJECT_HEADER header;
    STRING_TABLE string_table;
    SECTION section;
    bool is_bone_linked;
};
using pOBJECT = std::shared_ptr<OBJECT>;

// load binary object.
class OBJECT_LOADER {
 public:
    std::shared_ptr<OBJECT> LoadObject(std::istream& s);
    std::shared_ptr<OBJECT> LoadSingleObject(std::istream& s);
    static bool CheckHeader(const OBJECT_HEADER& header);
    static bool CheckSection(const SECTION& section);
    static void DescribeSection(int index, const SECTION_ITEM& item);

 private:
    OBJECT_HEADER LoadHeader(std::istream& s);
    STRING_TABLE LoadStringTable(std::istream& s);
    SECTION LoadSection(std::istream& s);
    void LoadSectionData(std::istream& s, const SECTION_ITEM& sc, OBJECT& obj);

    std::vector<float> PositionDataDecode(std::istream& s, const float bbox_min[3],
                                          const float bbox_max[3], const SECTION_ITEM& sc);
    std::vector<float> NormalDataDecode(std::istream& s, const SECTION_ITEM& sc);
    std::vector<float> TangentDataDecode(std::istream& s, const SECTION_ITEM& sc);
    std::vector<float> UVDataDecode(std::istream& s, const float bbox_min[2],
                                    const float bbox_max[2], const SECTION_ITEM& sc);
    std::vector<uint8_t> VertexColorDecode(std::istream& s, const SECTION_ITEM& sc);
    std::vector<float> BlendWeightDecode(std::istream& s, const SECTION_ITEM& sc);
    std::vector<uint16_t> BlendIndicesDecode(std::istream& s, const SECTION_ITEM& sc);
    std::vector<uint32_t> IndicesDecode(std::istream& s, const SECTION_ITEM& sc);
    std::vector<SUBMESH> SubMeshDecode(std::istream& s, const SECTION_ITEM& sc);
    std::vector<BONE> BoneDecode(std::istream& s, const SECTION_ITEM& sc);

    // tmp var, will be member variable.
    uint32_t obj_start_offset = 0;
    uint32_t obj_current_offset = 0;  // relative to start offset
};