#pragma once
#include <map>
#include <memory>
#include <tuple>
#include <variant>
#include "data_types/material_factory.h"
#include "opengl/opengl_interface.h"

class ShaderInfo;
class RenderableItem;

enum kShaderVariableEntryType {
    kEntryType_Normal = 0,
    kEntryType_Struct = 1,
    kEntryType_Array = 2,
    kEntryType_StructArray = 3
};
class ShaderVariableInfo {
 public:
    kShaderVariableEntryType entry_type = kEntryType_Normal;
    // self property
    std::string name;
    GLenum type = 0;
    GLint size = 0;
    GLint location = -1;
    // children
    std::variant<std::map<std::string, ShaderVariableInfo>, std::vector<ShaderVariableInfo>>
        children;
};

class ShaderProgram {
 public:
    bool Valid() const { return program > 0; }

 public:
    GLuint program = 0;
    std::map<std::string, ShaderVariableInfo> uniforms;
    std::map<std::string, ShaderVariableInfo> attributes;
    uint64_t vs_key;
    uint64_t fs_key;
};

// vs and fs
class ShaderFactory {
 public:
    static ShaderFactory& Instance() {
        static ShaderFactory instance;
        return instance;
    }
    // either from file or memory
    ShaderProgram GetShaderProgram(const RenderableItem& item);

    // call this only before exit program
    void DestroyPrograms() {
        for (const auto& m : cache) {
            glDeleteProgram(m.second.program);
        }
    }

 protected:
    ShaderFactory() {}

    ShaderProgram CreateShaderProgram(const std::string& vertex_shader_content,
                                      uint64_t vertex_shader_key,
                                      const std::string& fragment_shader_content,
                                      uint64_t fragment_shader_key,
                                      const std::vector<std::string>& vertex_shader_defines,
                                      const std::vector<std::string>& fragment_shader_defines,
                                      const std::vector<std::string>& vertex_shader_extensions,
                                      const std::vector<std::string>& fragment_shader_extensions);

    std::string ProcessShader(std::string shader_content, uint64_t shader_key,
                              std::vector<std::string> shader_defines,
                              std::vector<std::string> shader_extensions);
    GLuint CompileShaders(std::vector<ShaderInfo>& info);
    void GetUniforms(ShaderProgram& program);
    void GetAttributes(ShaderProgram& program);

    // material type, vertex shader key, fragment shader key
    std::map<std::tuple<kMaterialType, uint64_t, uint64_t>, ShaderProgram> cache;
};