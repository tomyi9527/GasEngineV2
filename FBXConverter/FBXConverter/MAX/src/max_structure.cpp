#include "entry_type.h"
#include "max_structure.h"
#include "error_printer.h"
#include <string>
#include <iostream>
namespace max {

void SceneNodeRecord::AddNodeRecord(const std::shared_ptr<SceneGraphNode>& item) {
    auto& node_list = NodeList();
    auto it = node_list.find(item);
    if (it == node_list.end()) {
        node_list.emplace(item);
    }
}

void SceneNodeRecord::AddComponentRecord(const std::shared_ptr<SceneGraphNode>& item) {
    auto& component = ComponentsList();
    auto it = component.find(item);
    if (it == component.end()) {
        component.emplace(item);
    }
}

void SceneNodeRecord::PrintComponents(std::ostream& s) {
    Print(s, ComponentsList());
}

void SceneNodeRecord::AddUnusedNodeRecord(const std::shared_ptr<SceneGraphNode>& item) {
    auto& unused_nodes = UnusedList();
    auto it = unused_nodes.find(item);
    if (it == unused_nodes.end()) {
        unused_nodes.emplace(item);
    }
}

void SceneNodeRecord::Print(std::ostream & s, const std::set<std::shared_ptr<SceneGraphNode>>& target) {
    s << "[" << std::endl;
    uint32_t idx = 0;
    for (const auto& m : target) {
        s << "belongs to < ";
        for (const auto& k : m->component_owner) {
            s << k->scene_values_index << " ";

        }
        s << ">: ";
        s << m->PrintSceneNodes(1, [](const std::shared_ptr<SceneGraphNode>& node) {
            return node->is_component;
        });
        idx++;
        if (idx != target.size()) {
            s << ", ";
        } else {
            s << "]";
        }
        s << std::endl;
    }
    s << "]" << std::endl;
}

void SceneNodeRecord::ClearAll() {
    NodeList().clear();
    ComponentsList().clear();
    UnusedList().clear();
}

void SceneGraphNode::AddChild(const std::shared_ptr<SceneGraphNode>& item, bool allow_node_reuse) {
    // check
    if (!item) {
        return;
    }
    auto it = std::find(children.begin(), children.end(), item);
    if (it != children.end()) {
        return;
    }
    // add
    if (item->parent && !allow_node_reuse) {
        item->parent->RemoveChild(item);
    }
    children.emplace_back(item);
    item->parent = GetPtr();
    SceneNodeRecord::AddNodeRecord(item);
    SceneNodeRecord::AddNodeRecord(GetPtr());
}

void SceneGraphNode::RemoveChild(const std::shared_ptr<SceneGraphNode>& item) {
    auto it = std::find(children.begin(), children.end(), item);
    if (it != children.end())
        children.erase(it);
}

void SceneGraphNode::AddComponent(int32_t type, const std::shared_ptr<SceneGraphNode>& item) {
    // check
    SceneNodeRecord::AddComponentRecord(item);
    components.emplace(type, item);
    item->component_owner.emplace(GetPtr());
}

void SceneGraphNode::RemoveComponent(int32_t type) {
    auto it = components.find(type);
    if (it != components.end()) {
        it->second->component_owner.erase(GetPtr());
    }
    components.erase(it);
}

std::string SceneGraphNode::PrintSceneNodesRecursive(int indent, bool recursive_children) {
    // print node
    std::stringstream ss;
    std::string indent_str(indent, '\t');
    std::string indent_str_1(indent + 1, '\t');
    std::string indent_str_2(indent + 2, '\t');
    ss << indent_str << "{" << std::endl;
    ss << indent_str_1 << "\"scene_values_index\": " << scene_values_index << ", " << std::endl;
    ss << indent_str_1 << "\"classname\": \"" << classinfo.name << "\", " << std::endl;
    ss << indent_str_1 << "\"data\": {" << std::endl;
    ss << indent_str_2 << data_ptr->ToString(indent + 2) << std::endl;
    ss << indent_str_1 << "}, " << std::endl;
    if (components.size()) {
        ss << indent_str_1 << "\"components\": {" << std::endl;
        for (const auto& m : components) {
            ss << indent_str_2 << "\"" << m.first << "\": " << m.second->scene_values_index << "," << std::endl;
        }
        ss << indent_str_1 << "}, " << std::endl;
    }
    ss << indent_str_1 << "\"children\": ";
    // print children
    if (children.size()) {
        ss << "[";
        ss << std::endl;
        int32_t idx = 0;
        for (const auto& m : children) {
            idx++;
            if (recursive_children)
                ss << m->PrintSceneNodesRecursive(indent + 2, recursive_children);
            else
                ss << indent_str_2 << m->scene_values_index;
            if (idx != children.size()) {
                ss << ", ";
            } else {
                ss << "]";
            }
            ss << std::endl;
        }
    } else {
        ss << "[]" << std::endl;
    }
    ss << indent_str << "}";
    return ss.str();
}

inline std::shared_ptr<SceneGraphNode> GetModifierBaseObjRecursively(const std::shared_ptr<SceneGraphNode>& node) {
    // requirement: node is a modifier
    if (node && node->data_ptr->id == 0x2032) {
        auto base = node->children.back();
        if (base->data_ptr->id == 0x2032) {
            return GetModifierBaseObjRecursively(base);
        } else {
            return base;
        }
    } else {
        return nullptr;
    }
}

Node Node::CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
    Node ret;
    if (!node) {
        return ret;
    }

    if ( !IsNodeType<Node>(node) && !IsNodeType<RootNode>(node)) {
        PrintError(MaxClassImplError, LEVEL_ERROR, "node type should be [" + std::string(english_class_name) + "], but it's [" + node->classinfo.name + "]");
    }
    ret.id = node->scene_values_index;
    const auto* node_data = node->data_ptr;
    if (node_data->type == mNode::Container) {
        auto node_parent_and_flag = ((mContainer*)node_data)->GetFirstContentByID(0x0960);
        if (node_parent_and_flag && node_parent_and_flag->type == mNode::Value) {
            auto parent_and_flag = ((mArrayValue<uint32_t>*)node_parent_and_flag)->GetValue();
            if (parent_and_flag.size() >= 2) {
                ret.flag = parent_and_flag[1];
            }
        }
        auto node_name = ((mContainer*)node_data)->GetFirstContentByID(0x0962);
        if (node_name && node_name->type == mNode::Value) {
            ret.name = ((mUCSStringValue*)node_name)->GetValue();
        }
        int transform_count = 0;
        auto node_pivot_pos = ((mContainer*)node_data)->GetFirstContentByID(0x096a);
        if (node_pivot_pos && node_pivot_pos->type == mNode::Value) {
            auto values = ((mArrayValue<float>*)node_pivot_pos)->GetValue();
            if (values.size() >= 3) {
                for (int i = 0; i < 3; ++i) {
                    ret.pivot_position[i] = values[i];
                }
                transform_count++;
            }
        }
        auto node_pivot_rot = ((mContainer*)node_data)->GetFirstContentByID(0x096b);
        if (node_pivot_rot && node_pivot_rot->type == mNode::Value) {
            auto values = ((mArrayValue<float>*)node_pivot_rot)->GetValue();
            if (values.size() >= 4) {
                for (int i = 0; i < 4; ++i) {
                    ret.pivot_rotation[i] = values[i];
                }
                transform_count++;
            }
        }
        auto node_pivot_scale = ((mContainer*)node_data)->GetFirstContentByID(0x096c);
        if (node_pivot_scale && node_pivot_scale->type == mNode::Value) {
            auto values = ((mArrayValue<float>*)node_pivot_scale)->GetValue();
            if (values.size() >= 7) {
                for (int i = 0; i < 3; ++i) {
                    ret.pivot_scale[i] = values[i];
                }
                for (int i = 0; i < 4; ++i) {
                    ret.pivot_scale_quat_axis[i] = values[i + 3];
                }
                transform_count++;
            }
        }
        ret.has_pivot_transform = transform_count == 3;
        //auto node_2 = ((mContainer*)node_1)->GetFirstContentByID(0x0914);
        //if (node_2 && node_2->type == mNode::Value) {
        //    // vertex
        //    auto vertex = ((mArrayValueWithHeader<int32_t, float>*)node_2)->GetValue();
        //    ret.points.resize(vertex.first);
        //    for (int i = 0; i < vertex.second.size() / 3; ++i) {
        //        ret.points[i].x = vertex.second[i * 3 + 0];
        //        ret.points[i].y = vertex.second[i * 3 + 1];
        //        ret.points[i].z = vertex.second[i * 3 + 2];
        //    }
        //}
        //auto node_3 = ((mContainer*)node_1)->GetFirstContentByID(0x0912);
        //if (node_3 && node_3->type == mNode::Value) {
        //    // triangle
        //    auto triangle = ((mArrayValueWithHeader<int32_t, MeshFaceStructure>*)node_3)->GetValue();
        //    ret.triangles.resize(triangle.first);
        //    // v1, v2, v3, ??, ??
        //    for (int i = 0; i < triangle.second.size(); ++i) {
        //        ret.triangles[i].v1 = triangle.second[i].v1;
        //        ret.triangles[i].v2 = triangle.second[i].v2;
        //        ret.triangles[i].v3 = triangle.second[i].v3;
        //    }
        //}
        //auto node_4 = ((mContainer*)node_1)->GetFirstContentByID(0x2394);
        //if (node_4 && node_4->type == mNode::Value) {
        //    // uv_points   only x,y has value
        //    auto uv = ((mArrayValueWithHeader<int32_t, float>*)node_4)->GetValue();
        //    ret.uv_points.resize(uv.first);
        //    for (int i = 0; i < uv.second.size() / 3; ++i) {
        //        ret.uv_points[i].x = uv.second[i * 3 + 0];
        //        ret.uv_points[i].y = uv.second[i * 3 + 1];
        //        ret.uv_points[i].z = uv.second[i * 3 + 2];
        //    }
        //}
        //auto node_5 = ((mContainer*)node_1)->GetFirstContentByID(0x2396);
        //if (node_5 && node_5->type == mNode::Value) {
        //    // triangle
        //    auto triangle = ((mArrayValueWithHeader<int32_t, uint32_t>*)node_5)->GetValue();
        //    ret.uv_triangles.resize(triangle.first);
        //    // v1, v2, v3
        //    for (int i = 0; i < triangle.second.size() / 3; ++i) {
        //        ret.uv_triangles[i].v1 = triangle.second[i * 3];
        //        ret.uv_triangles[i].v2 = triangle.second[i * 3 + 1];
        //        ret.uv_triangles[i].v3 = triangle.second[i * 3 + 2];
        //    }
        //}
    }
    for (const auto& m : node->components) {
        // find mesh and material component
        // gather link info only, no need to gather detail info here
        if (IsNodeType<PositionRotationScale>(m.second)) {
            ret.sRT_node = m.second;
        } else if (IsNodeType<EditableMesh>(m.second)) {
            ret.mesh_node = m.second;
        } else if (IsNodeType<EditablePoly>(m.second)) {
            ret.mesh_node = m.second;
        } else if (m.second->data_ptr->id == 0x2032) {
            // 可能会再次嵌套 modifier，因此递归获取。
            PrintInfo("modifier list exists on node '" + ret.name + "', will only take base object");
            auto node = GetModifierBaseObjRecursively(m.second);
            if (node) {
                if (IsNodeType<EditablePoly>(node) || IsNodeType<EditableMesh>(node))
                ret.mesh_node = node;
            } 
        } else if (IsNodeType<BaseLayer>(m.second)) {
            ret.layer_node = m.second;
        } else if (IsNodeType<StandardMaterial>(m.second) || IsNodeType<MultiMaterial>(m.second)) {
            ret.material_node = m.second;
        } // else if (animation_node)
    }
    return ret;
}

std::array<std::array<float, 3>, 2> EditableMesh::GetBBox() const {
    constexpr float _max = std::numeric_limits<float>::max();
    constexpr float _min = std::numeric_limits<float>::lowest();
    std::array<std::array<float, 3>, 2> ret;
    ret[0] = { _max, _max, _max };
    ret[1] = { _min, _min, _min };

    for (int i = 0; i < points.size(); ++i) {
        ret[0][0] = std::min(ret[0][0], points[i].x);
        ret[0][1] = std::min(ret[0][1], points[i].y);
        ret[0][2] = std::min(ret[0][2], points[i].z);
        ret[1][0] = std::max(ret[1][0], points[i].x);
        ret[1][1] = std::max(ret[1][1], points[i].y);
        ret[1][2] = std::max(ret[1][2], points[i].z);
    }
    return ret;
}

std::string EditableMesh::ExportAsObj() const {
    std::stringstream ss;
    bool save_uv = false;
    bool save_normal = false;
    // vertex part
    auto vc_per_vertex = GenerateVertexColorByVCChannel();
    if (vc_per_vertex.size() == points.size()) {
        for (int i = 0; i < points.size(); ++i) {
            ss << "v " << points[i].x << " " << points[i].y << " " << points[i].z << " "
                << vc_per_vertex[i].x << " " << vc_per_vertex[i].y << " " << vc_per_vertex[i].z << std::endl;
        }
    } else {
        for (const auto& m : points) {
            ss << "v " << m.x << " " << m.y << " " << m.z << std::endl;
        }
    }
    // vt part
    auto uv_channel = GetMapChannel(1);
    if (uv_channel && uv_channel->triangles.size() == triangles.size()) {
        save_uv = true;
    }
    if (normal_triangles.size() == triangles.size()) {
        save_normal = true;
    }
    if (save_uv) {
        for (const auto& m : uv_channel->values) {
            ss << "vt " << m.x << " " << m.y << std::endl;
        }
    }
    // vn part
    if (save_normal) {
        for (const auto& m : normal_points) {
            ss << "vn " << m.x << " " << m.y << " " << m.z << std::endl;
        }
    }
    // triangle part
    for (int i = 0; i < triangles.size(); ++i) {
        if (save_uv) {
            if (save_normal) {
                ss << "f "
                    << triangles[i].v1 + 1 << "/" << uv_channel->triangles[i].v1 + 1 << "/" << normal_triangles[i].v1 + 1 << " "
                    << triangles[i].v2 + 1 << "/" << uv_channel->triangles[i].v2 + 1 << "/" << normal_triangles[i].v2 + 1 << " "
                    << triangles[i].v3 + 1 << "/" << uv_channel->triangles[i].v3 + 1 << "/" << normal_triangles[i].v3 + 1 << std::endl;
            } else {
                ss << "f "
                    << triangles[i].v1 + 1 << "/" << uv_channel->triangles[i].v1 + 1 << " "
                    << triangles[i].v2 + 1 << "/" << uv_channel->triangles[i].v2 + 1 << " "
                    << triangles[i].v3 + 1 << "/" << uv_channel->triangles[i].v3 + 1 << std::endl;
            }
        } else {
            if (save_normal) {
                ss << triangles[i].v1 + 1 << "//" << normal_triangles[i].v1 + 1 << " "
                    << triangles[i].v2 + 1 << "//" << normal_triangles[i].v2 + 1 << " "
                    << triangles[i].v3 + 1 << "//" << normal_triangles[i].v3 + 1 << std::endl;
            } else {
                ss << "f "
                    << triangles[i].v1 + 1 << " "
                    << triangles[i].v2 + 1 << " "
                    << triangles[i].v3 + 1 << std::endl;
            }
        }
    }
    return ss.str();
}

std::array<std::array<float, 3>, 2> EditablePoly::GetBBox() const {
    constexpr float _max = std::numeric_limits<float>::max();
    constexpr float _min = std::numeric_limits<float>::lowest();
    std::array<std::array<float, 3>, 2> ret;
    ret[0] = { _max, _max, _max };
    ret[1] = { _min, _min, _min };

    for (int i = 0; i < points.size(); ++i) {
        ret[0][0] = std::min(ret[0][0], points[i].x);
        ret[0][1] = std::min(ret[0][1], points[i].y);
        ret[0][2] = std::min(ret[0][2], points[i].z);
        ret[1][0] = std::max(ret[1][0], points[i].x);
        ret[1][1] = std::max(ret[1][1], points[i].y);
        ret[1][2] = std::max(ret[1][2], points[i].z);
    }
    return ret;
}

std::string EditablePoly::ExportAsObj() const {
    std::stringstream ss;
    for (const auto& m : points) {
        ss << "v " << m.x << " " << m.y << " " << m.z << std::endl;
    }
    for (const auto& m : edges) {
        ss << "l " << m.v1 + 1 << " " << m.v2 + 1 << std::endl;
    }

    for (const auto& m : faces) {
        ss << "f";
        for (const auto& f : m.index) {
            ss << " " << f + 1;
        }
        ss << std::endl;
    }

    return ss.str();
}

EditableMesh EditableMesh::CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
    EditableMesh ret;
    if (!node) {
        return ret;
    }

    // get parent node name
    auto parent_node = GetNodeFromParent(node, [](const std::shared_ptr<SceneGraphNode>& node) {
        return IsNodeType<Node>(node);
    });
    if (parent_node) {
        auto node_converted = Node::CreateFrom(parent_node);
        ret.mesh_name = node_converted.name;
        ret.parent_id = node_converted.id;
    }

    ret.id = node->scene_values_index;
    NodeTypeCheck<EditableMesh>(node);

    const auto* node_data = node->data_ptr;
    if (node_data->type == mNode::Container) {
        auto node_1 = ((mContainer*)node_data)->GetFirstContentByID(0x08fe);
        if (node_1 && node_1->type == mNode::Container) {
            // vertex and triangle
            auto node_2 = ((mContainer*)node_1)->GetFirstContentByID(0x0914);
            if (node_2 && node_2->type == mNode::Value) {
                // vertex
                auto vertex = ((mArrayValueWithHeader<int32_t, float>*)node_2)->GetValue();
                ret.points.resize(vertex.first);
                for (int i = 0; i < vertex.second.size() / 3; ++i) {
                    ret.points[i].x = vertex.second[i * 3 + 0];
                    ret.points[i].y = vertex.second[i * 3 + 1];
                    ret.points[i].z = vertex.second[i * 3 + 2];
                }
            }
            auto node_3 = ((mContainer*)node_1)->GetFirstContentByID(0x0912);
            if (node_3 && node_3->type == mNode::Value) {
                // triangle
                auto triangle = ((mArrayValueWithHeader<int32_t, MeshFaceStructure>*)node_3)->GetValue();
                ret.triangles.resize(triangle.first);
                // v1, v2, v3, ??, ??  (last 2 items are flags related)
                for (int i = 0; i < triangle.second.size(); ++i) {
                    ret.triangles[i].v1 = triangle.second[i].v1;
                    ret.triangles[i].v2 = triangle.second[i].v2;
                    ret.triangles[i].v3 = triangle.second[i].v3;
                    ret.triangles[i].smooth_group = triangle.second[i].smGroup;
                    ret.triangles[i].submesh_index = triangle.second[i].submesh_index;
                }
            }
            // mapping channel info (including vertex color, vt)
            mContainer::mNodeConstIterator node_idx = ((mContainer*)node_1)->GetFirstContentIterByID(0x0959);
            auto node_idx_end = ((mContainer*)node_1)->ContentEnd();
            while(node_idx != node_idx_end) {
                if (node_idx->get() && node_idx->get()->type == mNode::Value) {
                    auto idx = ((mUint32Value*)node_idx->get())->GetValue();
                    MapChannel channel;
                    auto node_4 = ((mContainer*)node_1)->GetFirstContentIterByID(0x2394, node_idx);
                    if (node_4 != node_idx_end && node_4->get() && node_4->get()->type == mNode::Value) {
                        // if uv_points     only x,y has value      id > 0
                        // if vertex-color  rgb float               id == 0
                        auto uv = ((mArrayValueWithHeader<int32_t, float>*)node_4->get())->GetValue();
                        channel.values.resize(uv.first);
                        for (int i = 0; i < uv.second.size() / 3; ++i) {
                            channel.values[i].x = uv.second[i * 3 + 0];
                            channel.values[i].y = uv.second[i * 3 + 1];
                            channel.values[i].z = uv.second[i * 3 + 2];
                        }
                    }
                    auto node_5 = ((mContainer*)node_1)->GetFirstContentIterByID(0x2396, node_idx);
                    if (node_5 != node_idx_end && node_5->get() && node_5->get()->type == mNode::Value) {
                        // uv_triangle
                        auto triangle = ((mArrayValueWithHeader<int32_t, uint32_t>*)node_5->get())->GetValue();
                        channel.triangles.resize(triangle.first);
                        // v1, v2, v3
                        for (int i = 0; i < triangle.second.size() / 3; ++i) {
                            channel.triangles[i].v1 = triangle.second[i * 3 + 0];
                            channel.triangles[i].v2 = triangle.second[i * 3 + 1];
                            channel.triangles[i].v3 = triangle.second[i * 3 + 2];
                        }
                    }
                    ret.channels.emplace(idx, std::move(channel));
                }
                auto next_start = ++node_idx;
                node_idx = ((mContainer*)node_1)->GetFirstContentIterByID(0x0959, next_start);
            }
            // normal info
            auto node_6 = ((mContainer*)node_1)->GetFirstContentByID(0x23a0);
            if (node_6 && node_6->type == mNode::Container) {
                auto node_7 = ((mContainer*)node_6)->GetFirstContentByID(0x0110);
                if (node_7 && node_7->type == mNode::Value) {
                    // vn vertices
                    auto vn = ((mArrayValueWithHeader<int32_t, float>*)node_7)->GetValue();
                    ret.normal_points.resize(vn.first);
                    for (int i = 0; i < vn.second.size() / 3; ++i) {
                        ret.normal_points[i].x = vn.second[i * 3 + 0];
                        ret.normal_points[i].y = vn.second[i * 3 + 1];
                        ret.normal_points[i].z = vn.second[i * 3 + 2];
                    }
                }
                auto node_8 = ((mContainer*)node_6)->GetFirstContentByID(0x0120);
                if (node_8 && node_8->type == mNode::Value) {
                    // vn triangles
                    auto vn_tri_count = ((mUint32Value*)node_8)->GetValue(); // count
                    ret.normal_triangles.resize(vn_tri_count);

                    mContainer::mNodeConstIterator node_9;
                    mContainer::mNodeConstIterator node_10;
                    node_9 = node_10 = ((mContainer*)node_6)->ContentBegin();
                    mContainer::mNodeConstIterator node_end = ((mContainer*)node_6)->ContentEnd();
                    for (int i = 0; i < vn_tri_count; ++i) {
                        node_9 = ((mContainer*)node_6)->GetFirstContentIterByID(0x0124, node_9);
                        node_10 = ((mContainer*)node_6)->GetFirstContentIterByID(0x0128, node_10);
                        if (node_9 != node_end && node_9->get() && node_9->get()->type == mNode::Value &&
                            node_10 != node_end && node_10->get() && node_10->get()->type == mNode::Container) {
                            auto node_11 = ((mContainer*)node_10->get())->GetFirstContentByID(0x0200);
                            if (node_11 && node_11->type == mNode::Value) {
                                auto vn_tri_idx = ((mUint32Value*)node_9->get())->GetValue(); // idx
                                auto vn_tri_value = ((mArrayValue<uint32_t>*)node_11)->GetValue(); // value
                                ret.normal_triangles[vn_tri_idx].v1 = vn_tri_value[0];
                                ret.normal_triangles[vn_tri_idx].v2 = vn_tri_value[1];
                                ret.normal_triangles[vn_tri_idx].v3 = vn_tri_value[2];
                            }
                            ++node_9;
                            ++node_10;
                        }
                    }
                }
            }
        }
    }
    return ret;
}

EditablePoly EditablePoly::CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
    EditablePoly ret;
    if (!node) {
        return ret;
    }

    // get parent node name
    auto parent_node = GetNodeFromParent(node, [](const std::shared_ptr<SceneGraphNode>& node) {
        return IsNodeType<Node>(node);;
    });
    if (parent_node) {
        auto node_converted = Node::CreateFrom(parent_node);
        ret.mesh_name = node_converted.name;
        ret.parent_id = node_converted.id;
    }

    NodeTypeCheck<EditablePoly>(node);

    ret.id = node->scene_values_index;
    const auto* node_data = node->data_ptr;
    if (node_data->type == mNode::Container) {
        auto node_1 = ((mContainer*)node_data)->GetFirstContentByID(0x08fe);
        if (node_1 && node_1->type == mNode::Container) {
            // vertex and edge and face
            auto node_2 = ((mContainer*)node_1)->GetFirstContentByID(0x0100);
            if (node_2 && node_2->type == mNode::Value) {
                // vertex
                auto vertex = ((mArrayValueWithHeader<int32_t, float>*)node_2)->GetValue();
                ret.points.resize(vertex.first);
                for (int i = 0; i < vertex.second.size() / 4; ++i) {
                    ret.points[i].reserved = vertex.second[i * 4];
                    ret.points[i].x = vertex.second[i * 4 + 1];
                    ret.points[i].y = vertex.second[i * 4 + 2];
                    ret.points[i].z = vertex.second[i * 4 + 3];
                }
            }
            auto node_3 = ((mContainer*)node_1)->GetFirstContentByID(0x010a);
            if (node_3 && node_3->type == mNode::Value) {
                // edge
                auto edge = ((mArrayValueWithHeader<int32_t, uint32_t>*)node_3)->GetValue();
                ret.edges.resize(edge.first);
                for (int i = 0; i < edge.second.size() / 3; ++i) {
                    ret.edges[i].reserved = edge.second[i * 3];
                    ret.edges[i].v1 = edge.second[i * 3 + 1];
                    ret.edges[i].v2 = edge.second[i * 3 + 2];
                }
            }
            auto node_4 = ((mContainer*)node_1)->GetFirstContentByID(0x011a);
            if (node_4 && node_4->type == mNode::Value) {
                // face
                auto faces = ((mPolygonFaceStrctureValue*)node_4)->GetValue();
                ret.faces.reserve(faces.size());
                for (int i = 0; i < faces.size(); ++i) {
                    face item;
                    item.index = faces[i].polygon_vertex;
                    item.inner_edge = faces[i].edges;
                    item.smooth_group = faces[i].flag_interpreted.smooth_group;
                    item.submesh_index = faces[i].flag_interpreted.submesh_index;
                    ret.faces.push_back(std::move(item));
                }
            }
            // mapping channel info (including vertex color, vt)
            mContainer::mNodeConstIterator node_idx = ((mContainer*)node_1)->GetFirstContentIterByID(0x0124);
            mContainer::mNodeConstIterator node_idx_end = ((mContainer*)node_1)->ContentEnd();
            while(node_idx != node_idx_end) {
                if (node_idx->get() && node_idx->get()->type == mNode::Value) {
                    auto idx = ((mUint32Value*)node_idx->get())->GetValue();
                    MapChannel channel;
                    auto node_5 = ((mContainer*)node_1)->GetFirstContentIterByID(0x0128, node_idx);
                    if (node_5 != node_idx_end && node_5->get() && node_5->get()->type == mNode::Value) {
                        // if uv_points     only x,y has value      id > 0
                        // if vertex-color  rgb float               id == 0
                        auto uv = ((mArrayValueWithHeader<int32_t, float>*)node_5->get())->GetValue();
                        channel.values.resize(uv.first);
                        for (int i = 0; i < uv.second.size() / 3; ++i) {
                            channel.values[i].x = uv.second[i * 3 + 0];
                            channel.values[i].y = uv.second[i * 3 + 1];
                            channel.values[i].z = uv.second[i * 3 + 2];
                        }
                    }
                    auto node_6 = ((mContainer*)node_1)->GetFirstContentIterByID(0x012b, node_idx);
                    if (node_6 != node_idx_end && node_6->get() && node_6->get()->type == mNode::Value) {
                        // uv_face
                        auto faces = ((mArrayValue<uint32_t>*)node_6->get())->GetValue();
                        int current_index = 0;
                        while (current_index < faces.size()) {
                            uint32_t len = faces.at(current_index);
                            face current_face;
                            for (int i = 0; i < len; ++i) {
                                current_face.index.push_back(faces.at(1 + i + current_index));
                            }
                            current_index += len + 1;
                            channel.faces.push_back(std::move(current_face));
                        }
                    }
                    ret.channels.emplace(idx, std::move(channel));
                }
                // next
                auto next_start = ++node_idx;
                node_idx = ((mContainer*)node_1)->GetFirstContentIterByID(0x0124, next_start);
            }

            // normal info
            auto node_7 = ((mContainer*)node_1)->GetFirstContentByID(0x0300);
            if (node_7 && node_7->type == mNode::Container) {
                auto node_8 = ((mContainer*)node_7)->GetFirstContentByID(0x0110);
                if (node_8 && node_8->type == mNode::Value) {
                    // vn vertices
                    auto vn = ((mArrayValueWithHeader<int32_t, float>*)node_8)->GetValue();
                    ret.normal_points.resize(vn.first);
                    for (int i = 0; i < vn.second.size() / 3; ++i) {
                        ret.normal_points[i].x = vn.second[i * 3 + 0];
                        ret.normal_points[i].y = vn.second[i * 3 + 1];
                        ret.normal_points[i].z = vn.second[i * 3 + 2];
                    }
                }
                auto node_9 = ((mContainer*)node_7)->GetFirstContentByID(0x0120);
                if (node_9 && node_9->type == mNode::Value) {
                    // vn triangles
                    auto vn_tri_count = ((mUint32Value*)node_9)->GetValue(); // count
                    ret.normal_faces.resize(vn_tri_count);

                    mContainer::mNodeConstIterator node_10;
                    mContainer::mNodeConstIterator node_11;
                    node_11 = node_10 = ((mContainer*)node_7)->ContentBegin();
                    mContainer::mNodeConstIterator node_end = ((mContainer*)node_7)->ContentEnd();
                    for (int i = 0; i < vn_tri_count; ++i) {
                        node_10 = ((mContainer*)node_7)->GetFirstContentIterByID(0x0124, node_10);    // index
                        node_11 = ((mContainer*)node_7)->GetFirstContentIterByID(0x0128, node_11);  // n, v1, v2, ... vn
                        if (node_10 != node_end && node_10->get() && node_10->get()->type == mNode::Value &&
                            node_11 != node_end && node_11->get() && node_11->get()->type == mNode::Container) {
                            auto node_12 = ((mContainer*)node_11->get())->GetFirstContentByID(0x0200);
                            if (node_12 && node_12->type == mNode::Value) {
                                auto vn_tri_idx = ((mUint32Value*)node_10->get())->GetValue(); // idx
                                auto vn_tri_value = ((mArrayValue<uint32_t>*)node_12)->GetValue(); // value
                                if (vn_tri_value.size() && vn_tri_value.front() + 1 == vn_tri_value.size())
                                    for (int i = 0; i < vn_tri_value.front(); ++i) {
                                        ret.normal_faces[vn_tri_idx].index.push_back(vn_tri_value[i + 1]);
                                        // inner_edge doesn't exists for normal
                                    }
                            }
                            ++node_10;
                            ++node_11;
                        }
                    }
                }
            }
        }
    }
    return ret;
}

Blinn Blinn::CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
    Blinn ret;
    if (!node) {
        return ret;
    }
    ret.id = node->scene_values_index;
    for (const auto& m : node->children) {
        if (IsNodeType<ParamBlock2>(m)) {
            ret.param_blocks.emplace_back(ParamBlock2::CreateFrom(m));
        }
    }
    // Takeout values from paramblock
    if (ret.param_blocks.size() > 0 && ret.param_blocks[0].value_nodes.size() >= 12) {
#define AssignValueTo(idx1, idx2, output_var, expected_type, AssignFunction) { \
    auto v = ret.param_blocks[idx1].value_nodes[idx2]->GetValue();\
    if (expected_type == v.first.GetType()) {\
        v.second.AssignFunction(output_var);\
    } else {\
        PrintError(MaxClassImplError, LEVEL_ERROR, "assign type mismatch: " + std::to_string(expected_type) + " vs " + std::to_string(v.first.GetType())); \
    }\
}\

#define AssignValueToIterable(v1, v2, v3, v4) AssignValueTo(v1, v2, v3, v4, AssignToIterable)
#define AssignValueToValue(v1, v2, v3, v4) AssignValueTo(v1, v2, v3, v4, AssignToValue)

        // 0 
        AssignValueToIterable(0, 0, ret.ambient_color, TYPE_RGBA);
        // 1
        AssignValueToIterable(0, 1, ret.diffuse_color, TYPE_RGBA);
        // 2
        AssignValueToIterable(0, 2, ret.specular_color, TYPE_RGBA);
        // 3
        AssignValueToValue(0, 3, ret.map_lock, TYPE_BOOL);
        // 4
        AssignValueToValue(0, 4, ret.ambient_diffuse_bind, TYPE_BOOL);
        // 5
        AssignValueToValue(0, 5, ret.diffuse_specular_bind, TYPE_BOOL);
        // 6
        AssignValueToValue(0, 6, ret.self_illumination_is_color, TYPE_BOOL);
        // 7
        AssignValueToValue(0, 7, ret.self_illumination_percent, TYPE_PCNT_FRAC);
        // 8
        AssignValueToIterable(0, 8, ret.self_illumination_color, TYPE_RGBA);
        // 9
        AssignValueToValue(0, 9, ret.specular_level, TYPE_PCNT_FRAC);
        // 10
        AssignValueToValue(0, 10, ret.glossiness, TYPE_PCNT_FRAC);
        // 11
        AssignValueToValue(0, 11, ret.soften, TYPE_FLOAT);

#undef AssignValueToValue
#undef AssignValueToIterable
#undef AssignValueTo
    }
    return ret;
}

ParamBlock2 ParamBlock2::CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
    ParamBlock2 ret;
    if (!node) {
        return ret;
    }
    ret.id = node->scene_values_index;
    NodeTypeCheck<ParamBlock2>(node);

    const auto* node_data = node->data_ptr;
    if (node_data->type == mNode::Container) {
        // header
        auto node_1 = ((mContainer*)node_data)->GetFirstContentByID(0x0009);
        if (node_1 && node_1->type == mNode::Value) {
            auto values = ((mArrayValue<int16_t>*)node_1)->GetValue();
            if (values.size() >= 6) {
                ret.parent_index = values[6];
                ret.child_index_in_parent = values[2];
                ret.count = values[5];
            }
        } else {
            node_1 = ((mContainer*)node_data)->GetFirstContentByID(0x000b);
            if (node_1 && node_1->type == mNode::Value) {
                auto values = ((mArrayValue<int16_t>*)node_1)->GetValue();
                if (values.size() >= 10) {
                    ret.parent_index = values[10];
                    ret.child_index_in_parent = values[6];
                    ret.count = values[9];
                }
            }
        }
        // value
        mContainer::mNodeConstIterator node_2 = ((mContainer*)node_data)->GetFirstContentIterByID(0x100e);
        mContainer::mNodeConstIterator node_2_end = ((mContainer*)node_data)->ContentEnd();
        while(node_2 != node_2_end) {
            if (node_2->get() && node_2->get()->type == mNode::Value) {
                ret.value_nodes.push_back((mParamBlockValue*)node_2->get());
            }
            auto next_start = ++node_2;
            node_2 = ((mContainer*)node_data)->GetFirstContentIterByID(0x100e, next_start);
        }
        if (ret.value_nodes.size() != ret.count) {
            PrintError(MaxClassImplError, LEVEL_ERROR, "ParamBlock2 child nodes count incorrect (real/expected): " + std::to_string(ret.value_nodes.size()) + "/" + std::to_string(ret.count));
        }
    }
    return ret;
}

StandardMaterial StandardMaterial::CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
    StandardMaterial ret;
    if (!node) {
        return ret;
    }
    ret.id = node->scene_values_index;
    NodeTypeCheck<StandardMaterial>(node);

    const auto* node_data = node->data_ptr;
    if (node_data->type == mNode::Container) {
        auto node_1 = ((mContainer*)node_data)->GetFirstContentByID(0x4000);
        if (node_1 && node_1->type == mNode::Container) {
            ret.thumbnail = MaterialMapThumbnail::CreateFromDataNode(node_1);
        }
    }
    for (const auto& m : node->children) {
        if (IsNodeType<ParamBlock2>(m)) {
            ret.param_blocks.emplace_back(ParamBlock2::CreateFrom(m));
        }
    }
    // Takeout values from paramblock
    if (ret.param_blocks.size() > 0) {
#define AssignValueTo(idx1, idx2, output_var, expected_type, AssignFunction) { \
    auto v = ret.param_blocks[idx1].value_nodes[idx2]->GetValue();\
    if (expected_type == v.first.GetType()) {\
        v.second.AssignFunction(output_var);\
    } else {\
        PrintError(MaxClassImplError, LEVEL_ERROR, "assign type mismatch: " + std::to_string(expected_type) + " vs " + std::to_string(v.first.GetType())); \
    }\
}\

#define AssignValueToIterable(v1, v2, v3, v4) AssignValueTo(v1, v2, v3, v4, AssignToIterable)
#define AssignValueToValue(v1, v2, v3, v4) AssignValueTo(v1, v2, v3, v4, AssignToValue)
        if (ret.param_blocks[0].value_nodes.size() >= 5) {
            // 0 
            AssignValueToValue(0, 0, ret.type, TYPE_INT);
            // 1
            AssignValueToValue(0, 1, ret.is_wire, TYPE_BOOL);
            // 2
            AssignValueToValue(0, 2, ret.is_two_sided, TYPE_BOOL);
            // 3
            AssignValueToValue(0, 3, ret.is_face_map, TYPE_BOOL);
            // 4
            AssignValueToValue(0, 4, ret.is_faceted, TYPE_BOOL);
        }
        if (ret.param_blocks[1].value_nodes.size() >= 2) {
            // 1 
            AssignValueToValue(1, 1, ret.opacity, TYPE_PCNT_FRAC);
        }
#undef AssignValueToValue
#undef AssignValueToIterable
#undef AssignValueTo
    }
    
    for (const auto& m : node->children) {
        if (IsNodeType<Metal>(m)) {
            ret.mat_param = std::make_shared<Metal>(Metal::CreateFrom(m));
        } else if (IsNodeType<Blinn>(m)) {
            ret.mat_param = std::make_shared<Blinn>(Blinn::CreateFrom(m));
        } else if (IsNodeType<Phong>(m)) {
            ret.mat_param = std::make_shared<Phong>(Phong::CreateFrom(m));
        } else if (IsNodeType<Texmaps>(m)) {
            ret.maps = Texmaps::CreateFrom(m);
        }
    }
    if (!ret.mat_param || ret.mat_param->shader_type != ret.type) {
        PrintError(MaxClassImplError, LEVEL_ERROR, "material parameter parse failed.");
    }
    return ret;
}

std::shared_ptr<MapBase> MapBase::GenerateFrom(const std::shared_ptr<SceneGraphNode>& node) {
    if (!node) { return nullptr; }
    if (IsNodeType<ColorMap>(node)) {
        return std::make_shared<ColorMap>(ColorMap::CreateFrom(node));
    } else if (IsNodeType<Bitmap>(node)) {
        return std::make_shared<Bitmap>(Bitmap::CreateFrom(node));
    } else {
        return nullptr;
    }
}

ColorMap ColorMap::CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
    ColorMap ret;
    if (!node) {
        return ret;
    }
    NodeTypeCheck<ColorMap>(node);

    *(MapBase*)(&ret) = MapBase::CreateFrom(node);
    ret.type = MapType_ColorMap;

    // take data from pb2
    for (const auto& m : node->children) {
        if (IsNodeType<ParamBlock2>(m)) {
            auto pb2 = ParamBlock2::CreateFrom(m);
            if (pb2.child_index_in_parent == 0 && pb2.value_nodes.size() >= 5) {

#define AssignValueTo(idx, output_var, expected_type, AssignFunction) { \
    auto v = pb2.value_nodes[idx]->GetValue();\
    if (expected_type == v.first.GetType()) {\
        v.second.AssignFunction(output_var);\
    } else {\
        PrintError(MaxClassImplError, LEVEL_ERROR, "assign type mismatch: " + std::to_string(expected_type) + " vs " + std::to_string(v.first.GetType())); \
    }\
}\

#define AssignValueToIterable(v1, v2, v3) AssignValueTo(v1, v2, v3, AssignToIterable)
#define AssignValueToValue(v1, v2, v3) AssignValueTo(v1, v2, v3, AssignToValue)
                // 0
                AssignValueToIterable(0, ret.solid_color, TYPE_FRGBA);
                // 1
                AssignValueToValue(1, ret.map_index, TYPE_TEXMAP);
                // 2
                AssignValueToValue(2, ret.has_map, TYPE_BOOL);
                // 3
                AssignValueToValue(3, ret.gamma, TYPE_FLOAT);;
                // 4
                AssignValueToValue(4, ret.gain, TYPE_FLOAT);
                // 5
                AssignValueToValue(5, ret.reverse_gamma, TYPE_BOOL);
#undef AssignValueToValue
#undef AssignValueToIterable
#undef AssignValueTo
            }
        } else {
            auto map_ptr = MapBase::GenerateFrom(m);
            if (map_ptr) {
                ret.map = map_ptr;
            }
        }
    }

    return ret;
}

Bitmap Bitmap::CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
    Bitmap ret;
    if (!node) {
        return ret;
    }
    NodeTypeCheck<Bitmap>(node);

    *(MapBase*)(&ret) = MapBase::CreateFrom(node);
    ret.type = MapType_Bitmap;

    // take data from pb2
    for (const auto& m : node->children) {
        if (IsNodeType<ParamBlock2>(m)) {
            auto pb2 = ParamBlock2::CreateFrom(m);
            if (pb2.child_index_in_parent == 0 && pb2.value_nodes.size() >= 5) {
                // check for 0x0003
                auto node_map = ((mContainer*)m->data_ptr)->GetFirstContentByID(0x0003);
                if (node_map && node_map->type == mNode::Container) {
                    auto node_id = ((mContainer*)node_map)->GetFirstContentByID(0x1260);
                    if (node_id && node_id->type == mNode::Container) {
                        node_id = ((mContainer*)node_id)->GetFirstContentByID(0x0002);
                        if (node_id && node_id->type == mNode::Value) {
                            ret.bitmap_assetid = ((const mValue*)node_id)->data;
                        }
                    }
                    auto node_type = ((mContainer*)node_map)->GetFirstContentByID(0x1240);
                    if (node_type && node_type->type == mNode::Value) {
                        ret.bitmap_filetype = ((const mUCSStringValue*)node_type)->GetValue();
                    }
                }
            }
        } 
    }
    return ret;
}

MultiMaterial MultiMaterial::CreateFrom(const std::shared_ptr<SceneGraphNode>& node) {
    MultiMaterial ret;
    if (!node) {
        return ret;
    }
    ret.id = node->scene_values_index;
    NodeTypeCheck<MultiMaterial>(node);

    std::map<int, std::shared_ptr<SceneGraphNode>> sub_material_nodes; // 顺序-->node， first 不是 sub_material_id
    std::vector<int32_t> sub_material_ids; // 使用first在此数组查询即可得到 sub_material_id
    std::vector<int32_t> sub_material_enabled; // 使用first在此数组查询即可得到 enabled
    std::vector<std::string> sub_material_name; // 使用first在此数组查询即可得到 name

    // 一般来说是这种格式
    int index = 0;
    for (const auto& m : node->children) {
        if (IsNodeType<ParamBlock2>(m)) {
            ret.config_node = m;
            ret.config = ParamBlock2::CreateFrom(ret.config_node);
        } else if (IsNodeType<StandardMaterial>(m)) {
            // only standard is supported now. 
            sub_material_nodes.emplace(index - 1, m);
        }
        index++;
    }

    // 如果进行了修改可能会变成component的记录方式
    for (const auto& m : node->components) {
        if (IsNodeType<ParamBlock2>(m.second)) {
            ret.config_node = m.second;
            ret.config = ParamBlock2::CreateFrom(ret.config_node);
        } else if (IsNodeType<StandardMaterial>(m.second)) {
            // only standard is supported now. 
            assert(m.first > 0); // 0 是一个 paramBlock
            sub_material_nodes.emplace(m.first - 1, m.second);
        }
    }
    const auto* node_data = node->data_ptr;
    if (node_data->type == mNode::Container) {
        auto node_1 = ((mContainer*)node_data)->GetFirstContentByID(0x4000);
        if (node_1 && node_1->type == mNode::Container) {
            ret.thumbnail = MaterialMapThumbnail::CreateFromDataNode(node_1);
        }
    }

    if (ret.config_node && ret.config.value_nodes.size() >= 4) {
        sub_material_enabled = ret.config.value_nodes[1]->GetValue().second.GetAsBoolTypeArray();
        sub_material_name = ret.config.value_nodes[2]->GetValue().second.GetAsStringArray();
        sub_material_ids = ret.config.value_nodes[3]->GetValue().second.GetAsTrivalTypeArray<int32_t>();
    }

    for (const auto& m : sub_material_nodes) {
        SubMaterialItem item;
        item.component_key = m.first;
        item.target_material_node = m.second;
        if (!sub_material_enabled.empty() && m.first < sub_material_enabled.size()) {
            item.enabled = sub_material_enabled[m.first];
        }
        if (!sub_material_name.empty() && m.first < sub_material_name.size()) {
            item.name = sub_material_name[m.first];
        }
        if (!sub_material_ids.empty() && m.first < sub_material_ids.size()) {
            item.id = sub_material_ids[m.first];
        }
        ret.sub_materials.emplace(item.id, item);
    }

    return ret;
}
} // max