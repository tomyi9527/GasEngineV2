#pragma once
#include <glm/glm.hpp>
#include "data_types/material_factory.h"

class XYPureColorMaterial : public SingleMaterial {
 public:
    static std::shared_ptr<Material> GenerateMaterial() {
        return std::shared_ptr<XYPureColorMaterial>(new XYPureColorMaterial);
    }

    void UpdateUniforms(std::map<std::string, UniformValueStorage>& uniform_values,
                        const std::shared_ptr<CameraComponent>& camera, const RenderableItem& item,
                        const ShaderProgram& sp) const override {
        // nothing to set here
    }

 protected:
    XYPureColorMaterial() : SingleMaterial(kXYPureColorMaterial) {
        auto hint = GetShaderPathHint();

        vertex_shader_content = R"vs(
in vec3 position;
void main() 
{ 
    gl_Position = vec4(position.xy, 1.0, 1.0);
}
)vs";

        fragment_shader_content = R"(
out vec4 FragColor;
void main()
{
	FragColor = vec4(0.5, 0.0, 0.0, 1.0);
}
)";
    }
};
