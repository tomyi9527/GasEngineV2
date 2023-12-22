#include "shader_factory.h"
#include <algorithm>
#include <regex>
#include <sstream>
#include "opengl/buffer_type.h"
#include "opengl/renderable_item.h"
#include "utils/logger.h"

constexpr char global_default_precision[] = R"(
#ifdef GL_FRAGMENT_PRECISION_HIGH
    precision highp float;
#else
    precision mediump float;
#endif
)";

class ShaderInfo {
 public:
    ShaderInfo(GLenum in_type, const std::string& in_shader_src)
        : type(in_type), shader_src(in_shader_src) {}

 public:
    GLenum type = 0;
    std::string shader_src;
    GLuint shader = 0;
};

ShaderProgram ShaderFactory::GetShaderProgram(const RenderableItem& item) {
    if (item.mesh == nullptr || item.material == nullptr) {
        return ShaderProgram();
    }
    auto vs_key = item.material->GenerateVertexShaderKey(item);
    auto fs_key = item.material->GenerateFragmentShaderKey(item);

    auto search_key = std::make_tuple(item.material->GetType(), vs_key, fs_key);
    auto it = cache.find(search_key);
    if (it != cache.end()) {
        return it->second;
    }
    ShaderProgram ret;
    ret = CreateShaderProgram(item.material->GetVertexShaderContent(), vs_key,
                              item.material->GetFragmentShaderContent(), fs_key,
                              {"#define SHADERKEY " + std::to_string(vs_key)},
                              {"#define SHADERKEY " + std::to_string(fs_key)}, {},
                              {"#extension GL_NV_shadow_samplers_cube : enable"});
    if (ret.program > 0)
        cache.emplace(search_key, ret);
    else
        assert(ret.program);  // "shader compile failed."
    return ret;
}

ShaderProgram ShaderFactory::CreateShaderProgram(
    const std::string& vertex_shader_content, uint64_t vertex_shader_key,
    const std::string& fragment_shader_content, uint64_t fragment_shader_key,
    const std::vector<std::string>& vertex_shader_defines,
    const std::vector<std::string>& fragment_shader_defines,
    const std::vector<std::string>& vertex_shader_extensions,
    const std::vector<std::string>& fragment_shader_extensions) {
    // process shader with defines and extensions
    std::string vertex_shader_full_content = ProcessShader(
        vertex_shader_content, vertex_shader_key, vertex_shader_defines, vertex_shader_extensions);
    std::string fragment_shader_full_content =
        ProcessShader(fragment_shader_content, fragment_shader_key, fragment_shader_defines,
                      fragment_shader_extensions);

    std::vector<ShaderInfo> shaders = {{GL_VERTEX_SHADER, vertex_shader_full_content},
                                       {GL_FRAGMENT_SHADER, fragment_shader_full_content}};
    // compile shader
    ShaderProgram ret;
    ret.vs_key = vertex_shader_key;
    ret.fs_key = fragment_shader_key;
    ret.program = CompileShaders(shaders);
    // get uniform locations
    GetUniforms(ret);
    // get attribute locations
    GetAttributes(ret);

    return ret;
}

GLuint ShaderFactory::CompileShaders(std::vector<ShaderInfo>& infos) {
    GLuint program = glCreateProgram();
    for (auto& info : infos) {
        GLuint shader = glCreateShader(info.type);
        if (shader == 0) {
            LOG_ERROR("shader creation error on glCreateShader");
        }
        info.shader = shader;
        const GLchar* source = info.shader_src.c_str();
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLsizei len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            GLchar* log = new GLchar[len + 1];
            glGetShaderInfoLog(shader, len, &len, log);
            LOG_ERROR("Shader compilation failed: %s", log);
            LOG_ERROR("Shader content: %s", info.shader_src.c_str());
            delete[] log;
            return 0;
        }
        glAttachShader(program, shader);
    }
    glLinkProgram(program);
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLsizei len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        GLchar* log = new GLchar[len + 1];
        glGetProgramInfoLog(program, len, &len, log);
        LOG_ERROR("Shader linking failed: %s", log);
        delete[] log;
        for (auto& info : infos) {
            glDeleteShader(info.shader);
            LOG_ERROR("Shader content: %s", info.shader_src.c_str());
            info.shader = 0;
        }
        return 0;
    }
    for (auto& info : infos) {
        glDeleteShader(info.shader);  // after linked
        info.shader = 0;
    }
    return program;
}

constexpr GLsizei bufSize = 256;  // maximum name length
constexpr char struct_regex_str[] = R"reg(^([\w\d_]+)\.([\w\d_]+)$)reg";
constexpr char array_struct_regex_str[] = R"reg(^([\w\d_]+)\[(\d+)\]\.([\w\d_]+)$)reg";
constexpr char array_regex_str[] = R"reg(^([\w\d_]+)\[0\]$)reg";

void ShaderFactory::GetUniforms(ShaderProgram& program) {
    const static std::regex struct_regex(struct_regex_str);
    const static std::regex array_struct_regex(array_struct_regex_str);
    const static std::regex array_regex(array_regex_str);

    GLchar name[bufSize];  // variable name in GLSL
    GLsizei length;        // name length

    GLint count = 0;
    glGetProgramiv(program.program, GL_ACTIVE_UNIFORMS, &count);
    printf("Active Uniforms: %d\n", count);

    for (GLint i = 0; i < count; i++) {
        ShaderVariableInfo info;
        info.entry_type = kEntryType_Normal;
        glGetActiveUniform(program.program, (GLuint)i, bufSize, &length, &info.size, &info.type,
                           name);
        info.name.assign(name, length);
        info.location = glGetUniformLocation(program.program, name);
        printf("Uniform #%d Type: %u Name: %s\n", i, info.type, info.name.c_str());

        // following will match: struct, array_struct, array, normal
        // try struct match
        std::smatch match_result;
        if (std::regex_match(info.name, match_result, struct_regex)) {
            ShaderVariableInfo info_struct;
            info_struct.name = match_result[1];  // struct_name
            info_struct.entry_type = kEntryType_Struct;
            program.uniforms.emplace(info.name, info);
            info.name = match_result[2];  // struct_property
            std::get<0>(info_struct.children).emplace(info.name, info);
        } else if (std::regex_match(info.name, match_result, array_struct_regex)) {
            ShaderVariableInfo info_array;
            info_array.name = match_result[1];  // array_name
            info_array.entry_type = kEntryType_StructArray;
            program.uniforms.emplace(info.name, info);

            ShaderVariableInfo info_array_index;
            info_array_index.name = match_result[2];  // array_index
            info_array_index.entry_type = kEntryType_Struct;
            try {
                uint64_t array_index = std::stoull(info_array_index.name);
                if (std::get<1>(info_array.children).size() <= array_index)
                    std::get<1>(info_array.children).resize(array_index + 1);
                std::get<1>(info_array.children)[array_index] = (info);

                info.name = match_result[3];  // array_property
                std::get<0>(info_array_index.children).emplace(info.name, info);
            } catch (...) {
                LOG_ERROR("can not convert to number: %s", info_array_index.name.c_str());
            }
        } else if (std::regex_match(info.name, match_result, array_regex)) {
            info.name = match_result[1];  // array_name
            info.entry_type = kEntryType_Array;
            program.uniforms.emplace(info.name, info);
        } else {
            program.uniforms.emplace(info.name, info);
        }
    }
}

void ShaderFactory::GetAttributes(ShaderProgram& program) {
    GLchar name[bufSize];  // variable name in GLSL
    GLsizei length;        // name length

    GLint count = 0;
    glGetProgramiv(program.program, GL_ACTIVE_ATTRIBUTES, &count);
    printf("Active Attributes: %d\n", count);

    for (GLint i = 0; i < count; i++) {
        ShaderVariableInfo info;
        glGetActiveAttrib(program.program, (GLuint)i, bufSize, &length, &info.size, &info.type,
                          name);
        info.name.assign(name, length);
        info.location = glGetAttribLocation(program.program, name);
        printf("Attribute #%d Type: %u Name: %s\n", i, info.type, info.name.c_str());
        program.attributes.emplace(info.name, info);
    }
}

inline std::string InstrumentShaderlines(const std::string& content, GLuint source_id) {
    return std::string("\n#line 0 ") + std::to_string(source_id) + '\n' + content;
}

std::string ShaderFactory::ProcessShader(std::string shader_content, uint64_t shader_key,
                                         std::vector<std::string> shader_defines,
                                         std::vector<std::string> shader_extensions) {
    constexpr bool with_debug_lines = true;
    int source_id = 0;
    if (with_debug_lines) {
        shader_content = InstrumentShaderlines(shader_content, source_id++);
    }
    if (!shader_defines.empty()) {
        std::sort(shader_defines.begin(), shader_defines.end());
    }
    if (!shader_extensions.empty()) {
        std::sort(shader_extensions.begin(), shader_extensions.end());
    }
    // TODO(beanpliu): preprocess 还没有做
    // shader_content = preprocess(shader_content);

    // preprend
    std::stringstream ss;
    ss << "#version 330" << std::endl << std::endl;
    if (!shader_extensions.empty()) {
        for (const auto& item : shader_extensions) {
            ss << item << std::endl;
        }
        ss << std::endl;
    }
    if (global_default_precision) {
        ss << global_default_precision << std::endl;
    }
    ss << "#define SHADER_NAME " << shader_key << std::endl << std::endl;
    if (!shader_defines.empty()) {
        for (const auto& item : shader_defines) {
            ss << item << std::endl;
        }
        ss << std::endl;
    }
    ss << shader_content;
    return ss.str();
}