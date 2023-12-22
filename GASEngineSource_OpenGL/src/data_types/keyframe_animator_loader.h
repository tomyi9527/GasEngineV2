#pragma once
#include <stdint.h>
#include <istream>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "data_types/mesh_loader.h"
#include "glm/gtc/quaternion.hpp"

class Entity;

enum kAnimationTargetType {
    kTARGET_TYPE_TX,
    kTARGET_TYPE_TY,
    kTARGET_TYPE_TZ,
    kTARGET_TYPE_RX,
    kTARGET_TYPE_RY,
    kTARGET_TYPE_RZ,
    kTARGET_TYPE_SX,
    kTARGET_TYPE_SY,
    kTARGET_TYPE_SZ,
    kTARGET_TYPE_MORPH_WEIGHT,
    kTARGET_TYPE_VISIBILITY,
    kTARGET_TYPE_RQ = 11,
    kTARGET_TYPE_RQ_X = 11,
    kTARGET_TYPE_RQ_Y,
    kTARGET_TYPE_RQ_Z,
    kTARGET_TYPE_RQ_W,
    kTARGET_TYPE_TOTAL_COUNT
};

enum kAnimationKeyIndexType { kKeyIndexType_FLOAT = 0, kKeyIndexType_MMD = 1 };
enum kAnimationKeyValueType {
    kKeyValueType_FLOAT = 1,
    kKeyValueType_FLOAT_BEZIER_MMD = 2,
    kKeyValueType_QUATERNION_BEZIER_MMD = 10
};

static_assert(kTARGET_TYPE_TOTAL_COUNT == 15, "type count is not 15");

struct ANIMATION_BIN_HEADER {
    char FILE_FLAG[4];
    uint32_t ANIMATION_FILE_SIZE;
    uint32_t ENDIAN_FLAG;
    uint32_t VERSION;
    uint32_t SECTION_TABLE_OFFSET;
    int32_t CLIP_UNIQUEID;
    char OBJECT_TYPE[12];  // CAMERA  MESH	LIGHT  ANIMATION
    uint32_t OBJECT_NAME_INDEX;
    float FPS;
    float START_FRAME;
    float END_FRAME;
    char UNUSED[32];
    uint32_t STRING_TABLE_OFFSET;
    char MD5[12];
};

struct ANIMATION_TARGET_DATA {
    uint8_t targetType;
    uint8_t keyValueType;
    uint8_t keyIndexType;  // For compression use
    uint8_t keySize;
    uint32_t keyCount;
    uint32_t
        animationDataSize;  // For compression use size = key frame index size + key frame data size
    uint32_t propertyStringIndex;
    // void* animationData; // 后续长度由类型和count共同你决定
};

struct ANIMATION_SECTION_DATA {
    uint32_t objectNameIndex;
    uint32_t _reserved[3];
    uint32_t objectID;
    // 后续将接 attribute_count 个 ANIMATION_TARGET_DATA
};

class KeyframeFloatDataValue {
 public:
    float value = 0.0f;
    uint8_t mmd_bezier_x1 = 0;
    uint8_t mmd_bezier_y1 = 0;
    uint8_t mmd_bezier_x2 = 0;
    uint8_t mmd_bezier_y2 = 0;
};

class KeyframeQuatDataValue {
 public:
    glm::quat value = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
    uint8_t mmd_bezier_x1 = 0;
    uint8_t mmd_bezier_y1 = 0;
    uint8_t mmd_bezier_x2 = 0;
    uint8_t mmd_bezier_y2 = 0;
};

// using KeyframeIndexType = std::variant<int, float>;
using KeyframeIndexType = float;
using KeyframeDataType = std::variant<float, KeyframeFloatDataValue, KeyframeQuatDataValue>;

class KeyframeSetting {
 public:
    uint8_t key_index_type = 0;
    uint8_t key_value_type = 0;
    uint8_t target = 0;
    std::string target_name;
    std::vector<KeyframeIndexType> frames;
    std::vector<KeyframeDataType> values;
};

class AnimationChannel {
 public:
    std::vector<KeyframeSetting> kfs;
    std::string node_name;
    int32_t unique_id = -1;
    float start_frame = 0.0f;
    float end_frame = 0.0f;
    float duration = 0.0f;
    std::shared_ptr<Entity> node;

 public:
    bool HasEulerRotation() const;
};

using InterpolatedValue = std::variant<float, glm::quat>;
class KeyframeAnimation {
    friend class KeyframeAnimationLoader;

 public:
    static std::shared_ptr<KeyframeAnimation> GenerateAnimation() {
        return std::shared_ptr<KeyframeAnimation>(new KeyframeAnimation);
    }
    void CalculateAfterLoading();
    void LinkToObjectsAndProperties(const std::shared_ptr<Entity>& target_entity);
    void Update(float delta);
    void AnimateTransformationV3();

 public:
    static InterpolatedValue Interpolate(const KeyframeSetting& kfs, float duration,
                                        float frame_index);

 public:
    float fps = 0.0f;
    int32_t version = -1;
    int32_t clip_id = -1;
    std::string clip_name;
    std::vector<AnimationChannel> animated_nodes;
    float start_frame = 0.0f;
    float end_frame = 0.0f;
    float clamped_start_frame = 0.0f;
    float clamped_end_frame = 0.0f;
    float actual_start_frame = 0.0f;
    float actual_end_frame = 0.0f;
    float clip_duration = 0.0f;

 public:
    float StartTime() const { return start_frame / fps; }
    float EndTime() const { return end_frame / fps; }
    float Duration() const { return (end_frame - start_frame) / fps; }
    float GetCurrentFrame() const { return local_time * fps - start_frame; }
    float GetTotalFrame() const { return end_frame - start_frame; }
    float GetProgress() const { return GetCurrentFrame() / GetTotalFrame(); }
    void SetProgress(float val);

    // controling
    bool enable = false;
    bool enable_clamp = false;
    bool clip_started = false;
    bool clip_ended = false;
    bool force_update = false;
    float local_time = 0.0f;
    float speed = 1.0f;

 protected:
    KeyframeAnimation() {}
    ANIMATION_BIN_HEADER header;
    STRING_TABLE string_table;
    SECTION section;
};

class KeyframeAnimationLoader {
 public:
    std::shared_ptr<KeyframeAnimation> LoadAnimation(std::istream& s);
    static bool CheckHeader(const ANIMATION_BIN_HEADER& header);
    static bool CheckSection(const SECTION& section);

 private:
    ANIMATION_BIN_HEADER LoadHeader(std::istream& s);
    STRING_TABLE LoadStringTable(std::istream& s);
    SECTION LoadSection(std::istream& s);
    void LoadSectionData(std::istream& s, const SECTION_ITEM& sc, KeyframeAnimation& obj);

    // tmp var, will be member variable.
    uint32_t obj_start_offset = 0;
    uint32_t obj_current_offset = 0;
};