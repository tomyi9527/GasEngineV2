#pragma once
#include <math.h>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include "GLFW/glfw3.h"
#include "glad/glad.h"
struct GLFWwindow;

namespace glit {

#ifdef USE_OSMESA
constexpr const bool is_osmesa_build = true;
#else
constexpr const bool is_osmesa_build = false;
#endif

typedef struct {
    GLenum type;
    const char* filename;
    GLuint shader;
} ShaderInfo;

GLuint LoadShaders(ShaderInfo*);

void PrintExtensions();
// call this after setting the context
inline bool HasExt(const std::string& ext_name) {
    static std::set<std::string> names;
    if (names.empty()) {
        int NumberOfExtensions;
        glGetIntegerv(GL_NUM_EXTENSIONS, &NumberOfExtensions);
        for (int i = 0; i < NumberOfExtensions; i++) {
            names.emplace((const char*)glGetStringi(GL_EXTENSIONS, i));
        }
    }
    return names.count(ext_name) > 0;
}

int Init();
bool IsInitialized();
void Terminate();
std::string GetFrameBufferRGBContent(int width, int height, GLuint frame_buffer);
std::string GetFrameBufferRGBAContent(int width, int height, GLuint frame_buffer);

namespace cpp {

class CustomGLFWContext : public std::enable_shared_from_this<CustomGLFWContext> {
 public:
    using CustomResizeCallback =
        std::function<void(std::shared_ptr<CustomGLFWContext> window, int width, int height)>;
    using CustomCursorMoveCallback =
        std::function<void(std::shared_ptr<CustomGLFWContext> window, double xpos, double ypos)>;
    using CustomMouseButtonCallback = std::function<void(std::shared_ptr<CustomGLFWContext> window,
                                                         int button, int action, int mods)>;

    static std::shared_ptr<CustomGLFWContext> GetCurrentGLFWContext();

 public:
    static std::shared_ptr<CustomGLFWContext> Generate() {
        return std::shared_ptr<CustomGLFWContext>(new CustomGLFWContext);
    }
    std::shared_ptr<CustomGLFWContext> GetPtr() { return shared_from_this(); }
    ~CustomGLFWContext();

    static void sOnResize(GLFWwindow* window, int width, int height);
    static void sOnMouseButton(GLFWwindow* window, int button, int action, int mods);
    static void sOnMouseMove(GLFWwindow* window, double xpos, double ypos);

    bool CreateContext(int version_major, int version_minor, bool osmesa = false);
    bool CreateContext(bool osmesa = false);

    void ExtractFrameBuffer();

    float GetAspect() const;

 protected:
    CustomGLFWContext() {}

    static std::weak_ptr<CustomGLFWContext> s_current_context;

 public:
    GLFWwindow* window = nullptr;
    int width = 1000;
    int height = 800;
    bool visible = true;
    bool osmesa = false;
    std::string name;
    GLint fb = 0;
    CustomResizeCallback on_resize = nullptr;
    CustomCursorMoveCallback on_cursor_move = nullptr;
    CustomMouseButtonCallback on_mouse_button = nullptr;
};

}  // namespace cpp

}  // namespace glit

enum kCullingType {
    kCullingOff,
    kCullingOnCCW,
    kCullingOnCW,
    kCullingOnBoth,
};

enum kBlendingType {
    kNoBlend,
    kNormalBlend,
    kAdditiveBlend,
    kSubtractiveBlend,
    kMultiplyBlend,
    kCustomBlend
};

class RenderState {
 public:
    static RenderState& Instance() {
        static RenderState instance;
        return instance;
    }

    void ApplyRenderState();

 public:
    bool flag_depth_test = true;
    bool highlight = false;
    kBlendingType blending = kNoBlend;
    kCullingType culling = kCullingOff;
};