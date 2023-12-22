#include "opengl/renderable_item.h"
#include "data_types/mesh_loader.h"
#include "ecs/component/camera_component.h"
#include "glm/glm.hpp"
#include "opengl/buffer_type.h"
#include "opengl/opengl_interface.h"

namespace glit {

std::map<std::string, std::shared_ptr<GLTextureInfo>> GLTextureInfo::cache;

GLVAOInfo CreateBuffer(const std::shared_ptr<OBJECT>& obj, const std::vector<kSECTION_TYPE>& types,
                       bool is_dynamic) {
    // GLuint draw_mode = is_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    GLVAOInfo vao;
    for (const auto& type : types) {
        GLBufferInfo info(type);
        info.SetDynamic(is_dynamic);
        info.SetMeshTarget(obj);
        info.InitBuffer(vao.GetID());
        vao.GetBuffers().emplace(type, std::move(info));
    }
    return vao;
}

void UpdateBuffer(const std::shared_ptr<OBJECT>& obj, const std::vector<kSECTION_TYPE>& types,
                  bool is_dynamic) {
    // GLuint draw_mode = is_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    auto vao = obj->vao_info;
    for (const auto& type : types) {
        auto it = vao->GetBuffers().find(type);
        if (it != vao->GetBuffers().end()) {
            it->second.SetDynamic(is_dynamic);
            it->second.SetMeshTarget(obj);
            it->second.UpdateBuffer(vao->GetID());
        } else {
            GLBufferInfo info(type);
            info.SetDynamic(is_dynamic);
            info.SetMeshTarget(obj);
            info.InitBuffer(vao->GetID());
            vao->GetBuffers().emplace(type, std::move(info));
        }
    }
}

bool SetUniformsRecursive(const std::map<std::string, ShaderVariableInfo>& sp_uniform,
                          const std::map<std::string, UniformValueStorage>& uniform_values,
                          int& current_texture_slot) {
    for (const auto& uniform_entry : sp_uniform) {
        auto it = uniform_values.find(uniform_entry.first);  // name
        if (it != uniform_values.end()) {
            if (uniform_entry.second.entry_type == kEntryType_Normal ||
                uniform_entry.second.entry_type == kEntryType_Array) {
                SetSingleUniform(uniform_entry.second, it->second, current_texture_slot);
            } else {
                // TODO(beanpliu): 此处可能还不正确。
                // array or struct
                if (uniform_entry.second.entry_type == kEntryType_StructArray) {
                    for (int i = 0; i < uniform_entry.second.size; ++i) {
                        const auto& target_item = std::get<1>(uniform_entry.second.children)[i];
                        SetUniformsRecursive(std::get<0>(target_item.children), uniform_values,
                                             current_texture_slot);
                    }
                } else if (uniform_entry.second.entry_type == kEntryType_Struct) {
                    SetUniformsRecursive(std::get<0>(uniform_entry.second.children), uniform_values,
                                         current_texture_slot);
                }
            }
        } else {
            LOG_ERROR("uniform %s is requested but no value can be set by our material.",
                      uniform_entry.first.c_str());
            return false;
        }
    }
    return true;
}

void RenderItem_V1(const std::shared_ptr<CameraComponent>& camera, const RenderableItem& item) {
    // get shader program
    auto shader_program = ShaderFactory::Instance().GetShaderProgram(item);
    if (!shader_program.Valid()) {
        return;
    }
    // update camera uniforms according to material
    std::map<std::string, UniformValueStorage> uniform_values;
    // LOG_DEBUG("Fill uniform values from material and components");
    item.material->UpdateUniforms(uniform_values, camera, item, shader_program);

    RenderState before = RenderState::Instance();
    item.material->UpdateRenderStates();

    // morphed
    std::vector<std::tuple<std::string, std::shared_ptr<GLVAOInfo>, kSECTION_TYPE>>
        additional_attributes;
    if (item.mesh && item.mesh->IsMorphed()) {
        // 在uniform内加入morphTargetInfluences morphTargeti morphNormali  (i in [0, 3])
        auto active_morph_settings = item.mesh->GetMorphWeights(4);
        std::vector<float> morph_target_influences(4, 0.0f);
        for (int i = 0; i < active_morph_settings.size(); ++i) {
            morph_target_influences[i] = active_morph_settings[i].first;
            // targets
            const auto& target = active_morph_settings[i].second;
            if (target != nullptr) {
                additional_attributes.emplace_back("morphTarget" + std::to_string(i),
                                                   target->vao_info, kPOSITION);
                additional_attributes.emplace_back("morphNormal" + std::to_string(i),
                                                   target->vao_info, kNORMAL0);
            }
        }
        // weight
        uniform_values["morphTargetInfluences"] = CopyMemToByteArray(morph_target_influences);
        assert(additional_attributes.size() == 2 * active_morph_settings.size());
    }

    // update properties and draw the mesh
    RenderMesh(item, shader_program, uniform_values, additional_attributes,
               item.material->IsWireFrame(), item.material->IsShowUVLayout());

    // cleanup env

    // restore previous render state
    RenderState::Instance() = before;
    RenderState::Instance().ApplyRenderState();
}

void RenderPBR_V1(const std::set<std::shared_ptr<CameraComponent>>& cameras) {
    for (const auto& camera : cameras) {
        const auto& opaque_list = camera->GetOpaqueList();
        for (const auto& opaque : opaque_list) {
            RenderItem_V1(camera, opaque);
        }

        auto skybox = camera->GetSkyBox();
        if (skybox) RenderItem_V1(camera, *skybox);

        const auto& transparent_list = camera->GetTransparentList();
        for (const auto& transparent : transparent_list) {
            RenderItem_V1(camera, transparent);
        }

        const auto& helper_list = camera->GetHelperList();
        for (const auto& helper : helper_list) {
            RenderItem_V1(camera, helper);
        }

        break;  // only render the first camera now.
    }
}

void RenderMesh(const RenderableItem& item, const ShaderProgram& sp,
                const std::map<std::string, UniformValueStorage>& uniform_values,
                // name, vao, vbo
                std::vector<std::tuple<std::string, std::shared_ptr<GLVAOInfo>, kSECTION_TYPE>>
                    additional_attributes,
                bool is_wire_frame, bool show_uv_layout) {
    if (sp.program == 0) {
        return;
    }
    glUseProgram(sp.program);
    int texture_slot_index = 0;
    // update uniforms into opengl shader
    // LOG_DEBUG("Set uniform values recursively");
    bool result = SetUniformsRecursive(sp.uniforms, uniform_values, texture_slot_index);
    if (!result) return;
    // update attributes into opengl shader
    // LOG_DEBUG("Set attribute values");
    std::set<GLint> locations;
    for (const auto& m : sp.attributes) {
        locations.emplace(m.second.location);
    }
    const auto& buffers = item.mesh->vao_info->GetBuffers();
    glBindVertexArray(item.mesh->vao_info->GetID());
    for (const auto& mesh_section_item : item.mesh->section_item_map) {
        kSECTION_TYPE type = mesh_section_item.first;
        std::string name = item.mesh->GetAttributeName(type);
        auto it = sp.attributes.find(name);
        if (it != sp.attributes.end()) {
            const auto& current_buffer = buffers.at(type);
            glBindBuffer(current_buffer.opengl_buffer_type, current_buffer.GetID());
            glVertexAttribPointer(it->second.location, current_buffer.component_count,
                                  current_buffer.component_type,
                                  current_buffer.component_normalized,
                                  current_buffer.attribute_stride, current_buffer.attribute_offset);
            glEnableVertexAttribArray(it->second.location);  // location in shader
            locations.erase(it->second.location);
        }
    }
    for (const auto& attribute_item : additional_attributes) {
        const auto& name = std::get<0>(attribute_item);
        const auto& current_vao = std::get<1>(attribute_item);
        const auto& buffer_type = std::get<2>(attribute_item);
        if (!current_vao) {
            continue;
        }
        // 不要绑定到原本的vao上，仍使用当前模型的vao。
        // glBindVertexArray(current_vao->GetID()); // 此行注释
        auto it = sp.attributes.find(name);
        if (it != sp.attributes.end()) {
            const auto& current_buffer = current_vao->GetBuffers().at(buffer_type);
            glBindBuffer(current_buffer.opengl_buffer_type, current_buffer.GetID());
            glVertexAttribPointer(it->second.location, current_buffer.component_count,
                                  current_buffer.component_type,
                                  current_buffer.component_normalized,
                                  current_buffer.attribute_stride, current_buffer.attribute_offset);
            glEnableVertexAttribArray(it->second.location);  // location in shader
            locations.erase(it->second.location);
        }
    }
    for (const auto& m : locations) {
        glDisableVertexAttribArray(m);
    }

    // decide render mode
    bool do_not_have_indice = false;

    GLuint draw_count = item.submesh.offset;
    uint64_t draw_offset_byte = 0;

    auto it = buffers.end();
    if (is_wire_frame) {
        if (show_uv_layout) {
            it = buffers.find(kUVTOPOLOGY);
            if (it != buffers.end()) {
                draw_count = item.mesh->uvtopology.size();
            }
        } else {
            it = buffers.find(kTOPOLOGY);
            if (it != buffers.end()) {
                draw_count = item.mesh->topology.size();
            }
        }
        if (it == buffers.end()) is_wire_frame = false;
    }

    if (it == buffers.end()) {
        it = buffers.find(kINDEX);
    }

    static_assert(sizeof(uint64_t) == sizeof(void*), "should build on 64bit system.");
    if (it == buffers.end()) {
        do_not_have_indice = true;
    } else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, it->second.GetID());
        int size = (it->second.component_type == GL_UNSIGNED_INT) ? 4 : 2;
        draw_offset_byte = item.submesh.start * size;
    }
    RenderState::Instance().ApplyRenderState();

    GLuint draw_mode = item.mesh->draw_mode;
    if (is_wire_frame) {
        draw_mode = GL_LINES;
    }

    // LOG_DEBUG("run glDrawXXX");
    if (do_not_have_indice) {
        glDrawArrays(draw_mode, item.submesh.start, item.submesh.offset / 3);
    } else {
        glDrawElements(draw_mode, draw_count, it->second.component_type,
                       reinterpret_cast<void*>(draw_offset_byte));
    }
    for (int i = 0; i < texture_slot_index; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
    glBindVertexArray(0);
    glUseProgram(0);
}

void SetSingleUniform(const ShaderVariableInfo& uniform_entry, const UniformValueStorage& value,
                      int& current_texture_slot) {
    switch (uniform_entry.type) {
        case GL_FLOAT_MAT4:
            assert(sizeof(glm::mat4) * value.count == value.mem.size());
            glUniformMatrix4fv(uniform_entry.location, value.count, false,
                               reinterpret_cast<const GLfloat*>(value.mem.data()));
            break;
        case GL_FLOAT_MAT3:
            assert(sizeof(glm::mat3) * value.count == value.mem.size());
            glUniformMatrix3fv(uniform_entry.location, value.count, false,
                               reinterpret_cast<const GLfloat*>(value.mem.data()));
            break;
        case GL_FLOAT_MAT2:
            assert(sizeof(glm::mat2) * value.count == value.mem.size());
            glUniformMatrix2fv(uniform_entry.location, value.count, false,
                               reinterpret_cast<const GLfloat*>(value.mem.data()));
            break;
        case GL_FLOAT_VEC4:
            assert(sizeof(glm::vec4) * value.count == value.mem.size());
            glUniform4fv(uniform_entry.location, value.count,
                         reinterpret_cast<const GLfloat*>(value.mem.data()));
            break;
        case GL_FLOAT_VEC3:
            assert(sizeof(glm::vec3) * value.count == value.mem.size());
            glUniform3fv(uniform_entry.location, value.count,
                         reinterpret_cast<const GLfloat*>(value.mem.data()));
            break;
        case GL_FLOAT_VEC2:
            assert(sizeof(glm::vec2) * value.count == value.mem.size());
            glUniform2fv(uniform_entry.location, value.count,
                         reinterpret_cast<const GLfloat*>(value.mem.data()));
            break;
        case GL_FLOAT:
            assert(sizeof(glm::vec1) * value.count == value.mem.size());
            glUniform1fv(uniform_entry.location, value.count,
                         reinterpret_cast<const GLfloat*>(value.mem.data()));
            break;
        case GL_INT_VEC4:
            assert(sizeof(glm::ivec4) * value.count == value.mem.size());
            glUniform4iv(uniform_entry.location, value.count,
                         reinterpret_cast<const GLint*>(value.mem.data()));
            break;
        case GL_INT_VEC3:
            assert(sizeof(glm::ivec3) * value.count == value.mem.size());
            glUniform3iv(uniform_entry.location, value.count,
                         reinterpret_cast<const GLint*>(value.mem.data()));
            break;
        case GL_INT_VEC2:
            assert(sizeof(glm::ivec2) * value.count == value.mem.size());
            glUniform2iv(uniform_entry.location, value.count,
                         reinterpret_cast<const GLint*>(value.mem.data()));
            break;
        case GL_INT:
            assert(sizeof(glm::ivec1) * value.count == value.mem.size());
            glUniform1iv(uniform_entry.location, value.count,
                         reinterpret_cast<const GLint*>(value.mem.data()));
            break;
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_2D:
            assert(value.count == 1);
            assert(sizeof(GLuint) * value.count == value.mem.size());
            GLuint uniform_value = *reinterpret_cast<const GLuint*>(&(value.mem.front()));
            glUniform1iv(uniform_entry.location, 1, &current_texture_slot);
            glActiveTexture(GL_TEXTURE0 + current_texture_slot);
            GLuint texture_target = 0;
            if (uniform_entry.type == GL_SAMPLER_2D) {
                texture_target = GL_TEXTURE_2D;
            } else if (uniform_entry.type == GL_SAMPLER_CUBE) {
                texture_target = GL_TEXTURE_CUBE_MAP;
            }
            glBindTexture(texture_target, uniform_value);
            ++current_texture_slot;
            break;
    }
}

}  // namespace glit
