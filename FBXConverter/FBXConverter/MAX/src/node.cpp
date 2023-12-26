#include "node.h"
#include "max_structure.h"
#include <memory>

namespace max {

std::unique_ptr<mValue> BezierFloatValueClassStrategy::GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) {
    if (IsType<BezierFloat>(class_info)) {
        if (HasSubPath(id_path, { 0x2501 })) {
            return std::make_unique<mFloatValue>();
        }
    }
    return nullptr;
}

std::unique_ptr<mValue> EditableMeshValueClassStrategy::GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) {
    if (IsType<EditableMesh>(class_info)) {
        if (HasStartPath(id_path, { 0x08fe, 0x0914 })) {
            return std::make_unique<mArrayValueWithHeader<int32_t, float>>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x0912 })) {
            return std::make_unique<mArrayValueWithHeader<int32_t, MeshFaceStructure>>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x0959 })) {
            // channel
            return std::make_unique<mUint32Value>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x2394 })) {
            // vt
            return std::make_unique<mArrayValueWithHeader<int32_t, float>>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x2396 })) {
            // vt_tri
            return std::make_unique<mArrayValueWithHeader<int32_t, uint32_t>>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x23a0, 0x0110 })) {
            // 法向量内容
            return std::make_unique<mArrayValueWithHeader<int32_t, float>>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x23a0, 0x0120 })) {
            // 后续索引个数
            return std::make_unique<mUint32Value>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x23a0, 0x0124 })) {
            // 当前面片索引
            return std::make_unique<mUint32Value>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x23a0, 0x0128 })) {
            // 当前面片三个点索引
            return std::make_unique<mArrayValueWithHeader<int32_t, uint32_t>>();
        }
    }
    return nullptr;
}


std::unique_ptr<mValue> EditablePolyValueClassStrategy::GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) {
    if (IsType<EditablePoly>(class_info)) {
        if (HasStartPath(id_path, { 0x08fe, 0x0100 })) {
            // points (0,x,y,z)
            return std::make_unique<mArrayValueWithHeader<int32_t, float>>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x010a })) {
            // edges (0, v1, v2)
            return std::make_unique<mArrayValueWithHeader<int32_t, uint32_t>>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x011a })) {
            // polygon faces
            return std::make_unique<mPolygonFaceStrctureValue>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x0124 })) {
            // channel
            return std::make_unique<mUint32Value>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x0128 })) {
            // vt
            return std::make_unique<mArrayValueWithHeader<int32_t, float>>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x012b })) {
            // vt_face
            return std::make_unique<mArrayValue<uint32_t>>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x0300, 0x0110 })) {
            // normal
            // 后续索引个数
            return std::make_unique<mArrayValueWithHeader<int32_t, float>>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x0300, 0x0120 })) {
            // 后续索引个数
            return std::make_unique<mUint32Value>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x0300, 0x0124 })) {
            // 当前面片索引
            return std::make_unique<mUint32Value>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x0300, 0x0128 })) {
            // 当前面片三个点索引
            return std::make_unique<mArrayValue<uint32_t>>();
        } else if (HasStartPath(id_path, { 0x08fe, 0x0310 })) {
            // 边相关 (n, v1, v2, v3, ...)[]
            return std::make_unique<mPolygonFaceEdgeGroupStrctureValue>();
        }
    }
    return nullptr;
}

std::unique_ptr<mValue> NodeValueClassStrategy::GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) {
    if (IsType<Node>(class_info) || IsType<RootNode>(class_info)) {
        if (HasStartPath(id_path, { 0x096a })) {
            // pivot translate
            return std::make_unique<mArrayValue<float>>();
        } else if (HasStartPath(id_path, { 0x096b })) {
            // pivot rotation
            return std::make_unique<mArrayValue<float>>();
        } else if (HasStartPath(id_path, { 0x096c })) {
            // scale
            return std::make_unique<mArrayValue<float>>();
        }
    }
    return nullptr;
}

std::unique_ptr<mValue> StandardAndColorMapClassStrategy::GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) {
    // 重叠部分：
    if (IsType<StandardMaterial>(class_info) || IsType<ColorMap>(class_info) || IsType<Bitmap>(class_info)) {
        if (HasStartPath(id_path, { 0x4000, 0x4001 })) {
            // name
            return std::make_unique<mUCSStringValue>();
        } else if (HasStartPath(id_path, { 0x4000, 0x4210 })) {
            // 0x4200 的图边长 n
            return std::make_unique<mUint32Value>();
        } else if (HasStartPath(id_path, { 0x4000, 0x4200 })) {
            // 长度为 ((n * 3) + 2) * n
            return std::make_unique<mPrintLessUint8Array>();
        } else if (HasStartPath(id_path, { 0x4000, 0x4030 })) {
            // float[4] 含义不明
            return std::make_unique<mArrayValue<float>>();
        }
    }
    return nullptr;
}

std::unique_ptr<mValue> ParamBlock2ClassStrategy::GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry& class_info) {
    // 重叠部分：
    if (IsType<ParamBlock2>(class_info)) {
        if (HasStartPath(id_path, { 0x100e })) {
            // ParamBlockValues
            return std::make_unique<mParamBlockValue>();
        } else if (HasStartPath(id_path, { 0x0009 }) || HasStartPath(id_path, { 0x000b })) {
            // ?
            return std::make_unique <mArrayValue<int16_t>>();
        }
    }
    return nullptr;
}

std::unique_ptr<mValue> BaseLayerClassStrategy::GenerateByClass(const std::list<uint16_t>& id_path, uint16_t class_id, const ClassEntry & class_info) {
    if (IsType<BaseLayer>(class_info)) {
        if (HasStartPath(id_path, { 0x1010 })) {
            // ParamBlockValues
            return std::make_unique<mStringValue>();
        } else if (HasStartPath(id_path, { 0x1020 })) {
            // flag (0x00100000 为 hide 相关属性)
            return std::make_unique<mUint32Value>();
        }
    }
    return nullptr;
}

bool FlagInterpreter::Process(const std::vector<uint16_t>& flags) {
    // 00010000b: 未知，占用两个 int16_t，位于结尾
    // 00001000b: submesh index, 若没有则表示 第0个 submesh， 若有，则某一个值表示 submesh index(starts from 0)，位于上一个之前
    // 00000001b: 未知，占用两个 int16_t，位于开头
    if (flags.empty()) {
        return false;
    }
    int current_pos = flags.size();
    if (flags.front() & 0x0010) {
        assert(current_pos >= 3);
        current_pos -= 2;
        has_smooth_group = true;
        smooth_group = *((uint32_t*)&(flags[current_pos]));
    }
    if (flags.front() & 0x0008) {
        assert(current_pos >= 2);
        current_pos -= 1;
        has_smooth_group = true;
        submesh_index = flags[current_pos];
    }
    if (flags.front() & 0x0001) {
        assert(current_pos >= 3);
        current_pos -= 2;
        has_unknown2 = true;
        unknown2 = *((uint32_t*)&(flags[current_pos]));
    }
    assert(current_pos == 1);
    return true;
}
}// max