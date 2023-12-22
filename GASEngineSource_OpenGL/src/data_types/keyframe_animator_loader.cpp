#include "data_types/keyframe_animator_loader.h"
#include <string.h>
#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include "ecs/component/mesh_filter_component.h"
#include "ecs/entity_factory.h"

static_assert(sizeof(ANIMATION_BIN_HEADER) == 100, "sizeof ANIMATION_BIN_HEADER is not 100");
static_assert(sizeof(KeyframeFloatDataValue) == 8, "sizeof KeyframeFloatDataValue is not 8");
static_assert(sizeof(KeyframeQuatDataValue) == 20, "sizeof KeyframeQuatDataValue is not 20");
static_assert(sizeof(ANIMATION_SECTION_DATA) == 20, "sizeof ANIMATION_SECTION_DATA is not 20");

// const var from mesh_loader.cpp
const static char* FILE_BEGINNING_FLAG = "NEO*";
const static char* FILE_END_FLAG = "*OEN";
const static uint32_t ENDIAN_FLAG_VALUE = 0x12345678;
const static std::set<std::string_view> OBJECT_TYPE_CHOICES = {"UNKNOWN", "MESH", "MORPHTARGET",
                                                               "ANIMATION"};  // not section type!

class InterpolateVisitor {
 public:
    InterpolateVisitor(float in_weight) : weight(in_weight) {}
    void operator()(float v) {
        if (call_times == 0) {
            result = v * (1.0f - weight);
        } else if (call_times == 1) {
            result = std::get<float>(result) + v * weight;
        } else {
            assert(0);
        }
        call_times++;
    }
    void operator()(const KeyframeFloatDataValue& v) {
        if (call_times == 0) {
            result = v.value * (1.0 - weight);
        } else if (call_times == 1) {
            result = std::get<float>(result) + v.value * weight;
        } else {
            assert(0);
        }
        call_times++;
    }
    void operator()(const KeyframeQuatDataValue& v) {
        if (call_times == 0) {
            result = v.value;
        } else if (call_times == 1) {
            result = glm::mix(std::get<glm::quat>(result), v.value, weight);
        } else {
            assert(0);
        }
        call_times++;
    }

    InterpolatedValue GetResult() { return result; }
    InterpolatedValue result;
    float weight = 1.0f;
    int call_times = 0;
};

InterpolatedValue KeyframeAnimation::Interpolate(const KeyframeSetting& kfs, float duration,
                                                 float frame_index) {
    if (kfs.target == kTARGET_TYPE_VISIBILITY) {
    } else if (kfs.target == kTARGET_TYPE_RQ) {
    } else {
    }

    if (kfs.frames.size() == 1 || frame_index < kfs.frames.front()) {
        InterpolateVisitor tmp(0.0f);
        std::visit(tmp, kfs.values.front());
        return tmp.GetResult();
    }
    if (frame_index >= kfs.frames.back()) {
        InterpolateVisitor tmp(0.0f);
        std::visit(tmp, kfs.values.back());
        return tmp.GetResult();
    }
    // now each value is in [frames.min(), frames.max())
    // returns the first element >= frame_index
    auto right_bound = std::lower_bound(kfs.frames.begin(), kfs.frames.end(), frame_index);
    if (right_bound == kfs.frames.end()) {
        assert(0);
    }
    int right_idx = std::distance(kfs.frames.begin(), right_bound);
    if (*right_bound == frame_index) {
        InterpolateVisitor tmp(0.0f);
        std::visit(tmp, kfs.values[right_idx]);
        return tmp.GetResult();
    }
    assert(right_bound != kfs.frames.begin());
    auto left_bound = right_bound - 1;
    int left_idx = std::distance(kfs.frames.begin(), left_bound);
    float weight = (frame_index - *left_bound) / (*right_bound - *left_bound);
    InterpolateVisitor visitor(weight);
    std::visit(visitor, kfs.values[left_idx]);
    std::visit(visitor, kfs.values[right_idx]);
    return visitor.GetResult();
}

std::shared_ptr<KeyframeAnimation> KeyframeAnimationLoader::LoadAnimation(std::istream& s) {
    std::shared_ptr<KeyframeAnimation> obj_ptr = KeyframeAnimation::GenerateAnimation();
    KeyframeAnimation& obj = *obj_ptr;
    if (!s || s.eof()) {
        return obj_ptr;
    }
    obj_start_offset = static_cast<uint32_t>(s.tellg());
    obj_current_offset = 0;
    // header
    obj.header = LoadHeader(s);
    assert(CheckHeader(obj.header));
    assert(obj_current_offset == sizeof(ANIMATION_BIN_HEADER));

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
    }
    obj.CalculateAfterLoading();
    return obj_ptr;
}

// load header
ANIMATION_BIN_HEADER KeyframeAnimationLoader::LoadHeader(std::istream& s) {
    assert(obj_current_offset == 0);
    ANIMATION_BIN_HEADER header;
    char* p_header = reinterpret_cast<char*>(&header);
    s.read(p_header, sizeof(ANIMATION_BIN_HEADER));
    obj_current_offset += static_cast<uint32_t>(s.gcount());
    // assert(obj_current_offset == sizeof(ANIMATION_BIN_HEADER));
    if (obj_current_offset != sizeof(ANIMATION_BIN_HEADER)) {
        LOG_ERROR("ANIMATION_BIN_HEADER is incomplete.");
    }
    return header;
}

bool KeyframeAnimationLoader::CheckHeader(const ANIMATION_BIN_HEADER& header) {
    // check FILE_FLAG
    std::string_view FileFlag(header.FILE_FLAG, 4);
    std::string_view BeginFlag(FILE_BEGINNING_FLAG, 4);
    if (FileFlag != BeginFlag) {
        LOG_ERROR("FILE_FLAG in ANIMATION_BIN_HEADER error.");
        return false;
    }
    // check ENDIAN_FLAG
    if (header.ENDIAN_FLAG != ENDIAN_FLAG_VALUE) {
        LOG_ERROR("ENDIAN_FLAG in ANIMATION_BIN_HEADER error.");
        return false;
    }
    // check OBJECT_TYPE
    std::string_view ObjectFlag(header.OBJECT_TYPE, std::min((uint64_t)sizeof(header.OBJECT_TYPE),
                                                             (uint64_t)strlen(header.OBJECT_TYPE)));
    if (OBJECT_TYPE_CHOICES.count(ObjectFlag) == 0) {
        LOG_ERROR("OBJECT_TYPE in ANIMATION_BIN_HEADER error.");
        return false;
    }
    // check OFFSETs
    if (header.SECTION_TABLE_OFFSET < sizeof(ANIMATION_BIN_HEADER)) {
        LOG_ERROR("SECTION_TABLE_OFFSET in ANIMATION_BIN_HEADER error.");
        return false;
    }
    if (header.STRING_TABLE_OFFSET < sizeof(ANIMATION_BIN_HEADER)) {
        LOG_ERROR("SECTION_TABLE_OFFSET in ANIMATION_BIN_HEADER error.");
        return false;
    }
    return true;
}

// load string_table
// s should be at string_table offset
STRING_TABLE KeyframeAnimationLoader::LoadStringTable(std::istream& s) {
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
SECTION KeyframeAnimationLoader::LoadSection(std::istream& s) {
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

void KeyframeAnimationLoader::LoadSectionData(std::istream& s, const SECTION_ITEM& sc,
                                              KeyframeAnimation& obj) {
    // 开头16个为name
    ANIMATION_SECTION_DATA data_header;
    s.read(reinterpret_cast<char*>(&data_header), sizeof(data_header));
    obj_current_offset += static_cast<uint32_t>(s.gcount());

    AnimationChannel channel;
    channel.unique_id = data_header.objectID;
    if (obj.string_table.strings.size() > data_header.objectNameIndex)
        channel.node_name = obj.string_table.strings[data_header.objectNameIndex];

    float min_frame;
    float max_frame;
    for (int i = 0; i < sc.ATTRIBUTE_COUNT; ++i) {
        KeyframeSetting current_kfs;
        ANIMATION_TARGET_DATA data;
        s.read(reinterpret_cast<char*>(&data), sizeof(data));
        obj_current_offset += static_cast<uint32_t>(s.gcount());

        current_kfs.key_index_type = data.keyIndexType;
        current_kfs.key_value_type = data.keyValueType;
        current_kfs.target = data.targetType;
        // TODO(beanpliu): 这两个number代表什么？
        if (data.targetType == 9 || data.targetType == 12)
            if (obj.string_table.strings.size() > data.propertyStringIndex)
                current_kfs.target_name = obj.string_table.strings[data_header.objectNameIndex];

        uint32_t index_start_offset = obj_current_offset;
        // read index
        int32_t index_properties[4] = {0, 0};  // indexsize, indexcount, reserved, reserved
        if (data.keyIndexType == kKeyIndexType_FLOAT) {
            // old float data
            std::vector<float> data_read(data.keyCount, 0.0f);
            s.read(reinterpret_cast<char*>(data_read.data()), data_read.size() * sizeof(float));
            obj_current_offset += static_cast<uint32_t>(s.gcount());
            for (const auto& m : data_read) {
                current_kfs.frames.push_back(m);
            }
        } else if (data.keyIndexType == kKeyIndexType_MMD) {
            // mmd data
            s.read(reinterpret_cast<char*>(index_properties),
                   sizeof(index_properties) * sizeof(int32_t));
            obj_current_offset += static_cast<uint32_t>(s.gcount());

            std::vector<int32_t> data_read(data.keyCount, 0);
            s.read(reinterpret_cast<char*>(data_read.data()), data_read.size() * sizeof(int32_t));
            obj_current_offset += static_cast<uint32_t>(s.gcount());
            for (const auto& m : data_read) {
                current_kfs.frames.push_back(m);
            }
        }
        // read value
        int32_t value_properties[4] = {0, 0};  // datasize, datacount, reserved, reserved
        if (data.keyValueType == kKeyValueType_FLOAT) {
            assert(obj_current_offset == index_start_offset + data.animationDataSize / 2);
            // old float data
            std::vector<float> data_read(data.keyCount, 0.0f);
            s.read(reinterpret_cast<char*>(data_read.data()), data_read.size() * sizeof(float));
            obj_current_offset += static_cast<uint32_t>(s.gcount());
            for (const auto& m : data_read) {
                current_kfs.values.push_back(m);
            }
        } else if (data.keyValueType == kKeyValueType_FLOAT_BEZIER_MMD) {
            assert(obj_current_offset == index_start_offset + sizeof(index_properties) +
                                             index_properties[0] * index_properties[1]);
            // KEY_FLOAT_BEZIER_MMD
            s.read(reinterpret_cast<char*>(value_properties),
                   sizeof(value_properties) * sizeof(int32_t));
            obj_current_offset += static_cast<uint32_t>(s.gcount());
            KeyframeFloatDataValue value;
            for (int i = 0; i < value_properties[1]; ++i) {
                s.read(reinterpret_cast<char*>(&value), sizeof(value));
                obj_current_offset += static_cast<uint32_t>(s.gcount());
                current_kfs.values.push_back(value);
            }
        } else if (data.keyValueType == kKeyValueType_QUATERNION_BEZIER_MMD) {
            assert(obj_current_offset == index_start_offset + sizeof(index_properties) +
                                             index_properties[0] * index_properties[1]);
            // KEY_QUATERNION_BEZIER_MMD
            s.read(reinterpret_cast<char*>(value_properties),
                   sizeof(value_properties) * sizeof(int32_t));
            obj_current_offset += static_cast<uint32_t>(s.gcount());
            KeyframeQuatDataValue value;
            for (int i = 0; i < value_properties[1]; ++i) {
                s.read(reinterpret_cast<char*>(&value), sizeof(value));
                obj_current_offset += static_cast<uint32_t>(s.gcount());
                current_kfs.values.push_back(value);
            }
        }

        if (!current_kfs.frames.empty()) {
            // for checking purpose
            for (int i = 0; i < current_kfs.frames.size() - 1; ++i) {
                assert(current_kfs.frames[i] < current_kfs.frames[i + 1]);
            }
            float current_min_frame = current_kfs.frames.front();
            // if (current_kfs.frames.front().index() == 0) {
            //    current_min_frame = std::get<0>(current_kfs.frames.front());
            //} else {
            //    current_min_frame = std::get<1>(current_kfs.frames.front());
            //}
            float current_max_frame = current_kfs.frames.back();
            // if (current_kfs.frames.back().index() == 0) {
            //    current_max_frame = std::get<0>(current_kfs.frames.back());
            //} else {
            //    current_max_frame = std::get<1>(current_kfs.frames.back());
            //}
            if (i == 0) {
                min_frame = current_min_frame;
                max_frame = current_max_frame;
            } else {
                min_frame = std::min(current_min_frame, min_frame);
                max_frame = std::min(current_max_frame, max_frame);
            }
        }
        channel.kfs.push_back(std::move(current_kfs));
    }
    channel.start_frame = min_frame;
    channel.end_frame = max_frame;
    channel.duration = max_frame - min_frame;

    obj.animated_nodes.push_back(std::move(channel));
}

bool KeyframeAnimationLoader::CheckSection(const SECTION& section) {
    // nothing to check for animation
    return true;
}

void KeyframeAnimation::CalculateAfterLoading() {
    // information from header and string table
    version = header.VERSION;
    clip_id = header.CLIP_UNIQUEID;
    fps = header.FPS;
    if (string_table.strings.size() > header.OBJECT_NAME_INDEX) {
        clip_name = string_table.strings[header.OBJECT_NAME_INDEX];
        auto pos = clip_name.find_last_of('.');
        if (pos != std::string::npos) {
            clip_name.erase(clip_name.begin() + pos, clip_name.end());
        }
    }
    start_frame = std::max(header.START_FRAME, 0.0f);
    end_frame = std::max(header.END_FRAME, 0.0f);
    local_time = start_frame / fps;
    clamped_start_frame = start_frame;
    clamped_end_frame = end_frame;

    for (int i = 0; i < animated_nodes.size(); ++i) {
        if (i == 0) {
            actual_start_frame = animated_nodes.front().start_frame;
            actual_end_frame = animated_nodes.front().end_frame;
        } else {
            actual_start_frame = std::min(actual_start_frame, animated_nodes[i].start_frame);
            actual_end_frame = std::max(actual_end_frame, animated_nodes[i].end_frame);
        }
    }

    // after loading calculation
    clip_duration = actual_end_frame - actual_start_frame;
}

void KeyframeAnimation::LinkToObjectsAndProperties(const std::shared_ptr<Entity>& target_entity) {
    for (auto& m : animated_nodes) {
        pEntity target;
        if (m.unique_id != 0xffffffff) {
            m.node = target_entity->FindChildEntityByID(m.unique_id);
        } else {
            m.node = target_entity->FindChildEntityByName(m.node_name);
        }
        if (!m.node) {
            LOG_ERROR("animated node id: %lu name: %s is not found.", m.unique_id,
                      m.node_name.c_str());
        }
    }
}

void KeyframeAnimation::Update(float delta) {
    if (!enable) {
        delta = 0.0f;
    }
    AnimateTransformationV3();

    float start_time = start_frame / fps;
    if (enable_clamp) {
        start_time = clamped_start_frame / fps;
    }
    if (local_time <= start_time) {
        local_time = start_time;
        if (!clip_started || force_update) {
            clip_started = true;
            clip_ended = false;
            // OnStartCallback
        }
    }

    if (delta > 0.0 || force_update) {
        force_update = false;
        // OnUpdateCallback
    }
    local_time += delta * speed;

    float end_time = end_frame / fps;
    if (enable_clamp) {
        end_time = clamped_end_frame / fps;
    }
    if (local_time >= end_time) {
        local_time = end_time;
        if (!clip_ended) {
            clip_ended = true;
            // OnEndCallback
        }
    }
}

void KeyframeAnimation::AnimateTransformationV3() {
    for (const auto& animated_node : animated_nodes) {
        if (animated_node.node == nullptr) {
            continue;
        }
        if (animated_node.HasEulerRotation()) {
            // TODO(beanpliu): apply euler to quaterinion
        }
        int morphWeightIndex = 0;
        for (const auto& m : animated_node.kfs) {
            if (!m.frames.empty() && m.frames.size() == m.values.size()) {
                float frame_index = local_time * fps;
                // interpolate new value
                auto new_value = Interpolate(m, animated_node.duration, frame_index);

#define AssignToValue(val, src_variant)                         \
    assert(std::holds_alternative<decltype(val)>(src_variant)); \
    val = std::get<decltype(val)>(src_variant);

                // apply new value
                switch (m.target) {
                    case kTARGET_TYPE_SX:
                        AssignToValue(animated_node.node->scale.x, new_value);
                        break;
                    case kTARGET_TYPE_SY:
                        AssignToValue(animated_node.node->scale.y, new_value);
                        break;
                    case kTARGET_TYPE_SZ:
                        AssignToValue(animated_node.node->scale.z, new_value);
                        break;
                    case kTARGET_TYPE_TX:
                        AssignToValue(animated_node.node->translation.x, new_value);
                        break;
                    case kTARGET_TYPE_TY:
                        AssignToValue(animated_node.node->translation.y, new_value);
                        break;
                    case kTARGET_TYPE_TZ:
                        AssignToValue(animated_node.node->translation.z, new_value);
                        break;
                    case kTARGET_TYPE_RX:
                        AssignToValue(animated_node.node->rotation_euler.x, new_value);
                        animated_node.node->rotation_euler.x =
                            glm::radians(animated_node.node->rotation_euler.x);
                        animated_node.node->rotation_euler_mode = kEulerMode_ZYX;
                        break;
                    case kTARGET_TYPE_RY:
                        AssignToValue(animated_node.node->rotation_euler.y, new_value);
                        animated_node.node->rotation_euler.y =
                            glm::radians(animated_node.node->rotation_euler.y);
                        animated_node.node->rotation_euler_mode = kEulerMode_ZYX;
                        break;
                    case kTARGET_TYPE_RZ:
                        AssignToValue(animated_node.node->rotation_euler.z, new_value);
                        animated_node.node->rotation_euler.z =
                            glm::radians(animated_node.node->rotation_euler.z);
                        animated_node.node->rotation_euler_mode = kEulerMode_ZYX;
                        break;
                    case kTARGET_TYPE_RQ:
                        // 为啥是这样的？
                        AssignToValue(animated_node.node->rotation_quaternion, new_value);
                        break;
                    case kTARGET_TYPE_RQ_Y:
                        AssignToValue(animated_node.node->rotation_quaternion.y, new_value);
                        break;
                    case kTARGET_TYPE_RQ_Z:
                        AssignToValue(animated_node.node->rotation_quaternion.z, new_value);
                        break;
                    case kTARGET_TYPE_RQ_W:
                        AssignToValue(animated_node.node->rotation_quaternion.w, new_value);
                        break;
                    case kTARGET_TYPE_MORPH_WEIGHT: {
                        auto mfc = std::dynamic_pointer_cast<MeshFilterComponent>(
                            animated_node.node->GetComponent(kMeshFilter));
                        if (mfc) {
                            auto mesh = mfc->GetMesh();
                            if (mesh) {
                                float weight_value = 0.0f;
                                AssignToValue(weight_value, new_value);
                                mesh->SetMorphWeight(morphWeightIndex, weight_value);
                            }
                        }
                        ++morphWeightIndex;
                    } break;
                    case kTARGET_TYPE_VISIBILITY:
                        // nothing here
                        break;
                    default:
                        LOG_ERROR("unknown animation target type.");
                        break;
                }
#undef AssignToValue
            } else {
                LOG_ERROR("animation data is ill-formed.");
            }
        }
    }
}

void KeyframeAnimation::SetProgress(float val) {
    float total_frame = (end_frame - start_frame);
    if (enable_clamp) {
        val = std::clamp(val, clamped_start_frame / total_frame, clamped_start_frame / total_frame);
    } else {
        val = std::clamp(val, 0.0f, 1.0f);
    }
    local_time = (val * total_frame + start_frame) / fps;
    force_update = true;
    //clip_ended = val == 1.0f;
    //clip_started = val != 0.0f;
}

bool AnimationChannel::HasEulerRotation() const {
    for (const auto& m : kfs) {
        if (m.target == kTARGET_TYPE_RX || m.target == kTARGET_TYPE_RY ||
            m.target == kTARGET_TYPE_RZ) {
            return true;
        }
    }
    return false;
}
