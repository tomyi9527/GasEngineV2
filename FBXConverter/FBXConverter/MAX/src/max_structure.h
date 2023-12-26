#pragma once
#include <memory>
#include <vector>
#include <list>
#include <set>
#include <functional>
#include <iostream>
#include <map>
#include <array>
#include "node.h"
#include "bmp.h"
#include "error_printer.h"

#define _USE_MATH_DEFINES 
#include <math.h>

namespace max {

class SceneGraphNode : public std::enable_shared_from_this<SceneGraphNode> {
public:
    std::vector<std::shared_ptr<SceneGraphNode>> children;
    std::map<int32_t, std::shared_ptr<SceneGraphNode>> components;
    std::shared_ptr<SceneGraphNode> parent;
    const mNode* data_ptr;
    int32_t scene_values_index;
    ClassEntry classinfo;
    bool children_ready = false;

    bool is_component = false;
    std::set<std::shared_ptr<SceneGraphNode>> component_owner;

    void* user_ptr = nullptr;

    void Clear() {
        for (auto& m : children) {
            m->Clear();
        }
        children.clear();
        parent.reset();
        data_ptr = nullptr;
        scene_values_index = -1;
        return;
    }

    void SetUserDataPtr(void* ptr) {
        user_ptr = ptr;
    }
    void* GetUserDataPtr() const {
        return user_ptr;
    }

    void AddChild(const std::shared_ptr<SceneGraphNode>& item, bool allow_node_reuse = true);

    void RemoveChild(const std::shared_ptr<SceneGraphNode>& item);

    std::shared_ptr<SceneGraphNode> GetPtr() {
        return shared_from_this();
    }

    template<typename Pred>
    std::shared_ptr<SceneGraphNode> GetFirstChild(Pred pred) const {
        for (const auto& m : children) {
            if (pred(m)) {
                return m;
            }
        }
        return nullptr;
    }

    std::shared_ptr<SceneGraphNode> GetFirstChildByClassID(uint16_t id) const {
        return GetFirstChild([id](const std::shared_ptr<SceneGraphNode>& m) {
            return m->data_ptr->id == id;
        });
    }

    template<typename _3DSMAXClass>
    std::shared_ptr<SceneGraphNode> GetFirstChildByClassType() const;

    void AddComponent(int32_t type, const std::shared_ptr<SceneGraphNode>& item);

    void RemoveComponent(int32_t type);

    std::string PrintSceneNodes(int indent, const std::function<bool(const std::shared_ptr<SceneGraphNode>& ptr)>& pred_recursive_children) {
        return PrintSceneNodesRecursive(indent, pred_recursive_children(GetPtr()));
    }

    std::string PrintSceneNodesRecursive(int indent, bool recursive_children);

    static std::shared_ptr<SceneGraphNode> CreateNode(const mNode* node, int32_t index_of_scene_values) {
        auto ret = std::make_shared<SceneGraphNode>();
        ret->data_ptr = node;
        ret->scene_values_index = index_of_scene_values;
        return ret;
    }
};


template<typename _3DSMaxType>
inline bool IsNodeType_ClassID(const std::shared_ptr<SceneGraphNode>& node) {
    if (!node)
        return false;
    return IsType_ClassID<_3DSMaxType>(node->classinfo);
}

template<typename _3DSMaxType>
inline bool IsNodeType_DllName(const std::shared_ptr<SceneGraphNode>& node) {
    if (!node)
        return false;
    return IsType_DllName<_3DSMaxType>(node->classinfo);
}

template<typename _3DSMaxType>
inline bool IsNodeType_EnglishClassName(const std::shared_ptr<SceneGraphNode>& node) {
    if (!node)
        return false;
    return IsType_EnglishClassName<_3DSMaxType>(node->classinfo);
}

template<typename _3DSMaxType>
inline bool IsNodeType(const std::shared_ptr<SceneGraphNode>& node) {
    if (!node)
        return false;
    return IsType<_3DSMaxType>(node->classinfo);
}

template<typename _3DSMaxType>
inline void NodeTypeCheck(const std::shared_ptr<SceneGraphNode>& node) {
    if (!IsNodeType<_3DSMaxType>(node)) {
        std::string node_name = node ? node->classinfo.name : "";
        std::string expected_name = _3DSMaxType::english_class_name;
        PrintError(MaxClassImplError, LEVEL_ERROR, "node type should be [" + expected_name + "], but it's [" + node_name + "]");
    }
}


template<typename _3DSMAXClass>
inline std::shared_ptr<SceneGraphNode> SceneGraphNode::GetFirstChildByClassType() const {
    return GetFirstChild([](const std::shared_ptr<SceneGraphNode>& m) {
        return IsNodeType<_3DSMAXClass>(m);
    });
}


class SceneNodeRecord {
public:
    static std::set<std::shared_ptr<SceneGraphNode>>& NodeList() {
        static std::set<std::shared_ptr<SceneGraphNode>> instance;
        return instance;
    }
    static void AddNodeRecord(const std::shared_ptr<SceneGraphNode>& item);

    static std::set<std::shared_ptr<SceneGraphNode>>& ComponentsList() {
        static std::set<std::shared_ptr<SceneGraphNode>> instance;
        return instance;
    }
    static void AddComponentRecord(const std::shared_ptr<SceneGraphNode>& item);

    static std::set<std::shared_ptr<SceneGraphNode>>& UnusedList() {
        static std::set<std::shared_ptr<SceneGraphNode>> instance;
        return instance;
    }
    static void AddUnusedNodeRecord(const std::shared_ptr<SceneGraphNode>& item);

    static void Print(std::ostream& s, const std::set<std::shared_ptr<SceneGraphNode>>& target);
    static void PrintComponents(std::ostream& s);
    static void ClearAll();
};


inline void TraversalNodes(const std::shared_ptr<SceneGraphNode>& node, const std::function<void(const std::shared_ptr<SceneGraphNode>&)>& operation, std::list<std::shared_ptr<SceneGraphNode>>& parent_nodes) {
    if (node) {
        parent_nodes.push_back(node);
        operation(node);
        for (const auto& m : node->children) {
            auto it = std::find(parent_nodes.begin(), parent_nodes.end(), m);
            if (it == parent_nodes.end())
                TraversalNodes(m, operation, parent_nodes);
            else {
                std::stringstream ss;
                ss << "inner loop detected, will break recursive visit here, current path is: ";
                for (const auto& o : parent_nodes) {
                    ss << o->scene_values_index << ", ";
                }
                ss << m->scene_values_index;
                PrintError(MaxClassImplError, LEVEL_WARNING, ss.str());
            }
        }
        parent_nodes.pop_back();
    }
}

inline void TraversalNodes(const std::shared_ptr<SceneGraphNode>& node, const std::function<void(const std::shared_ptr<SceneGraphNode>&)>& operation) {
    std::list<std::shared_ptr<SceneGraphNode>> parent_nodes;
    TraversalNodes(node, operation, parent_nodes);
}

inline void TraversalNodesAndComponents(const std::shared_ptr<SceneGraphNode>& node, const std::function<void(const std::shared_ptr<SceneGraphNode>&)>& operation, std::list<std::shared_ptr<SceneGraphNode>>& parent_nodes) {
    if (node) {
        parent_nodes.push_back(node);
        operation(node);
        for (const auto& m : node->children) {
            auto it = std::find(parent_nodes.begin(), parent_nodes.end(), m);
            if (it == parent_nodes.end())
                TraversalNodesAndComponents(m, operation, parent_nodes);
            else {
                //std::cerr << "inner loop detected, current path is: ";
                //for (const auto& o : parent_nodes) {
                //    std::cerr << o->scene_values_index << ", ";
                //}
                //std::cerr << m->scene_values_index << std::endl;
            }
        }
        for (const auto& m : node->components) {
            auto it = std::find(parent_nodes.begin(), parent_nodes.end(), m.second);
            if (it == parent_nodes.end())
                TraversalNodesAndComponents(m.second, operation, parent_nodes);
            else {
                //std::cerr << "inner loop detected, current path is: ";
                //for (const auto& o : parent_nodes) {
                //    std::cerr << o->scene_values_index << ", ";
                //}
                //std::cerr << m.second->scene_values_index << std::endl;
            }
        }
        parent_nodes.pop_back();
    }
}

inline void TraversalNodesAndComponents(const std::shared_ptr<SceneGraphNode>& node, const std::function<void(const std::shared_ptr<SceneGraphNode>&)>& operation) {
    std::list<std::shared_ptr<SceneGraphNode>> parent_nodes;
    TraversalNodesAndComponents(node, operation, parent_nodes);
}

inline static std::shared_ptr<SceneGraphNode> GetNodeFromParent(const std::shared_ptr<SceneGraphNode>& node, const std::function<bool(const std::shared_ptr<SceneGraphNode>&)>& pred) {
    // 同时考虑componentParent
    if (!node) {
        return nullptr;
    }
    if (pred(node)) {
        return node;
    }
    std::shared_ptr<SceneGraphNode> ret = nullptr;
    if (node->parent) {
        ret = GetNodeFromParent(node->parent, pred);
        if (ret) return ret;
    }
    for (const auto& m : node->component_owner) {
        ret = GetNodeFromParent(m, pred);
        if (ret) return ret;
    }
    return nullptr;
}

class Node {
public:
    int32_t id;
    std::string name;

    std::shared_ptr<SceneGraphNode> bone_node;
    std::shared_ptr<SceneGraphNode> mesh_node; 
    std::shared_ptr<SceneGraphNode> material_node;
    std::shared_ptr<SceneGraphNode> sRT_node;
    std::shared_ptr<SceneGraphNode> layer_node;

    std::array<float, 3> pivot_position = { 0.0f, 0.0f, 0.0f };
    std::array<float, 4> pivot_rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
    std::array<float, 3> pivot_scale = { 1.0f, 1.0f, 1.0f };
    std::array<float, 4> pivot_scale_quat_axis = { 0.0f, 0.0f, 0.0f, 1.0f };
    bool has_pivot_transform = false;

    constexpr static const char english_class_name[] = "Node";
    constexpr static const char class_dll[] = "";
    constexpr static const uint32_t class_id[3] = { 0x1, 0x0, 0x1 };

    uint32_t flag = 0;

    bool IsHide() const {
        return flag & (1 << 20); // 同layer的判断方式
    }

    static Node CreateFrom(const std::shared_ptr<SceneGraphNode>& node);
};

class RootNode : public Node {
public:
    constexpr static const char english_class_name[] = "RootNode";
    constexpr static const char class_dll[] = "";
    constexpr static const uint32_t class_id[3] = { 0x2, 0x0, 0x1 };

    static RootNode CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
        RootNode ret;
        *(Node*)&ret = Node::CreateFrom(node);
        return ret;
    }
};

class EditablePoly {
public:
    int32_t id = -1;
    int32_t parent_id = -1;
    struct coordinate {
        float reserved = 0.0;
        float x = 0.0;
        float y = 0.0;
        float z = 0.0;
    };
    struct edge {
        uint32_t reserved = 0;
        uint32_t v1 = 0;
        uint32_t v2 = 0;
    };
    struct face {
        std::vector<uint32_t> index;
        std::vector<std::pair<uint32_t, uint32_t>> inner_edge;
        uint32_t smooth_group = 0;
        uint16_t submesh_index = 0;
    };
    std::string mesh_name;
    std::vector<coordinate> points;
    std::vector<edge> edges;
    std::vector<face> faces;
    struct MapChannel {
        std::vector<coordinate> values;
        std::vector<face> faces;
    };
    std::map<int, MapChannel> channels; // id -> channel
    std::vector<coordinate> normal_points;
    std::vector<face> normal_faces;

    std::array<std::array<float, 3>, 2> GetBBox() const;

    constexpr static const char english_class_name[] = "Editable Poly";
    constexpr static const char class_dll[] = "epoly.dlo";
    constexpr static const uint32_t class_id[3] = { 0x1bf8338d, 0x192f6098, 0x10 };

    std::string ExportAsObj() const;

    static EditablePoly CreateFrom(const std::shared_ptr<SceneGraphNode>& node);
};

class EditableMesh {
public:
    struct coordinate {
        float x = 0.0;
        float y = 0.0;
        float z = 0.0;
    };
    struct triangle {
        uint32_t v1 = 0;
        uint32_t v2 = 0;
        uint32_t v3 = 0;
        uint32_t smooth_group = 0;
        uint16_t submesh_index = 0;
    };
    int32_t id = -1;
    int32_t parent_id = -1;
    std::string mesh_name = "unknown";
    std::vector<coordinate> points;
    std::vector<triangle> triangles;
    struct MapChannel {
        std::vector<coordinate> values;
        std::vector<triangle> triangles;
    };
    std::map<int, MapChannel> channels; // id -> channel
    std::vector<coordinate> normal_points;
    std::vector<triangle> normal_triangles;

    std::array<std::array<float, 3>, 2> GetBBox() const;

    const MapChannel* GetVertexColorChannel() const {
        auto it = channels.find(0); // 0 means vc channel
        if (it != channels.end()) {
            return &(it->second);
        } else {
            return nullptr;
        }
    }

    const MapChannel* GetMapChannel(int channel) const {
        if (channel <= 0) {
            return nullptr;
        }// channel > 0 is map
        auto it = channels.find(channel);
        if (it != channels.end()) {
            return &(it->second);
        } else {
            return nullptr;
        }
    }

    const std::vector<coordinate> GenerateVertexColorByVCChannel() const {
        auto vc = GetVertexColorChannel();
        if (!vc || vc->triangles.size() != triangles.size()) {
            return {};
        }
        std::vector<coordinate> rgb_float(points.size());
        for (int i = 0; i < triangles.size(); ++i) {
            rgb_float[triangles[i].v1] = vc->values[vc->triangles[i].v1];
            rgb_float[triangles[i].v2] = vc->values[vc->triangles[i].v2];
            rgb_float[triangles[i].v3] = vc->values[vc->triangles[i].v3];
        }
        return rgb_float;
    }

    constexpr static const char english_class_name[] = "Editable Mesh";
    constexpr static const char class_dll[] = "update1.dlo";
    constexpr static const uint32_t class_id[3] = { 0xe44f10b3, 0x0, 0x10 };

    std::string ExportAsObj() const;

    static EditableMesh CreateFrom(const std::shared_ptr<SceneGraphNode>& node);
};

// sRT
class BezierFloat {
public:
    int32_t id = -1;
    float value = 0.0;
    constexpr static const char english_class_name[] = "Bezier Float";
    constexpr static const char class_dll[] = "";
    constexpr static const uint32_t class_id[3] = { 0x2007, 0x0, 0x9003 };

    static BezierFloat CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
        BezierFloat ret;
        if (!node) {
            return ret;
        }

        NodeTypeCheck<BezierFloat>(node);
        
        ret.id = node->scene_values_index;
        const auto* node_data = node->data_ptr;
        if (node_data->type == mNode::Container) {
            auto node_1 = ((mContainer*)node_data)->GetFirstContentByID(0x7127);
            if (node_1 && node_1->type == mNode::Container) {
                auto node_2 = ((mContainer*)node_1)->GetFirstContentByID(0x2501);
                if (node_2 && node_2->type == mNode::Value) {
                    ret.value = ((mFloatValue*)node_2)->GetValue();
                }
            } else {
                auto node_2 = ((mContainer*)node_data)->GetFirstContentByID(0x2501);
                if (node_2 && node_2->type == mNode::Value) {
                    ret.value = ((mFloatValue*)node_2)->GetValue();
                }
            }
        }
        if (isinf(ret.value) || isnan(ret.value)) {
            PrintError(MaxClassImplError, LEVEL_FATAL, "parsed bezier float value is nan or inf, will set to 0.0");
            ret.value = 0.0;
        }
        return ret;
    }
};

class PositionXYZ {
public:
    int32_t id = -1;
    std::array<BezierFloat, 3> position;
    constexpr static const char english_class_name[] = "Position XYZ";
    constexpr static const char class_dll[] = "ctrl.dlc";
    constexpr static const uint32_t class_id[3] = { 0x118f7e02, 0xffee238a, 0x900b };

    std::array<float, 3> GetValue() const {
        std::array<float, 3> ret;
        for (int i = 0; i < position.size(); ++i) {
            ret[i] = position[i].value;
        }
        return ret;
    }

    static PositionXYZ CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
        PositionXYZ ret;
        if (!node) {
            return ret;
        }
        ret.id = node->scene_values_index;
        NodeTypeCheck<PositionXYZ>(node);

        if (node->children.size() != 3) {
            PrintError(MaxClassImplError, LEVEL_ERROR, "children size of PositionXYZ is not 3, but " + std::to_string(node->children.size()));
        }
        for (int i = 0; i < node->children.size() && i < 3; ++i) {
            ret.position[i] = BezierFloat::CreateFrom(node->children[i]);
        }
        return ret;
    }
};

class EulerXYZ {
public:
    int32_t id = -1;
    std::array<BezierFloat, 3> euler; // radius
    constexpr static const char english_class_name[] = "Euler XYZ";
    constexpr static const char class_dll[] = "ctrl.dlc";
    constexpr static const uint32_t class_id[3] = { 0x2012, 0x0, 0x900c };

    std::array<float, 3> GetRadiusValue() const {
        std::array<float, 3> ret;
        for (int i = 0; i < euler.size(); ++i) {
            ret[i] = euler[i].value;
        }
        return ret;
    }

    std::array<float, 3> GetDegreeValue() const {
        std::array<float, 3> ret;
        for (int i = 0; i < euler.size(); ++i) {
            ret[i] = euler[i].value * 180.0f / M_PI;
        }
        return ret;
    }

    void RegularizeTo2PI() {
        // 转换为 0 - 2PI范围
        for (auto& m : euler) {
            while (m.value < 0) {
                m.value += 2 * M_PI;
            }
            while (m.value >= 2 * M_PI) {
                m.value -= 2 * M_PI;
            }
        }
    }

    static EulerXYZ CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
        EulerXYZ ret;
        if (!node) {
            return ret;
        }
        ret.id = node->scene_values_index;
        NodeTypeCheck<EulerXYZ>(node);

        if (node->children.size() != 3) {
            PrintError(MaxClassImplError, LEVEL_ERROR, "children size of EulerXYZ is not 3, but " + std::to_string(node->children.size()));
        }
        for (int i = 0; i < node->children.size() && i < 3; ++i) {
            ret.euler[i] = BezierFloat::CreateFrom(node->children[i]);
        }
        ret.RegularizeTo2PI();
        return ret;
    }
};

class BezierScale {
public:
    int32_t id = -1;
    std::array<float, 3> scale = {1.0f, 1.0f, 1.0f};
    std::array<float, 4> quat_axis = { 0.0f, 0.0f, 0.0f, 1.0f };
    constexpr static const char english_class_name[] = "Bezier Scale";
    constexpr static const char class_dll[] = "";
    constexpr static const uint32_t class_id[3] = { 0x2010, 0x0, 0x900d };

    std::array<float, 3> GetFactor() const {
        return scale;
    }
    std::array<float, 4> GetAxis() const {
        return quat_axis;
    }
    std::array<float, 7> GetValue() const {
        std::array<float, 7> ret;
        std::copy(scale.begin(), scale.end(), ret.begin());
        std::copy(quat_axis.begin(), quat_axis.end(), ret.begin() + scale.size());
        return ret;
    }

    static BezierScale CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
        BezierScale ret;
        if (!node) {
            return ret;
        }
        ret.id = node->scene_values_index;
        NodeTypeCheck<BezierScale>(node);

        const auto* node_data = node->data_ptr;
        if (node_data->type == mNode::Container) {
            auto node_1 = ((mContainer*)node_data)->GetFirstContentByID(0x2505);
            if (node_1 && node_1->type == mNode::Value) {
                auto values = ((mArrayValue<float>*)node_1)->GetValue();
                if (values.size() >= 3) {
                    ret.scale[0] = values[0];
                    ret.scale[1] = values[1];
                    ret.scale[2] = values[2];
                    if (values.size() >= 7) {
                        ret.quat_axis[0] = values[3];
                        ret.quat_axis[1] = values[4];
                        ret.quat_axis[2] = values[5];
                        ret.quat_axis[3] = values[6];
                    }
                }
            }
        }
        for (int i = 0; i < 3; ++i) {
            if (isinf(ret.scale[i]) || isnan(ret.scale[i])) {
                PrintError(MaxClassImplError, LEVEL_FATAL, "parsed bezier float value is nan or inf, will set to 0.0");
                ret.scale[i] = 0.0;
            }
        }
        for (int i = 0; i < 4; ++i) {
            if (isinf(ret.quat_axis[i]) || isnan(ret.quat_axis[i])) {
                PrintError(MaxClassImplError, LEVEL_FATAL, "parsed bezier float value is nan or inf, will set to 0.0");
                ret.quat_axis[i] = 0.0;
            }
        }
        return ret;
    }
};

class PositionRotationScale {
public:
    int32_t id = -1;
    PositionXYZ position;
    EulerXYZ rotation;
    BezierScale scale;
    constexpr static const char english_class_name[] = "Position/Rotation/Scale";
    constexpr static const char class_dll[] = "";
    constexpr static const uint32_t class_id[3] = { 0x2005, 0x0, 0x9008 };

    static PositionRotationScale CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
        PositionRotationScale ret;
        if (!node) {
            return ret;
        }
        ret.id = node->scene_values_index;
        NodeTypeCheck<PositionRotationScale>(node);

        if (node->children.size() != 3) {
            PrintError(MaxClassImplError, LEVEL_ERROR, "children size of Position/Rotation/Scale is not 3, but " + std::to_string(node->children.size()));
        }
        for (int i = 0; i < node->children.size(); ++i) {
            const auto& child_node = node->children[i];
            if (IsNodeType<PositionXYZ>(child_node)) {
                ret.position = PositionXYZ::CreateFrom(child_node);
            } else if (IsNodeType<EulerXYZ>(child_node)) {
                ret.rotation = EulerXYZ::CreateFrom(child_node);
            } else if (IsNodeType<BezierScale>(child_node)) {
                ret.scale = BezierScale::CreateFrom(child_node);
            }
        }
        return ret;
    }
};


class MaterialMapThumbnail {
public:
    class Image {
    public:
        struct pixel {
            uint8_t red = 0;
            uint8_t green = 0;
            uint8_t blue = 0;
        };
        std::vector<pixel> data;
        int width = 0;
        int height = 0;

        void ExportAsBmp(const std::string& path) const {
            if (width == 0 && height == 0) {
                PrintError(MaxClassImplError, LEVEL_WARNING, "image is empty, will not export to " + path);
            } else {
                BMP image(width, height);
                image.FromRGB(width, height, reinterpret_cast<const char*>(data.data()));
                image.save(path);
            }
        }
    };

public:
    Image image;
    std::string name;
    
    static MaterialMapThumbnail CreateFromDataNode(const mNode* node) {
        MaterialMapThumbnail ret;
        if (!node) {
            return ret;
        }
        if (node->type == mNode::Container && node->id == 0x4000) {
            auto node_1 = ((mContainer*)node)->GetFirstContentByID(0x4001);
            if (node_1 && node_1->type == mNode::Value) {
                ret.name = ((mUCSStringValue*)node_1)->GetValue();
            }
            auto node_2 = ((mContainer*)node)->GetFirstContentByID(0x4210);
            if (node_2 && node_2->type == mNode::Value) {
                ret.image.width = ret.image.height = ((mUint32Value*)node_2)->GetValue();
                ret.image.data.resize(ret.image.width * ret.image.height);
            }
            auto node_3 = ((mContainer*)node)->GetFirstContentByID(0x4200);
            if (node_3 && node_3->type == mNode::Value) {
                const auto& content = ((mArrayValue<uint8_t>*)node_3)->GetValue();
                // length is ((n * 3) + 2) * n
                if ((ret.image.width * 3 + 2) * ret.image.height == content.size()) {
                    for (int row = 0; row < ret.image.width; ++row) {
                        for (int col = 0; col < ret.image.height; ++col) {
                            int content_offset = (ret.image.width * 3 + 2) * row + 3 * col;
                            int data_offset = ((ret.image.height - 1 - row) * ret.image.width + col);
                            ret.image.data[data_offset].blue = content[content_offset];
                            ret.image.data[data_offset].green = content[content_offset + 1];
                            ret.image.data[data_offset].red = content[content_offset + 2];
                        }
                    }
                } else if (ret.image.width * ret.image.height * 3 == content.size()) {
                    for (int row = 0; row < ret.image.width; ++row) {
                        for (int col = 0; col < ret.image.height; ++col) {
                            int content_offset = (row * ret.image.width + col) * 3;
                            int data_offset = ((ret.image.height - 1 - row) * ret.image.width + col);
                            ret.image.data[data_offset].blue = content[content_offset];
                            ret.image.data[data_offset].green = content[content_offset + 1];
                            ret.image.data[data_offset].red = content[content_offset + 2];
                        }
                    }
                }
            }
        }
        return ret;
    }
};

enum MapType {
    MapType_Invalid = -1,
    MapType_ColorMap = 0,
    MapType_Bitmap = 1,
};

class MapBase {
public:
    int32_t id = -1;
    MaterialMapThumbnail thumbnail; // thumbnail
    MapType type = MapType_Invalid;

    virtual std::string GetImageFile() const {
        return std::string();
    }

    // factory
    static std::shared_ptr<MapBase> GenerateFrom(const std::shared_ptr<SceneGraphNode>& node);

protected:
    static MapBase CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
        MapBase ret;
        if (!node) {
            return ret;
        }
        ret.id = node->scene_values_index;
        const auto* node_data = node->data_ptr;
        if (node_data->type == mNode::Container) {
            auto node_1 = ((mContainer*)node_data)->GetFirstContentByID(0x4000);
            if (node_1 && node_1->type == mNode::Container) {
                ret.thumbnail = MaterialMapThumbnail::CreateFromDataNode(node_1);
            }
        }
        return ret;
    }
};

class ColorMap : public MapBase {
public:
    // param block
    std::array<float, 4> solid_color = {1.0, 1.0, 1.0, 1.0};  // 0/0
    bool has_map = false; // 0/2
    int32_t map_index; // 0/1
    float gain = 1.0; // 0/3
    float gamma = 1.0; // 0/4
    bool reverse_gamma = false; // 0/5
    std::shared_ptr<MapBase> map;

    constexpr static const char english_class_name[] = "ColorMap";
    constexpr static const char class_dll[] = "colormap.dlt";
    constexpr static const uint32_t class_id[3] = { 0x139f22c6, 0x13f6a914, 0xc10 };

    std::string GetImageFile() const override {
        if (map)
            return map->GetImageFile();
        else
            return std::string();
    }

    ColorMap() {
        type = MapType_ColorMap;
    }

    static ColorMap CreateFrom(const std::shared_ptr<SceneGraphNode>& node);
};

class Bitmap : public MapBase {
public:
    int32_t id = -1;
    MaterialMapThumbnail map; // thumbnail

    std::string bitmap_filetype;
    std::string bitmap_assetid;

    constexpr static const char english_class_name[] = "Bitmap";
    constexpr static const char class_dll[] = "mtl.dlt";
    constexpr static const uint32_t class_id[3] = { 0x240, 0x0, 0xc10 };

    std::string GetImageFile() const override {
        const auto& result = Linker::GetFileAssetEntryByID(bitmap_assetid);
        return result.path1;
    }

    Bitmap() {
        type = MapType_Bitmap;
    }

    static Bitmap CreateFrom(const std::shared_ptr<SceneGraphNode>& node);
};


class ParamBlock2 {
public:
    int32_t id = -1;
    int32_t parent_index = -1;
    int32_t child_index_in_parent = -1;
    uint16_t count = 0;
    std::vector<mParamBlockValue*> value_nodes;

    constexpr static const char english_class_name[] = "ParamBlock2";
    constexpr static const char class_dll[] = "";
    constexpr static const uint32_t class_id[3] = { 0x82, 0x0, 0x82 };

    static ParamBlock2 CreateFrom(const std::shared_ptr<SceneGraphNode>& node);
};


// other type is not supported currently
enum ShaderType {
    ShaderType_Invalid = -1,
    ShaderType_Blinn = 1,
    ShaderType_Metal = 2,
    ShaderType_Phong = 5,
    // type 值是 3dsmax 界面里的 index
};
inline constexpr const char* GetShaderTypeName(ShaderType type) {
    switch (type) {
    case ShaderType_Blinn:
        return "Blinn";
    case ShaderType_Metal:
        return "Metal";
    case ShaderType_Phong:
        return "Phong";
    default:
        return "";
    }
}
class ShaderBase {
public:
    int32_t id = -1;
    ShaderType shader_type = ShaderType_Invalid;

    // index in blinn
    std::array<float, 3> ambient_color = { 0.7f, 0.7f, 0.7f }; // 0/0 as RGB float
    std::array<float, 3> diffuse_color = { 0.7f, 0.7f, 0.7f }; // 0/1 as RGB float
    std::array<float, 3> specular_color = { 0.7f, 0.7f, 0.7f }; // 0/2 as RGB float
    bool map_lock = false; // 0/3 as bool
    bool ambient_diffuse_bind = false; // 0/4 as bool
    bool diffuse_specular_bind = false; // 0/5 as bool
    bool self_illumination_is_color = false; // 0/6 as bool
    float self_illumination_percent = 0.0f; // 0/7 as float
    std::array<float, 3> self_illumination_color = { 0.7f, 0.7f, 0.7f }; // 0/8 as RGB float
    float specular_level = 0.0f; // 0/9 as float
    float glossiness = 0.6f; // 0/10 as float
    float soften = 0.0f; // 0/11 as float

    ShaderBase(ShaderType type) : shader_type(type) { }
    ShaderType GetShaderType() const { return shader_type; }

    virtual std::vector<std::string> GetMapList() const = 0;
    virtual int GetAmbientMapIndex() const = 0;
    virtual int GetDiffuseMapIndex() const = 0;
    virtual int GetSpecularMapIndex() const = 0;
    virtual int GetGlossinessMapIndex() const = 0;
    virtual int GetOpacityMapIndex() const = 0;
};
class Blinn : public ShaderBase {
public:
    int32_t id = -1;

    constexpr static const ShaderType s_shader_type = ShaderType_Blinn;
    constexpr static const char english_class_name[] = "Blinn";
    constexpr static const char class_dll[] = "mtl.dlt";
    constexpr static const uint32_t class_id[3] = { 0x38, 0x0, 0x10b0 };

    constexpr static const char* map_names[] = {
        "Ambient Color", 
        "Diffuse Color", 
        "Specular Color", 
        "Specular Level", 
        "Glossiness", 
        "Self-Illumination", 
        "Opacity", 
        "Filter Color", 
        "Bump", 
        "Reflection", 
        "Refraction", 
        "Displacement"
    };
    std::vector<std::string> GetMapList() const override {
        static std::vector<std::string> ret(std::begin(map_names), std::end(map_names));
        return ret;
    }
    int GetAmbientMapIndex() const {
        return 0;
    }
    int GetDiffuseMapIndex() const {
        return 1;
    }
    int GetSpecularMapIndex() const {
        return 2;
    }
    int GetGlossinessMapIndex() const {
        return 4;
    }
    int GetOpacityMapIndex() const {
        return 6;
    }
    Blinn(): ShaderBase(s_shader_type) {
        const static std::array<float, 3> default_rgb = { 1.0f, 1.0f, 1.0f };
        ambient_color = default_rgb;
        diffuse_color = default_rgb;
        specular_color = default_rgb;
        self_illumination_color = default_rgb;
    }

    static Blinn CreateFrom(const std::shared_ptr<SceneGraphNode>& node);

protected:
    std::vector<ParamBlock2> param_blocks;
};

class Phong : public Blinn {
public:
    constexpr static const ShaderType s_shader_type = ShaderType_Phong;
    constexpr static const char english_class_name[] = "Phong";
    constexpr static const char class_dll[] = "mtl.dlt";
    constexpr static const uint32_t class_id[3] = { 0x37, 0x0, 0x10b0 };

    Phong() {
        shader_type = s_shader_type;
    }
    static Phong CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
        Phong ret;
        *(Blinn*)&ret = (Blinn::CreateFrom(node));
        ret.shader_type = s_shader_type;
        return ret;
    }
};

// content is the same
class Metal : public Blinn {
public:
    constexpr static const ShaderType s_shader_type = ShaderType_Metal;
    constexpr static const char english_class_name[] = "Metal";
    constexpr static const char class_dll[] = "mtl.dlt";
    constexpr static const uint32_t class_id[3] = { 0x39, 0x0, 0x10b0 };

    Metal() {
        shader_type = s_shader_type;
    }
    static Metal CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
        Metal ret;
        *(Blinn*)&ret = (Blinn::CreateFrom(node));
        ret.shader_type = s_shader_type;
        return ret;
    }

protected:
    using Blinn::soften;
    using Blinn::specular_color;
};

class Texmaps {
public:
    int32_t id = -1;
    struct MapBasicInfo {
        bool enabled = false;
        float amount = 1.0;
        MapType map_type = MapType_Invalid;
        std::shared_ptr<MapBase> target_map_node;
    };
    std::map<uint32_t, MapBasicInfo> maps;

    constexpr static const char english_class_name[] = "Texmaps";
    constexpr static const char class_dll[] = "mtl.dlt";
    constexpr static const uint32_t class_id[3] = { 0x1200, 0x0, 0x1080 };

    static Texmaps CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
        Texmaps ret;
        if (!node) {
            return ret;
        }
        ret.id = node->scene_values_index;
        NodeTypeCheck<Texmaps>(node);

        const auto* node_data = node->data_ptr;
        if (node_data->type == mNode::Container) {
            auto node_1 = ((mContainer*)node_data)->GetFirstContentByID(0x5003);
            if (node_1 && node_1->type == mNode::Value) {
                uint32_t enable_flags = 0;
                enable_flags = ((mUint32Value*)node_1)->GetValue();
                int idx = 0;
                while (enable_flags) {
                    ret.maps[idx].enabled = (enable_flags & 1);
                    enable_flags >>= 1;
                    ++idx;
                }
            }
            for (const auto& m : ((mContainer*)node_data)->content) {
                const static uint16_t prefix_mask = 0xff00;
                uint16_t id_prefix_checker = m->id & prefix_mask;
                if (id_prefix_checker == 0x5100 && m->type == mNode::Value) {
                    int idx = m->id - 0x5100;
                    ret.maps[idx].amount = ((mFloatValue*)m.get())->GetValue();
                }
            }
        }
        for (const auto& m : node->components) {
            int idx = m.first / 2;
            if (m.first % 2 == 0) {
                float v = BezierFloat::CreateFrom(m.second).value;
                if (ret.maps[idx].amount != v) {
                    // 这个值同时出现在两个位置，按理说应该是相同的。
                    PrintError(MaxClassImplError, LEVEL_WARNING, "value in texmaps is not the same as the bezier float component.");
                }
            } else {
                ret.maps[idx].target_map_node = MapBase::GenerateFrom(m.second); 
                if (ret.maps[idx].target_map_node) {
                    ret.maps[idx].map_type = ret.maps[idx].target_map_node->type;
                } else {
                    // unsupported map
                    PrintError(MaxClassImplError, LEVEL_ERROR, "unsupported map type " + m.second->classinfo.name);
                }
            }
        }
        return ret;
    }
};

class StandardMaterial {
public:
    int32_t id = -1;
    MaterialMapThumbnail thumbnail; // 预览图
    std::shared_ptr<ShaderBase> mat_param;
    Texmaps maps;
    int type = -1; // 0/0 as int
    bool is_wire = false; // 0/1 as bool
    bool is_two_sided = false; // 0/2 as bool
    bool is_face_map = false; // 0/3 as bool
    bool is_faceted = false; // 0/4 as bool
    float opacity = 1.0; // 1/1 as float 

    int Culling() const {
        return is_two_sided ? 0 : 1; // 0: no culling, 1: cull ccw
    }
    Texmaps::MapBasicInfo GetMapByIndex(uint32_t index) const {
        auto it = maps.maps.find(index);
        if (it != maps.maps.end()) {
            return it->second;
        }
        return Texmaps::MapBasicInfo();
    }
    Texmaps::MapBasicInfo GetAmbientMap() const {
        if (mat_param)
            return GetMapByIndex(mat_param->GetAmbientMapIndex());
        else
            return Texmaps::MapBasicInfo();
    }
    Texmaps::MapBasicInfo GetDiffuseMap() const {
        if (mat_param)
            return GetMapByIndex(mat_param->GetDiffuseMapIndex());
        else
            return Texmaps::MapBasicInfo();
    }
    Texmaps::MapBasicInfo GetSpecularMap() const {
        if (mat_param)
            return GetMapByIndex(mat_param->GetSpecularMapIndex());
        else
            return Texmaps::MapBasicInfo();
    }
    Texmaps::MapBasicInfo GetGlossinessMap() const {
        if (mat_param)
            return GetMapByIndex(mat_param->GetGlossinessMapIndex());
        else
            return Texmaps::MapBasicInfo();
    }
    Texmaps::MapBasicInfo GetTransparencyMap() const {
        if (mat_param)
            return GetMapByIndex(mat_param->GetOpacityMapIndex());
        else
            return Texmaps::MapBasicInfo();
    }

    constexpr static const char english_class_name[] = "Standard";
    constexpr static const char class_dll[] = "mtl.dlt";
    constexpr static const uint32_t class_id[3] = { 0x2, 0x0, 0xc00 };

    static StandardMaterial CreateFrom(const std::shared_ptr<SceneGraphNode>& node);

protected:
    std::vector<ParamBlock2> param_blocks;
};

class MultiMaterial {
public:
    constexpr static const char english_class_name[] = "Multi/Sub-Object";
    constexpr static const char class_dll[] = "mtl.dlt";
    constexpr static const uint32_t class_id[3] = { 0x200, 0x0, 0xc00 };

    struct SubMaterialItem {
        int32_t id = -1;
        int32_t component_key = -1;
        bool enabled = false;
        std::string name;
        std::shared_ptr<SceneGraphNode> target_material_node;
    };

    int32_t id = -1;
    std::shared_ptr<SceneGraphNode> config_node;
    ParamBlock2 config;
    MaterialMapThumbnail thumbnail; // info with thumbnail

    std::map<int32_t, SubMaterialItem> sub_materials; // only StandardMaterial for now. key is id

    static MultiMaterial CreateFrom(const std::shared_ptr<SceneGraphNode>& node);
};


class Scene {
public:
    int32_t id = -1;

    constexpr static const char english_class_name[] = "Scene";
    constexpr static const char class_dll[] = "";
    constexpr static const uint32_t class_id[3] = { 0x2222, 0x0, 0x100 };

    std::shared_ptr<SceneGraphNode> root;

    std::shared_ptr<SceneGraphNode> GetRoot() const {
        return root;
    }

    static Scene CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
        Scene ret;
        if (!node) {
            return ret;
        }
        ret.id = node->scene_values_index;
        NodeTypeCheck<Scene>(node);

        ret.root = node->GetFirstChildByClassType<RootNode>();
        // TODO (beanpliu): fill children.
        // 目前还是直接找RootNode的，所以没有怎么用到Scene这个类。


        return ret;
    }
};

class NodeMonitor {
public:
    constexpr static const char english_class_name[] = "NodeMonitor";
    constexpr static const char class_dll[] = "ctrl.dlc";
    constexpr static const uint32_t class_id[3] = { 0x18f81903, 0x19033fd2, 0x200 };
};

class BaseLayer {
public:
    int32_t id = -1;
    std::string name;
    uint32_t flag = 0;

    bool IsHide() const {
        return flag & 0x00100000;
    }

    constexpr static const char english_class_name[] = "Base Layer";
    constexpr static const char class_dll[] = "";
    constexpr static const uint32_t class_id[3] = { 0x7e9858fe, 0x1dba1df0, 0x10f0 };

    static BaseLayer CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
        BaseLayer ret;
        if (!node) {
            return ret;
        }
        ret.id = node->scene_values_index;
        NodeTypeCheck<BaseLayer>(node);

        const auto* node_data = node->data_ptr;
        if (node_data->type == mNode::Container) {
            auto node_1 = ((mContainer*)node_data)->GetFirstContentByID(0x1010);
            if (node_1 && node_1->type == mNode::Value) {
                ret.name = ((mStringValue*)node_1)->GetValue();
            }
            auto node_2 = ((mContainer*)node_data)->GetFirstContentByID(0x1020);
            if (node_2 && node_2->type == mNode::Value) {
                ret.flag = ((mUint32Value*)node_2)->GetValue();
            }
        }
        return ret;
    }
};

}// max