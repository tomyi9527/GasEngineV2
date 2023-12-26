#pragma once 
#include <fstream>
#include <assert.h>
#include "binary_to_printable.h"
#include "node.h"
#include "max_structure.h"
#include "cfb_utils.h"
#include "entry_type.h"
#include "error_printer.h"

namespace max {

class BIN_HEADER {
public:
    explicit BIN_HEADER(const char* data) {
        _data = data;
    }

    uint16_t id() const {
        return _id();
    }
    uint64_t size() const {
        uint32_t ret = _size() & 0x7fffffff;
        return ret ? ret : _size_long() & 0x7fffffffffffffff;
    }
    bool IsContainer() const {
        uint32_t ret = _size();
        return ret ? ret & 0x80000000 : _size_long() & 0x8000000000000000;
    }
    uint32_t header_size() const {
        uint32_t ret = _size();
        return ret ? 6 : 14;
    }

protected:
    uint16_t _id() const {
        return *((uint16_t*)&(_data[0]));
    }
    uint32_t _size() const {
        return *((uint32_t*)&(_data[2]));
    }
    uint64_t _size_long() const {
        return *((uint64_t*)&(_data[6]));
    }

    const char *_data;
};

class MaxCracker {
public:
    constexpr static const char FIELD_NAME_3DSMAX_VERSION[] = "3ds Max Version";
    constexpr static const char STREAM_NAME_SUMMARY_INFO[] = "\x05""SummaryInformation";
    constexpr static const char STREAM_NAME_DOCUMENT_SUMMARY_INFO[] = "\x05""DocumentSummaryInformation";
    constexpr static const char STREAM_NAME_VIDEO_POST_QUEUE[] = "VideoPostQueue";
    constexpr static const char STREAM_NAME_DLL_DIRECTORY[] = "DllDirectory";
    constexpr static const char STREAM_NAME_CLASS_DIRECTORY_3[] = "ClassDirectory3";
    constexpr static const char STREAM_NAME_FILE_ASSET_META_3[] = "FileAssetMetaData3";
    constexpr static const char STREAM_NAME_SCENE[] = "Scene";

public:
    MaxCracker(const void* buffer, size_t len)
        :reader(buffer, len) {
        DefaultInitStrategy();
    }

    void run() {
        //auto ptr_VideoPostQueue = ConvertBinary("VideoPostQueue");
        //std::cout << ptr_VideoPostQueue->ToString() << std::endl;

        //auto ptr_DllDirectory = ConvertBinary("DllDirectory");
        //std::cout << ptr_DllDirectory->ToString() << std::endl;

        //auto ptr_ClassDirectory3 = ConvertBinary("ClassDirectory3");
        //std::cout << ptr_ClassDirectory3->ToString() << std::endl;

        //// unknown
        //auto ptr_ClassData = ConvertBinary("ClassData");
        //std::cout << ptr_ClassData->ToString() << std::endl;

        //// unknown
        //auto ptr_Config = ConvertBinary("Config");
        //std::cout << ptr_Config->ToString() << std::endl;

        ParseSummaryStream();
        ParseDllDirectoryStream();
        ParseClassDirectory3();
        ParseFileAssetMeta();
        ParseScene();
        LinkScene();

        //auto ptr_Scene = ConvertBinary("Scene");
        //std::cout << ptr_Scene->ToString() << std::endl;
        //std::ofstream f("out");
        //PrintSceneValues(f);
        //f << std::endl;
        //PrintSceneNodes(f);
        //f << std::endl;
        //Linker::PrintComponents(f);
        //std::cout << _DebugGenerateMindMap(scene_root);
    }

    void Checker_ParseAllUsedStreams() const {
        //auto ptr_VideoPostQueue = ConvertBinary(STREAM_NAME_VIDEO_POST_QUEUE);
        auto ptr_ClassDirectory3 = ConvertBinary(STREAM_NAME_CLASS_DIRECTORY_3);
        auto ptr_FileAssetMetaData3 = ConvertFileAssetMetaData3Binary();
        //auto ptr_ClassData = ConvertBinary("ClassData");
        //auto ptr_Config = ConvertBinary("Config");
        auto ptr_DllDirectory = ConvertBinary(STREAM_NAME_DLL_DIRECTORY);
        auto ptr_Scene = ConvertBinary(STREAM_NAME_SCENE);
    }

    std::unique_ptr<mNode> ConvertBinary(const std::string& name) const {
        std::string memory = reader.GetBuffer(name);
        if (memory.empty()) {
            return nullptr;
        }
        std::list<uint16_t> list;
        return ParseBinary(memory, list);
    }

    // 专门用于转换 FileAssetMetaData3 的
    std::unique_ptr<mNode> ConvertFileAssetMetaData3Binary() const {
        std::string memory = reader.GetBuffer(STREAM_NAME_FILE_ASSET_META_3);
        if (memory.empty()) {
            return nullptr;
        }
        return ParseFileAssetMetaData3Binary(memory);
    }

    std::string GetMaxVersion() const {
        if (max_version > 0)
            return std::to_string(max_version);
        else
            return "version load failed.";
    }

    std::shared_ptr<SceneGraphNode> GetSceneRoot() const {
        return scene_root;
    }

protected:
    void ParseSummaryStream() {
        //reader.GetProperties(STREAM_NAME_SUMMARY_INFO);
        auto document_properties = reader.GetProperties(STREAM_NAME_DOCUMENT_SUMMARY_INFO);
        for (const auto& m : document_properties) {
            for (const auto& k : m.entries) {
                if (k.data.find(FIELD_NAME_3DSMAX_VERSION) != std::string::npos) {
                    ExtractVersionInfo(k.data);
                }
            }
        }
    }

    void ExtractVersionInfo(const std::string& _3dsmax_property_stream) {
        uint32_t start = 8;
        // convert to map
        std::multimap<std::string, std::string> property_map;
        while (start < _3dsmax_property_stream.size()) {
            uint32_t block_len = *(uint32_t*)&(_3dsmax_property_stream[start]);
            start += sizeof(uint32_t);
            if (block_len + start > _3dsmax_property_stream.size()) {
                break;
            }
            std::string_view content(_3dsmax_property_stream.data() + start, block_len);
            while (content.size() && content.back() == '\0') {
                content = content.substr(0, content.size() - 1);
            }
            size_t first_sep_pos = content.find_first_of(':');
            if (first_sep_pos == std::string_view::npos)
                first_sep_pos = content.find_first_of('=');
            std::string_view key = content;
            std::string_view value;
            if (first_sep_pos != std::string_view::npos) {
                key = content.substr(0, first_sep_pos);
                value = content.substr(first_sep_pos + 1);
                while (value.size() && value.front() == ' ') {
                    value = value.substr(1);
                }
                while (value.size() && value.back() == ' ') {
                    value = value.substr(0, value.size() - 1);
                }
            }
            property_map.emplace(key, value);
            start += block_len;
        }
        // find version
        auto it = property_map.find(FIELD_NAME_3DSMAX_VERSION);
        if (it != property_map.end()) {
            max_version = strtod(it->second.data(), nullptr);
        }
    }

    void ParseDllDirectoryStream() {
        auto ptr_DllDirectory = ConvertBinary(STREAM_NAME_DLL_DIRECTORY);
        if (!ptr_DllDirectory || ptr_DllDirectory->type != mNode::Container) {
            PrintError(MaxBinaryParserError, LEVEL_ERROR, "cannot load stream: DllDirectory");
            return;
        }
        //std::cout << ptr_DllDirectory->ToString() << std::endl;
        auto ptr_container = (mContainer*)(ptr_DllDirectory.get());
        Linker::DllInfo().reserve(ptr_container->content.size());
        for (int i = 0; i < ptr_container->content.size(); ++i) {
            const auto& m = ptr_container->content[i];
            if (m->type != mNode::Container) {
                continue;
            }
            auto item_container = (mContainer*)(m.get());
            DllEntry entry;
            entry.valid = true;
            for (const auto& m : item_container->content) {
                if (m->id == 0x2037) {
                    entry.name = ((mUCSStringValue*)(m.get()))->GetValue();
                }
                if (m->id == 0x2039) {
                    entry.description = ((mUCSStringValue*)(m.get()))->GetValue();
                }
            }
            Linker::DllInfo().push_back(std::move(entry));
        }
    }

    void ParseClassDirectory3() {
        auto ptr_ClassDirectory3 = ConvertBinary(STREAM_NAME_CLASS_DIRECTORY_3);
        if (!ptr_ClassDirectory3 || ptr_ClassDirectory3->type != mNode::Container) {
            PrintError(MaxBinaryParserError, LEVEL_ERROR, "cannot load stream: ClassDirectory3");
            return;
        }
        //std::cout << ptr_ClassDirectory3->ToString() << std::endl;
        auto ptr_container = (mContainer*)(ptr_ClassDirectory3.get());
        Linker::ClassInfo().reserve(ptr_container->content.size());
        for (int i = 0; i < ptr_container->content.size(); ++i) {
            const auto& m = ptr_container->content[i];
            if (m->type != mNode::Container) {
                continue;
            }
            auto item_container = (mContainer*)(m.get());
            ClassEntry entry;
            entry.valid = true;
            for (const auto& m : item_container->content) {
                if (m->id == 0x2060) {
                    auto ptr_data = ((mDllClassIDValue*)(m.get()))->GetValue();
                    entry.dll_idx = ptr_data.dll_index;
                    entry.dll_name = ptr_data.dll_name;
                    entry.dll_description = ptr_data.dll_description;
                    entry.data[0] = ptr_data.id1;
                    entry.data[1] = ptr_data.id2;
                    entry.data[2] = ptr_data.id3;
                }
                if (m->id == 0x2042) {
                    entry.name = ((mUCSStringValue*)(m.get()))->GetValue();
                }
            }
            Linker::ClassInfo().push_back(std::move(entry));
        }
    }

    void ParseFileAssetMeta() {
        // FileAssetMetaData2 is not supported now.
        //auto ptr_FileAssetMetaData2 = ConvertBinary("FileAssetMetaData2");
        //if (ptr_FileAssetMetaData2)
        //    std::cout << ptr_FileAssetMetaData2->ToString() << std::endl;

        auto ptr_FileAssetMetaData3 = ConvertFileAssetMetaData3Binary();
        if (!ptr_FileAssetMetaData3) {
            PrintError(MaxBinaryParserError, LEVEL_ERROR, "cannot load stream: FileAssetMetaData3");
            return;
        }
        //std::cout << ptr_FileAssetMetaData3->ToString() << std::endl;
        if (ptr_FileAssetMetaData3->type == mNode::Value) {
            auto result = ((mFileAssetResourceValue*)ptr_FileAssetMetaData3.get())->GetValue();
            for (const auto& m : result) {
                Linker::FileAssetInfo().push_back(m);
            }
        }
    }

    void ParseScene() {
        InitMaxClassStrategy();
        auto ptr_Scene = ConvertBinary(STREAM_NAME_SCENE);
        if (!ptr_Scene || ptr_Scene->type != mNode::Container) {
            PrintError(MaxBinaryParserError, LEVEL_ERROR, "cannot load stream: Scene");
            return;
        }
        //std::cout << ptr_Scene->ToString() << std::endl;
        auto ptr_container = (mContainer*)(ptr_Scene.get());
        if (ptr_container->content.size() == 0) {
            PrintError(MaxBinaryParserError, LEVEL_ERROR, "stream Scene is empty");
            return;
        }
        ptr_container = (mContainer*)(ptr_container->content[0].get());
        // 之后发现这个id好像和version没关系。。
        //max_version = ptr_container->id;
        scene_values = std::move(ptr_container->content);
    }

    void LinkScene() {
        SceneNodeRecord::ClearAll();
        if (scene_root) {
            scene_root->Clear();
            scene_root.reset();
        }
        std::vector<std::shared_ptr<SceneGraphNode>> scene_nodes;
        int32_t idx;
        idx = 0;
        for (const auto& m : scene_values) {
            scene_nodes.push_back(SceneGraphNode::CreateNode(m.get(), idx));
            if (m->id >= 0) {
                const auto& target = Linker::GetClassEntryByIndex(m->id);
                scene_nodes.back()->classinfo = target;
            }
            if (IsNodeType<Scene>(scene_nodes.back())) {
                scene_root = scene_nodes.back();
            }
            idx++;
        }
        idx = 0;
        for (const auto& m : scene_nodes) {
            int32_t parent_idx = GetParentIndex(m->data_ptr);
            if (parent_idx >= 0) {
                scene_nodes[parent_idx]->AddChild(m);
            }
            idx++;
        }
        std::list<std::shared_ptr<SceneGraphNode>> pathes;
        LinkNodes(scene_nodes, scene_root, pathes, false);

        const auto& used_nodes = SceneNodeRecord::NodeList();
        const auto& component_nodes = SceneNodeRecord::ComponentsList();
        for (const auto& m : scene_nodes) {
            auto it1 = used_nodes.find(m);
            auto it2 = component_nodes.find(m);
            if (it1 == used_nodes.end() && it2 == component_nodes.end()) {
                SceneNodeRecord::AddUnusedNodeRecord(m);
            }
        }
    }

public:
    void PrintParsedStream(const std::string& stream_name, std::ostream& s) {
        std::unique_ptr<mNode> ptr;
        if (stream_name == STREAM_NAME_FILE_ASSET_META_3) {
            ptr = ConvertFileAssetMetaData3Binary();
        } else {
            ptr = ConvertBinary(stream_name);
        }
        if (ptr)
            s << ptr->ToString() << std::endl;
    }
    void PrintSceneValues(std::ostream& s) {
        int idx = 0;
        for (const auto& m : scene_values) {
            int32_t class_idx = m->id;
            if (Linker::ClassInfo().size() > class_idx && class_idx >= 0 && Linker::ClassInfo().at(class_idx).valid) {
                const auto& target = Linker::ClassInfo().at(class_idx);
                s << target.name << "  ";
            }
            s << "<" << idx << ">  "
              << m->ToString() << std::endl << std::endl;
            idx++;
        }
    }

    void _DebugExportMeshData() {
        int index = 0;
        int poly_count = 0;
        int mesh_count = 0;
        TraversalNodesAndComponents(scene_root, [&index, &poly_count, &mesh_count](const std::shared_ptr<SceneGraphNode>& node) {
            if (IsNodeType<EditablePoly>(node)) {
                poly_count++;
                auto poly = EditablePoly::CreateFrom(node);
                auto content = poly.ExportAsObj();
                std::fstream out("editablePoly-" + std::to_string(++index) + ".obj", std::ios::out);
                out.write(content.data(), content.size());
            } else if (IsNodeType<EditableMesh>(node)) {
                mesh_count++;
                auto poly = EditableMesh::CreateFrom(node);
                auto content = poly.ExportAsObj();
                std::fstream out("editableMesh-" + std::to_string(++index) + ".obj", std::ios::out);
                out.write(content.data(), content.size());
            }
        });
        PrintInfo("found " + std::to_string(mesh_count) + " Editable Mesh");
        PrintInfo("found " + std::to_string(poly_count) + " Editable Poly");
    }

    void _DebugExportMaterialMapImage() {
        int index = 0;
        int standard_count = 0;
        int colormap_count = 0;
        TraversalNodesAndComponents(scene_root, [&index, &standard_count, &colormap_count](const std::shared_ptr<SceneGraphNode>& node) {
            if (IsNodeType<StandardMaterial>(node)) {
                standard_count++;
                auto standard = StandardMaterial::CreateFrom(node);
                standard.thumbnail.image.ExportAsBmp(standard.thumbnail.name + ".bmp");
            } else if (IsNodeType<ColorMap>(node)) {
                colormap_count++;
                auto colormap = ColorMap::CreateFrom(node);
                colormap.thumbnail.image.ExportAsBmp(colormap.thumbnail.name + ".bmp");
            }
        });
        PrintInfo("found " + std::to_string(standard_count) + " Standard");
        PrintInfo("found " + std::to_string(colormap_count) + " ColorMap");
    }

    void PrintSceneNodes(std::ostream& s, int indent = 0) {
        s << PrintSceneNodes(scene_root, indent) << std::endl;
    }

    std::string PrintSceneNodes(const std::shared_ptr<SceneGraphNode>& node, int indent) {
        if (node) {
            return node->PrintSceneNodesRecursive(indent, true);
        } else {
            return "";
        }
    }

    void _DebugGenerateMindMap(std::stringstream& ss, const std::shared_ptr<SceneGraphNode>& node, std::list<std::shared_ptr<SceneGraphNode>>& parent_nodes, int indent = 1) {
        if (node) {
            parent_nodes.push_back(node);
            std::string indent_str(indent, '*');
            ss << indent_str << " " << (node->classinfo.name.empty() ? "unknown class" : node->classinfo.name) << std::endl;
            if (IsNodeType<NodeMonitor>(node)) {
                for (const auto& m : node->children) {
                    auto it = std::find(parent_nodes.begin(), parent_nodes.end(), m);
                    if (it == parent_nodes.end())
                        _DebugGenerateMindMap(ss, m, parent_nodes, indent + 1);
                }
                for (const auto& m : node->components) {
                    auto it = std::find(parent_nodes.begin(), parent_nodes.end(), m.second);
                    if (it == parent_nodes.end())
                        _DebugGenerateMindMap(ss, m.second, parent_nodes, indent + 1);
                }
            }
            parent_nodes.pop_back();
        }
    }

    std::string _DebugGenerateMindMap(const std::shared_ptr<SceneGraphNode>& node, int indent = 1) {
        std::list<std::shared_ptr<SceneGraphNode>> parent_nodes;
        std::stringstream ss;
        _DebugGenerateMindMap(ss, node, parent_nodes, indent);
        return ss.str();
    }

protected:
    void LinkNodes(const std::vector<std::shared_ptr<SceneGraphNode>>& scene_nodes, std::shared_ptr<SceneGraphNode>& node, std::list<std::shared_ptr<SceneGraphNode>>& link_path, bool is_component) {
        if (!node || !node->data_ptr) {
            return;
        }
        link_path.push_back(node);
        if (!node->children_ready) {
            auto children = GetReferenceChildrenList(node->data_ptr);
            for (const auto & m : children) {
                if (m >= 0 && m < scene_values.size()) {
                    node->AddChild(scene_nodes[m]);
                }
            }
            for (auto& m : node->children) {
                auto it = std::find(link_path.begin(), link_path.end(), m);
                if (it == link_path.end()) {
                    // skip dependency child
                    LinkNodes(scene_nodes, m, link_path, false);
                }
            }
            node->children_ready = true;
        }

        auto components = GetReferenceComponentList(node->data_ptr);
        for (const auto& m : components) {
            if (m.second >= 0 && m.second < scene_values.size()) {
                node->AddComponent(m.first, scene_nodes[m.second]);
            }
        }
        for (auto& m : node->components) {
            auto it = std::find(link_path.begin(), link_path.end(), m.second);
            if (!m.second->children_ready && it == link_path.end()) {
                LinkNodes(scene_nodes, m.second, link_path, true);
            }
        }

        node->is_component = is_component;
        link_path.pop_back();
    }

    int32_t GetParentIndex(const mNode* node) {
        if (!node) {
            return -1;
        }
        if (node->type != mNode::Container) {
            return -1;
        }
        auto ptr = (mContainer*)node;
        for (const auto& m : ptr->content) {
            if (m->id == 0x0960) {
                auto ref = (mArrayValue<uint32_t>*)m.get();
                auto ret = ref->GetValue();
                if (ret.size() > 0) {
                    return ret[0];
                }
            }
        }
        return -1;
    }

    std::vector<int32_t> GetReferenceChildrenList(const mNode* node) {
        if (!node) {
            return {};
        }
        if (node->type != mNode::Container) {
            return {};
        }
        auto ptr = (mContainer*)node;
        for (const auto& m : ptr->content) {
            if (m->id == 0x2034) {
                auto ref = (mReferenceIndexValue*)m.get();
                return ref->GetValue();
            }
        }
        return {};
    }

    std::vector<std::pair<int32_t, int32_t>> GetReferenceComponentList(const mNode* node) {
        if (!node) {
            return {};
        }
        if (node->type != mNode::Container) {
            return {};
        }
        auto ptr = (mContainer*)node;
        // 第一个值 一般表示最长允许长度。-1不限制。有限制的如 MultiMaterial。只读不关心这个值因此不读取。
        for (const auto& m : ptr->content) {
            if (m->id == 0x2035) {
                auto refs = ((mReferenceIndexValue*)m.get())->GetValue();
                if (refs.size() <= 2) {
                    return {};
                }
                auto ret = std::vector<std::pair<int32_t, int32_t>>((refs.size() - 1) / 2, { -1, -1 });
                for (int i = 0; i < ret.size(); ++i) {
                    ret[i] = { refs[2 * i + 1], refs[2 * i + 2] };
                }
                return ret;
            }
        }
        return {};
    }

protected:
    std::vector<std::unique_ptr<mNode>> scene_values;
    std::shared_ptr<SceneGraphNode> scene_root;
    double max_version = 0.0;

    std::unique_ptr<mNode> ParseBinary(const std::string& data, std::list<uint16_t>& id_list, size_t start = 0, size_t end = 0) const {
        if (end == 0) {
            end = data.size();
        }
        if (start > end) {
            return nullptr;
        }
        auto ptr = std::make_unique<mContainer>();
        while (start < end) {
            BIN_HEADER header(data.data() + start);
            uint16_t id = header.id();
            int64_t size = header.size();
            uint32_t header_size = header.header_size();
            //std::cout << std::string(id_list.size() * 2, '-') << std::hex << "id: " << id << std::dec << ", sz: " << size << std::endl;
            if (size + start > end || size < header_size) {
                // important note (beanpliu):
                // 一些max文件在parse过程中会出现 size + start > end 的问题，
                // 1. 如果size越界了，那么当前块[start, end]必定无法成功解析出来。
                //    但是上一层(parent)的[start, end]仍有效，因此直接返回ptr。
                //    直接返回相当于跳过此块的剩余数据，而不是使程序崩溃。
                // 2. 出现问题的原因一般是 data.data() + start 的数据位置不是真正的某一个块的header，
                //    这是由上一个块size错误导致的，上一个块的size可能偏大、偏小，使得当前块起始位置落在了数据区，
                //    因此无法成功解析
                // 3. 影响上，解析结果相比原有内容，可能会丢失部分数据。
                //    出现几率并不大，只有 1 / 1275 的几率
                PrintError(MaxBinaryParserError, LEVEL_ERROR, "size out of range while parsing binary");
                return ptr;
            }
            id_list.push_back(id);
            if (header.IsContainer()) {
                auto container = ParseBinary(data, id_list, start + header_size, start + size);
                container->id = id;
                ptr->content.push_back(std::move(container));
            } else {
                uint32_t data_size = size - header_size;
                assert(data_size >= 0);
                auto value = ValueClassFactory::Generate(id_list);
                value->data = data.substr(start + header_size, data_size);
                value->id = id;
                ptr->content.push_back(std::move(value));
            }
            id_list.pop_back();
            start += size;
        }
        return ptr;
    }

    // 无嵌套
    std::unique_ptr<mNode> ParseFileAssetMetaData3Binary(const std::string& data) const {
        auto ptr = ValueClassFactory::Generate<mFileAssetResourceValue>();
        ptr->data = data;
        return ptr;
    }

    CFBUtils reader;
};
} // max