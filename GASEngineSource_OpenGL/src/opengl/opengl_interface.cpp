#include "opengl_interface.h"
#include <iostream>
#include <string>
#include <vector>
#include "FreeImage.h"  // initialization
#include "utils/logger.h"

namespace glit {
GLFWwindow* CreateContext(int width, int height, int version_major, int version_minor,
                          bool visible = true, bool osmesa = false,
                          const std::string& window_title = "");
GLFWwindow* CreateContext(int width, int height, bool visible = true, bool osmesa = false,
                          const std::string& window_title = "");
GLFWwindow* CreateWindow(int width, int height, bool core = true, int major = 3, int minor = 3,
                         bool visible = false, bool osmesa = true, bool debug = false,
                         const std::string& window_title = "");
GLuint CreateFrameBuffer(int width, int height);

void PrintGLInfo();
void VersionCheck();
std::vector<std::pair<int, int>> TryVersions();
void glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
                   const char* message, const void* userParam);
void ErrorCallback(int error, const char* description);

static bool initialized = false;

void glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
                   const char* message, const void* userParam) {
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    if (severity >= GL_DEBUG_SEVERITY_LOW) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            std::cout << "Source: API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            std::cout << "Source: Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            std::cout << "Source: Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            std::cout << "Source: Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            std::cout << "Source: Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            std::cout << "Source: Other";
            break;
    }
    std::cout << std::endl;

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            std::cout << "Type: Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            std::cout << "Type: Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            std::cout << "Type: Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            std::cout << "Type: Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            std::cout << "Type: Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            std::cout << "Type: Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            std::cout << "Type: Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            std::cout << "Type: Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            std::cout << "Type: Other";
            break;
    }
    std::cout << std::endl;

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            std::cout << "Severity: high";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            std::cout << "Severity: medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            std::cout << "Severity: low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            std::cout << "Severity: notification";
            break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

GLFWwindow* CreateWindow(int width, int height, bool core, int major, int minor, bool visible,
                         bool osmesa, bool debug, const std::string& window_title) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    if (debug) glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    if (osmesa) glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_OSMESA_CONTEXT_API);
    // else glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);
    if (!is_osmesa_build) {
        if (major >= 3)
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_SAMPLES, 4);
    }
    if (major < 3) {
        core = false;
    } else if (minor < 3) {
        core = false;
    }
    if (major * 10 + minor >= 32)
        glfwWindowHint(GLFW_OPENGL_PROFILE,
                       core ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    GLFWwindow* window = glfwCreateWindow(width, height, window_title.c_str(), NULL, NULL);
    if (window == nullptr) {
        LOG_ERROR("failed to create window");
        return nullptr;
    } else {
        return window;
        // glfwDestroyWindow(window);
    }
}

void PrintGLInfo() {
    // to use mesa
    // export LIBGL_ALWAYS_SOFTWARE=true
    // export GALLIUM_DRIVER=llvmpipe     (or softpipe swr)
    // to create higher context
    // export MESA_GL_VERSION_OVERRIDE=4.5
    // export MESA_GLSL_VERSION_OVERRIDE=450
    // but may have runtime error
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_ERROR("glad: loader went wrong!");
    }
    // need to set context first
    std::string ver_GL;
    std::string ver_Vender;
    std::string ver_Renderer;
    std::string ver_GLSL;
    if (glGetString != nullptr) {
        if (glGetString(GL_VERSION)) {
            ver_GL = (const char*)glGetString(GL_VERSION);
        }
        if (glGetString(GL_VENDOR)) {
            ver_Vender = (const char*)glGetString(GL_VENDOR);
        }
        if (glGetString(GL_RENDERER)) {
            ver_Renderer = (const char*)glGetString(GL_RENDERER);
        }
        if (glGetString(GL_SHADING_LANGUAGE_VERSION)) {
            ver_GLSL = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
        }
        LOG_INFO("GL: %s, GLSL: %s, Vender: %s, Renderer: %s", ver_GL.c_str(), ver_GLSL.c_str(),
                 ver_Vender.c_str(), ver_Renderer.c_str());
    } else {
        LOG_ERROR("glGetString not available");
    }
}

std::vector<std::pair<int, int>> TryVersions() {
    std::vector<std::pair<int, int>> versions_to_try = {
        // {2, 0}, {3, 0}, {3, 1}, {3, 2},  // set to not supported
        {3, 3}, {4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 5}, {4, 6}};
    std::vector<std::pair<int, int>> supported;
    for (const auto& item : versions_to_try) {
        LOG_DEBUG("trying version: %d.%d", item.first, item.second);
        GLFWwindow* ptr = CreateWindow(100, 100, true, item.first, item.second, false,
#ifdef USE_OSMESA
                                       true
#else
                                       false
#endif
        );
        if (ptr) {
            glfwMakeContextCurrent(ptr);
            PrintGLInfo();
            glfwMakeContextCurrent(nullptr);
            glfwDestroyWindow(ptr);
            supported.push_back(item);
        }
    }
    return supported;
}

void VersionCheck() {
    // something in 3.2
    if (glDrawElementsBaseVertex && glDrawRangeElementsBaseVertex &&
        glDrawElementsInstancedBaseVertex && glMultiDrawElementsBaseVertex && glProvokingVertex &&
        glFenceSync && glIsSync && glDeleteSync && glClientWaitSync && glWaitSync &&
        glGetInteger64v && glGetSynciv && glGetInteger64i_v && glGetBufferParameteri64v &&
        glFramebufferTexture && glTexImage2DMultisample && glTexImage3DMultisample &&
        glGetMultisamplefv && glSampleMaski) {
        LOG_DEBUG("glad 3.2 all found");
    } else {
        LOG_INFO("glad 3.2 maybe unsupported");
    }

    // something in 3.3
    if (glBindFragDataLocationIndexed && glGetFragDataIndex && glGenSamplers && glDeleteSamplers &&
        glIsSampler && glBindSampler && glSamplerParameteri && glSamplerParameteriv &&
        glSamplerParameterf && glSamplerParameterfv && glSamplerParameterIiv &&
        glSamplerParameterIuiv && glGetSamplerParameteriv && glGetSamplerParameterIiv &&
        glGetSamplerParameterfv && glGetSamplerParameterIuiv && glQueryCounter &&
        glGetQueryObjecti64v && glGetQueryObjectui64v && glVertexAttribDivisor &&
        glVertexAttribP1ui && glVertexAttribP1uiv && glVertexAttribP2ui && glVertexAttribP2uiv &&
        glVertexAttribP3ui && glVertexAttribP3uiv && glVertexAttribP4ui && glVertexAttribP4uiv &&
        glVertexP2ui && glVertexP2uiv && glVertexP3ui && glVertexP3uiv && glVertexP4ui &&
        glVertexP4uiv && glTexCoordP1ui && glTexCoordP1uiv && glTexCoordP2ui && glTexCoordP2uiv &&
        glTexCoordP3ui && glTexCoordP3uiv && glTexCoordP4ui && glTexCoordP4uiv &&
        glMultiTexCoordP1ui && glMultiTexCoordP1uiv && glMultiTexCoordP2ui &&
        glMultiTexCoordP2uiv && glMultiTexCoordP3ui && glMultiTexCoordP3uiv &&
        glMultiTexCoordP4ui && glMultiTexCoordP4uiv && glNormalP3ui && glNormalP3uiv &&
        glColorP3ui && glColorP3uiv && glColorP4ui && glColorP4uiv && glSecondaryColorP3ui &&
        glSecondaryColorP3uiv) {
        LOG_DEBUG("glad 3.3 all found");
    } else {
        LOG_INFO("glad 3.3 maybe unsupported");
    }

    // something in 4.0
    if (glMinSampleShading && glBlendEquationi && glBlendEquationSeparatei && glBlendFunci &&
        glBlendFuncSeparatei && glDrawArraysIndirect && glDrawElementsIndirect && glUniform1d &&
        glUniform2d && glUniform3d && glUniform4d && glUniform1dv && glUniform2dv && glUniform3dv &&
        glUniform4dv && glUniformMatrix2dv && glUniformMatrix3dv && glUniformMatrix4dv &&
        glUniformMatrix2x3dv && glUniformMatrix2x4dv && glUniformMatrix3x2dv &&
        glUniformMatrix3x4dv && glUniformMatrix4x2dv && glUniformMatrix4x3dv && glGetUniformdv &&
        glGetSubroutineUniformLocation && glGetSubroutineIndex && glGetActiveSubroutineUniformiv &&
        glGetActiveSubroutineUniformName && glGetActiveSubroutineName && glUniformSubroutinesuiv &&
        glGetUniformSubroutineuiv && glGetProgramStageiv && glPatchParameteri &&
        glPatchParameterfv && glBindTransformFeedback && glDeleteTransformFeedbacks &&
        glGenTransformFeedbacks && glIsTransformFeedback && glPauseTransformFeedback &&
        glResumeTransformFeedback && glDrawTransformFeedback && glDrawTransformFeedbackStream &&
        glBeginQueryIndexed && glEndQueryIndexed && glGetQueryIndexediv) {
        LOG_DEBUG("glad 4.0 all found");
    } else {
        LOG_INFO("glad 4.0 maybe unsupported");
    }

    // something in 4.1
    if (glReleaseShaderCompiler && glShaderBinary && glGetShaderPrecisionFormat && glDepthRangef &&
        glClearDepthf && glGetProgramBinary && glProgramBinary && glProgramParameteri &&
        glUseProgramStages && glActiveShaderProgram && glCreateShaderProgramv &&
        glBindProgramPipeline && glDeleteProgramPipelines && glGenProgramPipelines &&
        glIsProgramPipeline && glGetProgramPipelineiv && glProgramUniform1i &&
        glProgramUniform1iv && glProgramUniform1f && glProgramUniform1fv && glProgramUniform1d &&
        glProgramUniform1dv && glProgramUniform1ui && glProgramUniform1uiv && glProgramUniform2i &&
        glProgramUniform2iv && glProgramUniform2f && glProgramUniform2fv && glProgramUniform2d &&
        glProgramUniform2dv && glProgramUniform2ui && glProgramUniform2uiv && glProgramUniform3i &&
        glProgramUniform3iv && glProgramUniform3f && glProgramUniform3fv && glProgramUniform3d &&
        glProgramUniform3dv && glProgramUniform3ui && glProgramUniform3uiv && glProgramUniform4i &&
        glProgramUniform4iv && glProgramUniform4f && glProgramUniform4fv && glProgramUniform4d &&
        glProgramUniform4dv && glProgramUniform4ui && glProgramUniform4uiv &&
        glProgramUniformMatrix2fv && glProgramUniformMatrix3fv && glProgramUniformMatrix4fv &&
        glProgramUniformMatrix2dv && glProgramUniformMatrix3dv && glProgramUniformMatrix4dv &&
        glProgramUniformMatrix2x3fv && glProgramUniformMatrix3x2fv && glProgramUniformMatrix2x4fv &&
        glProgramUniformMatrix4x2fv && glProgramUniformMatrix3x4fv && glProgramUniformMatrix4x3fv &&
        glProgramUniformMatrix2x3dv && glProgramUniformMatrix3x2dv && glProgramUniformMatrix2x4dv &&
        glProgramUniformMatrix4x2dv && glProgramUniformMatrix3x4dv && glProgramUniformMatrix4x3dv &&
        glValidateProgramPipeline && glGetProgramPipelineInfoLog && glVertexAttribL1d &&
        glVertexAttribL2d && glVertexAttribL3d && glVertexAttribL4d && glVertexAttribL1dv &&
        glVertexAttribL2dv && glVertexAttribL3dv && glVertexAttribL4dv && glVertexAttribLPointer &&
        glGetVertexAttribLdv && glViewportArrayv && glViewportIndexedf && glViewportIndexedfv &&
        glScissorArrayv && glScissorIndexed && glScissorIndexedv && glDepthRangeArrayv &&
        glDepthRangeIndexed && glGetFloati_v && glGetDoublei_v) {
        LOG_DEBUG("glad 4.1 all found");
    } else {
        LOG_INFO("glad 4.1 maybe unsupported");
    }

    // something in 4.2
    if (glDrawArraysInstancedBaseInstance && glDrawElementsInstancedBaseInstance &&
        glDrawElementsInstancedBaseVertexBaseInstance && glGetInternalformativ &&
        glGetActiveAtomicCounterBufferiv && glBindImageTexture && glMemoryBarrier &&
        glTexStorage1D && glTexStorage2D && glTexStorage3D && glDrawTransformFeedbackInstanced &&
        glDrawTransformFeedbackStreamInstanced) {
        LOG_DEBUG("glad 4.2 all found");
    } else {
        LOG_INFO("glad 4.2 maybe unsupported");
    }

    // something in 4.3
    if (glClearBufferData && glClearBufferSubData && glDispatchCompute &&
        glDispatchComputeIndirect && glCopyImageSubData && glFramebufferParameteri &&
        glGetFramebufferParameteriv && glGetInternalformati64v && glInvalidateTexSubImage &&
        glInvalidateTexImage && glInvalidateBufferSubData && glInvalidateBufferData &&
        glInvalidateFramebuffer && glInvalidateSubFramebuffer && glMultiDrawArraysIndirect &&
        glMultiDrawElementsIndirect && glGetProgramInterfaceiv && glGetProgramResourceIndex &&
        glGetProgramResourceName && glGetProgramResourceiv && glGetProgramResourceLocation &&
        glGetProgramResourceLocationIndex && glShaderStorageBlockBinding && glTexBufferRange &&
        glTexStorage2DMultisample && glTexStorage3DMultisample && glTextureView &&
        glBindVertexBuffer && glVertexAttribFormat && glVertexAttribIFormat &&
        glVertexAttribLFormat && glVertexAttribBinding && glVertexBindingDivisor &&
        glDebugMessageControl && glDebugMessageInsert && glDebugMessageCallback &&
        glGetDebugMessageLog && glPushDebugGroup && glPopDebugGroup && glObjectLabel &&
        glGetObjectLabel && glObjectPtrLabel && glGetObjectPtrLabel && glGetPointerv) {
        LOG_DEBUG("glad 4.3 all found");
    } else {
        LOG_INFO("glad 4.3 maybe unsupported");
    }

    // something in 4.4
    if (glBufferStorage && glClearTexImage && glClearTexSubImage && glBindBuffersBase &&
        glBindBuffersRange && glBindTextures && glBindSamplers && glBindImageTextures &&
        glBindVertexBuffers) {
        LOG_DEBUG("glad 4.4 all found");
    } else {
        LOG_INFO("glad 4.4 maybe unsupported");
    }

    // something in 4.6
    if (glClipControl && glCreateTransformFeedbacks && glTransformFeedbackBufferBase &&
        glTransformFeedbackBufferRange && glGetTransformFeedbackiv && glGetTransformFeedbacki_v &&
        glGetTransformFeedbacki64_v && glCreateBuffers && glNamedBufferStorage &&
        glNamedBufferData && glNamedBufferSubData && glCopyNamedBufferSubData &&
        glClearNamedBufferData && glClearNamedBufferSubData && glMapNamedBuffer &&
        glMapNamedBufferRange && glUnmapNamedBuffer && glFlushMappedNamedBufferRange &&
        glGetNamedBufferParameteriv && glGetNamedBufferParameteri64v && glGetNamedBufferPointerv &&
        glGetNamedBufferSubData && glCreateFramebuffers && glNamedFramebufferRenderbuffer &&
        glNamedFramebufferParameteri && glNamedFramebufferTexture &&
        glNamedFramebufferTextureLayer && glNamedFramebufferDrawBuffer &&
        glNamedFramebufferDrawBuffers && glNamedFramebufferReadBuffer &&
        glInvalidateNamedFramebufferData && glInvalidateNamedFramebufferSubData &&
        glClearNamedFramebufferiv && glClearNamedFramebufferuiv && glClearNamedFramebufferfv &&
        glClearNamedFramebufferfi && glBlitNamedFramebuffer && glCheckNamedFramebufferStatus &&
        glGetNamedFramebufferParameteriv && glGetNamedFramebufferAttachmentParameteriv &&
        glCreateRenderbuffers && glNamedRenderbufferStorage &&
        glNamedRenderbufferStorageMultisample && glGetNamedRenderbufferParameteriv &&
        glCreateTextures && glTextureBuffer && glTextureBufferRange && glTextureStorage1D &&
        glTextureStorage2D && glTextureStorage3D && glTextureStorage2DMultisample &&
        glTextureStorage3DMultisample && glTextureSubImage1D && glTextureSubImage2D &&
        glTextureSubImage3D && glCompressedTextureSubImage1D && glCompressedTextureSubImage2D &&
        glCompressedTextureSubImage3D && glCopyTextureSubImage1D && glCopyTextureSubImage2D &&
        glCopyTextureSubImage3D && glTextureParameterf && glTextureParameterfv &&
        glTextureParameteri && glTextureParameterIiv && glTextureParameterIuiv &&
        glTextureParameteriv && glGenerateTextureMipmap && glBindTextureUnit && glGetTextureImage &&
        glGetCompressedTextureImage && glGetTextureLevelParameterfv &&
        glGetTextureLevelParameteriv && glGetTextureParameterfv && glGetTextureParameterIiv &&
        glGetTextureParameterIuiv && glGetTextureParameteriv && glCreateVertexArrays &&
        glDisableVertexArrayAttrib && glEnableVertexArrayAttrib && glVertexArrayElementBuffer &&
        glVertexArrayVertexBuffer && glVertexArrayVertexBuffers && glVertexArrayAttribBinding &&
        glVertexArrayAttribFormat && glVertexArrayAttribIFormat && glVertexArrayAttribLFormat &&
        glVertexArrayBindingDivisor && glGetVertexArrayiv && glGetVertexArrayIndexediv &&
        glGetVertexArrayIndexed64iv && glCreateSamplers && glCreateProgramPipelines &&
        glCreateQueries && glGetQueryBufferObjecti64v && glGetQueryBufferObjectiv &&
        glGetQueryBufferObjectui64v && glGetQueryBufferObjectuiv && glMemoryBarrierByRegion &&
        glGetTextureSubImage && glGetCompressedTextureSubImage && glGetGraphicsResetStatus &&
        glGetnCompressedTexImage && glGetnTexImage && glGetnUniformdv && glGetnUniformfv &&
        glGetnUniformiv && glGetnUniformuiv && glReadnPixels && glGetnMapdv && glGetnMapfv &&
        glGetnMapiv && glGetnPixelMapfv && glGetnPixelMapuiv && glGetnPixelMapusv &&
        glGetnPolygonStipple && glGetnColorTable && glGetnConvolutionFilter &&
        glGetnSeparableFilter && glGetnHistogram && glGetnMinmax && glTextureBarrier) {
        LOG_DEBUG("glad 4.5 all found");
    } else {
        LOG_INFO("glad 4.5 maybe unsupported");
    }

    // something in 4.6
    if (glSpecializeShader && glMultiDrawArraysIndirectCount && glMultiDrawElementsIndirectCount &&
        glPolygonOffsetClamp) {
        LOG_DEBUG("glad 4.6 all found");
    } else {
        LOG_INFO("glad 4.6 maybe unsupported");
    }
}

void ErrorCallback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

int Init() {
    if (initialized) {
        return 0;
    }
    if (!glfwInit()) {
        LOG_ERROR("failed to initialize glfw");
        glfwTerminate();
        return -1;
    }
    LOG_INFO("glfw release version: %s", glfwGetVersionString());
    glfwSetErrorCallback(ErrorCallback);

    FreeImage_Initialise();

    initialized = true;
    return 0;
}

bool IsInitialized() { return initialized; }

GLFWwindow* CreateContext(int width, int height, int version_major, int version_minor, bool visible,
                          bool osmesa, const std::string& window_title) {
    GLFWwindow* window = CreateWindow(width, height, true, version_major, version_minor, visible,
                                      osmesa, true, window_title);
    if (window == nullptr) {
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Something went wrong on glad!\n");
        return nullptr;
    }

    LOG_INFO("OpenGL %d.%d", GLVersion.major, GLVersion.minor);
    LOG_INFO("OpenGL %s, GLSL %s", glGetString(GL_VERSION),
             glGetString(GL_SHADING_LANGUAGE_VERSION));

    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    VersionCheck();
    glViewport(0, 0, width, height);
    return window;
}

GLFWwindow* CreateContext(int width, int height, bool visible, bool osmesa,
                          const std::string& window_title) {
    auto supported = TryVersions();
    if (supported.empty()) {
        LOG_ERROR("no supported opengl version found");
        glfwTerminate();
        return nullptr;
    }
    return CreateContext(width, height, supported.back().first, supported.back().second, visible,
                         osmesa, window_title);
}

void PrintExtensions() {
    // call this after setting the context
    GLint n = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n);
    LOG_DEBUG("Total Extensions:  %d", n);
    for (GLint i = 0; i < n; i++) {
        const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
        LOG_DEBUG("Supported Extension %d: %s", i, extension);
    }
}

GLuint CreateFrameBuffer(int width, int height) {
    LOG_INFO("framebuffer resolution is %d x %d", width, height);
    GLuint frame_buffer;
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

    // The color buffer
    GLuint renderedTexture;
    glGenTextures(1, &renderedTexture);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG_ERROR("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return frame_buffer;
}

std::string GetFrameBufferRGBContent(int width, int height, GLuint frame_buffer) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, frame_buffer);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    std::string buffer;
    buffer.resize(3 * width * height, 0);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)buffer.data());
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    return buffer;
}

std::string GetFrameBufferRGBAContent(int width, int height, GLuint frame_buffer) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, frame_buffer);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    std::string buffer;
    buffer.resize(4 * width * height, 0);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)buffer.data());
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    return buffer;
}

void Terminate() {
    initialized = false;
    FreeImage_DeInitialise();
    glfwTerminate();
}

namespace cpp {
std::weak_ptr<CustomGLFWContext> CustomGLFWContext::s_current_context;
std::shared_ptr<CustomGLFWContext> CustomGLFWContext::GetCurrentGLFWContext() {
    auto spt = s_current_context.lock();
    return spt;
}
CustomGLFWContext::~CustomGLFWContext() {
    if (window != nullptr) {
        glfwSetWindowUserPointer(window, nullptr);
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    window = nullptr;
}
void CustomGLFWContext::sOnResize(GLFWwindow* window, int width, int height) {
    CustomGLFWContext* ptr = reinterpret_cast<CustomGLFWContext*>(glfwGetWindowUserPointer(window));
    if (ptr != nullptr) {
        ptr->width = width;
        ptr->height = height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);  // for mac retina display
        if (ptr->on_resize != nullptr) ptr->on_resize(ptr->GetPtr(), width, height);
    }
}
void CustomGLFWContext::sOnMouseButton(GLFWwindow* window, int button, int action, int mods) {
    CustomGLFWContext* ptr = reinterpret_cast<CustomGLFWContext*>(glfwGetWindowUserPointer(window));
    if (ptr != nullptr && ptr->on_mouse_button != nullptr) {
        ptr->on_mouse_button(ptr->GetPtr(), button, action, mods);
    }
}
void CustomGLFWContext::sOnMouseMove(GLFWwindow* window, double xpos, double ypos) {
    CustomGLFWContext* ptr = reinterpret_cast<CustomGLFWContext*>(glfwGetWindowUserPointer(window));
    if (ptr != nullptr && ptr->on_cursor_move != nullptr) {
        ptr->on_cursor_move(ptr->GetPtr(), xpos, ypos);
    }
}
bool CustomGLFWContext::CreateContext(int version_major, int version_minor, bool osmesa) {
    window =
        glit::CreateContext(width, height, version_major, version_minor, visible, osmesa, name);
    if (window) {
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);  // for mac retina display

        glfwSetWindowUserPointer(window, this);
        glfwSetWindowSizeCallback(window, sOnResize);
        glfwSetMouseButtonCallback(window, sOnMouseButton);
        glfwSetCursorPosCallback(window, sOnMouseMove);
        ExtractFrameBuffer();
        s_current_context = GetPtr();

        PrintExtensions();
    }
    return window != nullptr;
}
bool CustomGLFWContext::CreateContext(bool osmesa) {
    this->osmesa = osmesa;
    window = glit::CreateContext(width, height, visible, osmesa, name);
    if (window) {
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);  // for mac retina display

        glfwSetWindowUserPointer(window, this);
        glfwSetWindowSizeCallback(window, sOnResize);
        ExtractFrameBuffer();
        s_current_context = GetPtr();

        PrintExtensions();
    }
    return window != nullptr;
}
void CustomGLFWContext::ExtractFrameBuffer() {
    if (osmesa) {
        fb = glit::CreateFrameBuffer(width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, fb);
    } else {
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fb);
    }
}
float CustomGLFWContext::GetAspect() const {
    double aspect = width / (double)height;
    if (isnan(aspect)) {
        return 1.0f;
    } else {
        return aspect;
    }
}
}  // namespace cpp

}  // namespace glit

void RenderState::ApplyRenderState() {
    // glEnable(GL_MULTISAMPLE);

    // culling mode
    glFrontFace(GL_CCW);
    switch (culling) {
        case kCullingOff: {
            glDisable(GL_CULL_FACE);
            break;
        }
        case kCullingOnCCW: {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        }
        case kCullingOnCW: {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
        }
        case kCullingOnBoth: {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT_AND_BACK);
            break;
        }
        default:
            break;
    }

    // blend mode
    switch (blending) {
        case kNoBlend: {
            glDisable(GL_BLEND);
            glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
        } break;
        case kNormalBlend: {
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
        } break;
        case kAdditiveBlend: {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
        } break;
        case kSubtractiveBlend: {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } break;
        case kMultiplyBlend: {
            LOG_ERROR("not implemented yet.");
        } break;
        case kCustomBlend: {
            LOG_ERROR("not implemented yet.");
        } break;
        default:
            break;
    }
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // depth test
    if (flag_depth_test) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    // stencil test
    glEnable(GL_STENCIL_TEST);
    // glStencilFunc(GL_NEVER, 0, 0xFFFFFFFF);
    if (highlight) {
        glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 3, 0xFF);
    } else {
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
    }
    glStencilMask(0xFF);
}
