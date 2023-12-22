#include <stdlib.h>
#include <chrono>
#include <string>
#include <thread>
#include "GLFW/glfw3.h"
#include "opengl_interface.h"
#include "utils/bmp.h"
#include "utils/logger.h"

#ifdef USE_OSMESA
const bool osmesa = true;
#else
const bool osmesa = false;
#endif

#ifdef WIN32
#define _put_env(p, s) _putenv_s(p, s)
#define _get_env(p) getenv(p)
#else
#define _put_env(p, s)        \
    do {                      \
        if (p) {              \
            std::string m(p); \
            putenv(m.data()); \
        }                     \
    } while (0)
#define _get_env(p) getenv(p)
#endif

inline void set_env(const std::string& key, const std::string& value) {
    _put_env(key.c_str(), value.c_str());
}

inline std::string get_env(const std::string& key) {
    char* ptr = _get_env(key.c_str());
    if (ptr == nullptr)
        return std::string();
    else
        return std::string(ptr);
}

void simple_draw_triangle();
GLuint simple_load_shader(const GLchar* vert_shader, const GLchar* frag_shader);

int main(int argc, char* argv[]) {
    if (osmesa) {
        if (get_env("GALLIUM_DRIVER").empty() || get_env("MESA_GL_VERSION_OVERRIDE").empty() ||
            get_env("MESA_GLSL_VERSION_OVERRIDE").empty()) {
            LOG_ERROR("you can try setting following env for higher opengl version:");
            LOG_ERROR("LIBGL_ALWAYS_SOFTWARE=true    ");
            LOG_ERROR("GALLIUM_DRIVER=llvmpipe       ");
            LOG_ERROR("MESA_GL_VERSION_OVERRIDE=4.6  ");
            LOG_ERROR("MESA_GLSL_VERSION_OVERRIDE=460");
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }

    int ret = glit::Init();
    if (ret) {
        return ret;
    }

    int width = 400;
    int height = 400;
    {
        auto context = glit::cpp::CustomGLFWContext::Generate();
        context->CreateContext(osmesa);
        if (!context->window) {
            return -1;
        }

        const int max_iter = 240;
        int iter = 0;
        do {
            simple_draw_triangle();

            glfwSwapBuffers(context->window);
            glfwPollEvents();
            ++iter;
        } while (!glfwWindowShouldClose(context->window) && !osmesa && iter < max_iter);

        std::string buffer = glit::GetFrameBufferRGBContent(width, height, context->fb);
        BMP bmp(width, height);
        bmp.FromRGB(width, height, buffer.data());
        bmp.save("screen.bmp");
    }

    glit::Terminate();
    return 0;
}

const GLchar* vert_shader = R"(
#version 330 core
layout(location = 0) in vec4 vPosition; 
void main() 
{ 
    gl_Position = vPosition; 
} 
)";
const GLchar* frag_shader = R"(
#version 330 core
layout(location = 0) out vec4 fragColor; 
void main() 
{
    fragColor = vec4(
        (1.0 + sin(gl_FragCoord.x / 100.0))/2.0, 
        (1.0 + cos(gl_FragCoord.y / 100.0))/2.0, 
        (1.0 + cos((gl_FragCoord.y + gl_FragCoord.x) / 100.0))/2.0,
        0.0
    ); 
} 
)";

void simple_draw_triangle() {
    enum VAO_IDs { Triangles, NumVAOs };
    enum Buffer_IDs { ArrayBuffer, NumBuffers };
    enum Attrib_IDs { vPosition = 0 };

    GLuint VAOs[NumVAOs];
    GLuint Buffers[NumBuffers];

    const GLuint NumVertices = 6;
    static const GLfloat vertices[NumVertices][2] = {
        {-0.90, -0.90}, {0.85, -0.90}, {-0.60, 0.85}, {0.90, -0.85}, {0.90, 0.90}, {-0.55, 0.90},
    };
    // gl 3.3 call
    glGenBuffers(NumBuffers, Buffers);
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // gl 3.3 call
    glGenVertexArrays(NumVAOs, VAOs);
    glBindVertexArray(VAOs[Triangles]);
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
    glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(vPosition);

    // load shader
    GLuint program = simple_load_shader(vert_shader, frag_shader);
    glUseProgram(program);

    // draw
    glClearColor(0.5, 0.0, 0.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(VAOs[Triangles]);
    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    // glFinish();
}

GLuint simple_load_shader(const GLchar* vert_shader, const GLchar* frag_shader) {
    GLuint program = glCreateProgram();
    {
        GLuint shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(shader, 1, &vert_shader, NULL);
        glCompileShader(shader);

        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLsizei len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

            GLchar* log = new GLchar[len + 1];
            glGetShaderInfoLog(shader, len, &len, log);
            fprintf(stderr, "Shader compilation failed: %s\n", log);
            delete[] log;

            return 0;
        }
        glAttachShader(program, shader);
    }
    {
        GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(shader, 1, &frag_shader, NULL);
        glCompileShader(shader);

        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLsizei len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

            GLchar* log = new GLchar[len + 1];
            glGetShaderInfoLog(shader, len, &len, log);
            fprintf(stderr, "Shader compilation failed: %s\n", log);
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
        fprintf(stderr, "Shader linking failed: %s\n", log);
        delete[] log;

        return 0;
    }

    return program;
}